#include "stdafx.h"
#include "MMOServer.h"


bool CMMOServer::CSession::SendPacket(CPacket *pSendPacket)
{
	pSendPacket->IncrementRefCount();
	pSendPacket->Encode();

	this->_sendQ.Enqueue(pSendPacket);
	return true;
}

bool CMMOServer::CSession::Disconnect(void)
{
	shutdown(this->_connectInfo->_clientSock, SD_BOTH);
	return true;
}

void CMMOServer::CSession::SetMode_Game(void)
{
	this->_bAuthToGame = true;
	return;
}

/////////////////////////////////////////////////////////////////
//
//	�Լ��̸� : void CMMOServer::CSession::CompleteRecv(int recvBytes)
//  ����
//		int recvBytes : WSARecv�� ���� ����Ʈ ��. GQCS�� ���� transfered
//
//	streamQ�� _recvQ�� write�� �Ű��ְ�, �ϼ��� ��Ŷ�� ������ ��ȣȭ �ؼ�
//	������ ������ _completeRecvQ�� �־��ش�.
//
/////////////////////////////////////////////////////////////////
long CMMOServer::CSession::CompleteRecv(int recvBytes)
{
	int iRecvPacketCount = 0;

	if (!this->_recvQ.MoveWrite(recvBytes))
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"recvQ is full");

	while (true)
	{
		PACKET_HEADER header;

		while (0 < this->_recvQ.GetUseSize())
		{
			this->_recvQ.Peek((char *)&header, sizeof(PACKET_HEADER));

			if (CPacket::_packetCode != header.byCode)
			{
				Disconnect();
				SYSLOG(L"PACKET", LOG::LEVEL_DEBUG, L"[CLIENT_ID : %d] Header Code Error", this->_clientID);
				break;
			}

			if (header.shLen > CPacket::dfDEFUALT_BUFF_SIZE)
			{
				Disconnect();
				SYSLOG(L"PACKET", LOG::LEVEL_DEBUG, L"[CLIENT_ID : %d] Header Len Error", this->_clientID);
				break;
			}

			// ��� ���� + ���̷ε� ����
			if (sizeof(PACKET_HEADER) + header.shLen <= this->_recvQ.GetUseSize())
			{
				CPacket *packet = CPacket::Alloc();
				packet->SetHeaderSize(5);

				if (-1 == this->_recvQ.Dequeue((char *)packet->GetBuffPtr(), header.shLen + sizeof(PACKET_HEADER)))
					SYSLOG(L"SESSION", LOG::LEVEL_ERROR, L"[CLIENT_ID : %d] RecvQ Dequeue error", EXTRACTCLIENTID(this->_clientID));
				else
				{
					// �̹� write�� ���̷ε���� ����Ű�� �ֱ� ������ ���̷ε� �����ŭ�� �Ű��ش�.
					packet->MoveWrite(header.shLen);
				}

				if (!packet->Decode())
				{
					SYSLOG(L"PACKET", LOG::LEVEL_DEBUG, L"[CLIENT_ID : %d] CheckSum Error", this->_clientID);
					Disconnect();
					packet->Free();
					break;
				}

				packet->ClearHeader();
				this->_completeRecvQ.Enqueue(packet);
				iRecvPacketCount++;
			}
			else
				break;
		}

		return iRecvPacketCount;
	}
}


/////////////////////////////////////////////////////////////////
//
//	�Լ��̸� : void CMMOServer::CSession::CompleteSend(void)
//
//	_sendQ�� �ִ� ��Ŷ�� ���� free() ȣ�����ش�. _iSendCount�� WSASend��
//	ȣ���� SendThread�� ������ �� ���̴�. 
//	NetServer�� �ٸ��� SendThread�� �����Ƿ� ���� SendPost�� ȣ������ �ʴ´�.
//
/////////////////////////////////////////////////////////////////
void CMMOServer::CSession::CompleteSend(void)
{
	CPacket *pPacket = NULL;

	for (int i = 0; i < this->_iSendCount; ++i)
	{
		if (false == this->_sendQ.Dequeue(&pPacket))
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"There is no packet in sendQ");
			break;
		}
		pPacket->Free();
	}

	if (FALSE == InterlockedCompareExchange(&this->_iSending, FALSE, TRUE))
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"SendFlag Sync error");
	}

	return;
}

void CMMOServer::CSession::SendPost(void)
{
	if (InterlockedCompareExchange((long *)&this->_iSending, TRUE, FALSE))
		return;

	if (CMMOServer::CSession::MODE_AUTH != this->_iSessionMode &&
		CMMOServer::CSession::MODE_GAME != this->_iSessionMode)
	{
		InterlockedExchange((long *)&this->_iSending, FALSE);
		return;
	}

	if (0 >= this->_sendQ.GetUseSize())
	{
		InterlockedExchange((long *)&this->_iSending, FALSE);
		return;
	}

	InterlockedIncrement(&this->_IOCount);

	WSABUF wsabuf[200];
	int iPacketCount = min(200, this->_sendQ.GetUseSize());
	CPacket *pPacket = NULL;

	for (int i = 0; i < iPacketCount; ++i)
	{
		if (this->_sendQ.Peek(&pPacket, i))
		{
			wsabuf[i].buf = (*pPacket).GetBuffPtr();
			wsabuf[i].len = (*pPacket).GetUseSize();
		}
		else
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"SendQ Peek Error");
			CCrashDump::Crash();
		}
	}

	DWORD lpFlag = 0;
	ZeroMemory(&this->_sendOverlap, sizeof(OVERLAPPED));
	this->_iSendCount = iPacketCount;

	if (SOCKET_ERROR == WSASend(this->_connectInfo->_clientSock, wsabuf, iPacketCount, NULL, lpFlag, &this->_sendOverlap, NULL))
	{
		int iError = WSAGetLastError();
		if (WSA_IO_PENDING != iError)
		{
			if (WSAENOBUFS == iError)
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_WARNING, L"WSAENOBUFS. Check nonpagedpool");
				CCrashDump::Crash();
			}

			this->Disconnect();

			if (0 == InterlockedDecrement(&this->_IOCount))
			{
				this->_bLogout = true;
			}

			InterlockedExchange((long *)&this->_iSending, FALSE);
		}
	}

	return;
}

void CMMOServer::CSession::RecvPost(bool incrementFlag)
{
	WSABUF wsabuf[2];
	int wsabufCount;

	if (true == incrementFlag)
		InterlockedIncrement(&this->_IOCount);

	wsabuf[0].len = this->_recvQ.GetNotBrokenPutsize();
	wsabuf[0].buf = this->_recvQ.GetWriteBufferPtr();
	wsabufCount = 1;

	int isBrokenLen = this->_recvQ.GetFreeSize() - this->_recvQ.GetNotBrokenPutsize();
	if (0 < isBrokenLen)
	{
		wsabuf[1].len = isBrokenLen;
		wsabuf[1].buf = this->_recvQ.GetBufferPtr();
		wsabufCount = 2;
	}

	DWORD lpFlag = 0;

	ZeroMemory(&this->_recvOverlap, sizeof(OVERLAPPED));
	if (SOCKET_ERROR == WSARecv(this->_connectInfo->_clientSock, wsabuf, wsabufCount, NULL, &lpFlag, &this->_recvOverlap, NULL))
	{
		int iError = WSAGetLastError();
		if (WSA_IO_PENDING != iError)
		{
			if (WSAENOBUFS == iError)
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_WARNING, L"WSAENOBUFS. Check nonpagedpool");
				CCrashDump::Crash();
			}

			this->Disconnect();

			if (0 == InterlockedDecrement(&this->_IOCount))
			{
				this->_bLogout = true;
			}
		}
	}

	return;
}