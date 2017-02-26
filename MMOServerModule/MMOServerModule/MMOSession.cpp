#include "stdafx.h"
#include "MMOSession.h"

bool CSESSION::SendPacket(CPacket *pSendPacket)
{
	pSendPacket->IncrementRefCount();
	pSendPacket->Encode();

	this->_sendQ.Enqueue(pSendPacket);
	return true;
}

bool CSESSION::Disconnect(void)
{
	shutdown(this->_connectInfo->_clientSock, SD_BOTH);
	return true;
}

void CSESSION::SetMode_Game(void)
{
	this->_bAuthToGame = true;
	return;
}

/////////////////////////////////////////////////////////////////
//
//	�Լ��̸� : void CSESSION::CompleteRecv(int recvBytes)
//  ����
//		int recvBytes : WSARecv�� ���� ����Ʈ ��. GQCS�� ���� transfered
//
//	streamQ�� _recvQ�� write�� �Ű��ְ�, �ϼ��� ��Ŷ�� ������ ��ȣȭ �ؼ�
//	������ ������ _completeRecvQ�� �־��ش�.
/////////////////////////////////////////////////////////////////
void CSESSION::CompleteRecv(int recvBytes)
{
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
					return;
				}

				this->_completeRecvQ.Enqueue(packet);
			}
			else
				break;
		}

		return;
	}
}


/////////////////////////////////////////////////////////////////
//
//	�Լ��̸� : void CSESSION::CompleteSend(void)
//
//	_sendQ�� �ִ� ��Ŷ�� ���� free() ȣ�����ش�. _iSendCount�� WSASend��
//	ȣ���� SendThread�� ������ �� ���̴�. 
//	NetServer�� �ٸ��� SendThread�� �����Ƿ� ���� SendPost�� ȣ������ �ʴ´�.
/////////////////////////////////////////////////////////////////
void CSESSION::CompleteSend(void)
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

/////////////////////////////////////////////////////////////////
bool CPlayer::OnAuth_ClientJoin(void)
{
	return true;
}

bool CPlayer::OnAuth_PacketProc(void)
{
	while (true)
	{
		break;
	}

	return true;
}

bool CPlayer::OnAuth_ClientLeave(bool bToGame)
{
	return true;
}

bool CPlayer::OnGame_ClientJoin(void)
{
	this->_iSessionMode = en_SESSION_MODE::MODE_GAME;
	return true;
}

bool CPlayer::OnGame_PacketProc(void)
{
	while (true)
	{
		break;
	}

	return true;
}

bool CPlayer::OnGame_ClientLeave(void)
{
	return true;
}

bool CPlayer::OnGame_ClientRelease(void)
{
	return true;
}