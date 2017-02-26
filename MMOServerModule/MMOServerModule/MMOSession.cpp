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
//	함수이름 : void CSESSION::CompleteRecv(int recvBytes)
//  인자
//		int recvBytes : WSARecv로 받은 바이트 수. GQCS로 받은 transfered
//
//	streamQ인 _recvQ의 write를 옮겨주고, 완성된 패킷이 있으면 복호화 해서
//	검증이 끝나면 _completeRecvQ에 넣어준다.
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

			// 헤더 길이 + 페이로드 길이
			if (sizeof(PACKET_HEADER) + header.shLen <= this->_recvQ.GetUseSize())
			{
				CPacket *packet = CPacket::Alloc();
				packet->SetHeaderSize(5);

				if (-1 == this->_recvQ.Dequeue((char *)packet->GetBuffPtr(), header.shLen + sizeof(PACKET_HEADER)))
					SYSLOG(L"SESSION", LOG::LEVEL_ERROR, L"[CLIENT_ID : %d] RecvQ Dequeue error", EXTRACTCLIENTID(this->_clientID));
				else
				{
					// 이미 write는 페이로드부터 가르키고 있기 때문에 페이로드 사이즈만큼만 옮겨준다.
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
//	함수이름 : void CSESSION::CompleteSend(void)
//
//	_sendQ에 있는 패킷에 대해 free() 호출해준다. _iSendCount는 WSASend를
//	호출한 SendThread가 세팅해 줄 것이다. 
//	NetServer와 다르게 SendThread가 있으므로 따로 SendPost는 호출하지 않는다.
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