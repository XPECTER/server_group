#include "stdafx.h"
#include "Config.h"
#include "GameServer.h"

void CGameServer::CPlayer::Player_Init(void)
{
	this->_clientID = -1;
	this->_connectInfo = NULL;
	this->_recvQ.ClearBuffer();
	this->_iSessionMode = CSession::MODE_NONE;
	this->_bAuthToGame = false;
	this->_bLogout = false;
	//ZeroMemory(&this->_recvOverlap, sizeof(OVERLAPPED));
	//ZeroMemory(&this->_sendOverlap, sizeof(OVERLAPPED));
	this->_IOCount = 0;
	this->_iSending = FALSE;
	this->_iSendCount = 0;
}

bool CGameServer::CPlayer::OnAuth_ClientJoin(void)
{
	return true;
}

bool CGameServer::CPlayer::OnAuth_PacketProc(void)
{
	CPacket *pRecvPacket = NULL;
	WORD type;

	for (int iCnt = 0; iCnt < dfAUTH_PACKET_PROC_REPEAT; ++iCnt)
	{
		if (this->_completeRecvQ.GetUseSize() > 0)
		{
			this->_completeRecvQ.Dequeue(&pRecvPacket);
			*pRecvPacket >> type;

			switch (type)
			{
			case en_PACKET_CS_GAME_REQ_LOGIN:
			{
				PacketProc_Login(pRecvPacket);
				break;
			}

			default:
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_DEBUG, L"Wrong type packet");
				CCrashDump::Crash();
			}
			}

			pRecvPacket->Free();
		}
		else
			break;
	}

	return true;
}

bool CGameServer::CPlayer::OnAuth_ClientLeave(bool bToGame)
{
	return true;
}

bool CGameServer::CPlayer::OnGame_ClientJoin(void)
{

	return true;
}

bool CGameServer::CPlayer::OnGame_PacketProc(void)
{
	CPacket *pRecvPacket = NULL;
	WORD type;

	for (int iCnt = 0; iCnt < dfGAME_PACKET_PROC_REPEAT; ++iCnt)
	{
		if (this->_completeRecvQ.GetUseSize() > 0)
		{
			this->_completeRecvQ.Dequeue(&pRecvPacket);
			*pRecvPacket >> type;

			switch (type)
			{
				case en_PACKET_CS_GAME_REQ_ECHO:
				{
					PacketProc_ReqEcho(pRecvPacket);
					break;
				}

				default:
				{
					SYSLOG(L"SYSTEM", LOG::LEVEL_DEBUG, L"Wrong type packet");
					CCrashDump::Crash();
				}
			}

			pRecvPacket->Free();
		}
		else
			break;
	}

	return true;
}

bool CGameServer::CPlayer::OnGame_ClientLeave(void)
{
	return true;
}

bool CGameServer::CPlayer::OnGame_ClientRelease(void)
{
	this->_clientID = -1;
	this->_connectInfo = NULL;
	this->_recvQ.ClearBuffer();
	this->_iSessionMode = CSession::MODE_NONE;
	this->_bAuthToGame = false;
	this->_bLogout = false;
	this->_iSendCount = 0;

	CPacket *pPacket = NULL;
	while (0 < this->_completeRecvQ.GetUseSize())
	{
		this->_completeRecvQ.Dequeue(&pPacket);
		pPacket->Free();
	}

	while (0 < this->_sendQ.GetUseSize())
	{
		this->_sendQ.Dequeue(&pPacket);
		pPacket->Free();
	}

	return true;
}

void  CGameServer::CPlayer::PacketProc_Login(CPacket *pRecvPacket)
{
	__int64 iAccountNo;
	char SessionKey[64];

	*pRecvPacket >> iAccountNo;
	pRecvPacket->Dequeue(SessionKey, 64);

	// 1. 세션 키 검사 후 없다면 로그인 실패 바로 전송
	// 2. DB에서 accountNo로 검색 후 로그인 성공 여부 전송

	CPacket *pSendPacket = CPacket::Alloc();
	MakePacket_ResLogin(1, iAccountNo, pSendPacket);
	SendPacket(pSendPacket);
	pSendPacket->Free();

	SetMode_Game();
	return;
}

void CGameServer::CPlayer::MakePacket_ResLogin(BYTE iStatus, __int64 iAccountNo, CPacket *pSendPacket)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_LOGIN;
	*pSendPacket << iStatus;
	*pSendPacket << iAccountNo;

	return;
}

void CGameServer::CPlayer::PacketProc_ReqEcho(CPacket *pRecvPacket)
{
	__int64 iAccountNo;
	__int64 SendTick;

	*pRecvPacket >> iAccountNo;
	*pRecvPacket >> SendTick;

	CPacket *pSendPacket = CPacket::Alloc();
	MakePacket_ResEcho(iAccountNo, SendTick, pSendPacket);
	SendPacket(pSendPacket);
	pSendPacket->Free();

	return;
}

void CGameServer::CPlayer::MakePacket_ResEcho(__int64 iAccountNo, __int64 SendTick, CPacket *pSendPacket)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_ECHO;
	*pSendPacket << iAccountNo;
	*pSendPacket << SendTick;

	return;
}

CGameServer::CGameServer(int iClientMax) : CMMOServer(iClientMax)
{
	this->_lanClient_Monitoring = new CLanClient_Monitoring(this);
	this->_lanClient_Login = new CLanClient_Login(this);

	this->pPlayerArray = new CPlayer[iClientMax];

	for (int i = 0; i < iClientMax; ++i)
	{
		this->pPlayerArray[i].Player_Init();
		this->SetSessionArray(i, (void *)&this->pPlayerArray[i]);
	}

	_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThreadFunc, this, NULL, NULL);
	if (NULL == _hMonitorThread)
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Create MonitoringTh failed");
		CCrashDump::Crash();
	}
}

CGameServer::~CGameServer()
{

}

bool CGameServer::Start(void)
{
	CMMOServer::Start(g_Config.szNetBindIP, g_Config.iNetBindPort, g_Config.bNetNagleOpt, g_Config.iNetThreadNum);
	this->_lanClient_Monitoring->Connect(NULL, g_Config.szMonitoringServerIP, g_Config.iMonitoringServerPort, 2, false);
	
	
	return true;
}

void CGameServer::OnAuth_Update(void)
{
	return;
}

void CGameServer::OnGame_Update(void)
{
	return;
}

unsigned __stdcall CGameServer::MonitorThreadFunc(void *lpParam)
{
	CGameServer *pServer = (CGameServer *)lpParam;
	return pServer->MonitorThread_update();
}

bool CGameServer::MonitorThread_update(void)
{
	while (!_bStop)
	{
		_iAcceptTPS = _iAcceptCounter;
		_iSendPacketTPS = _iSendPacketCounter;
		_iRecvPacketTPS = _iRecvPacketCounter;
		_iAuthThLoopTPS = _iAuthThLoopCounter;
		_iGameThLoopTPS = _iGameThLoopCounter;

		if (_lanClient_Monitoring->_bConnected)
		{
			this->SendPacket_SessionCount();
			this->SendPacket_AuthPlayer();
			this->SendPacket_GamePlayer();
			this->SendPacket_AcceptTPS();
			this->SendPacket_RecvPacketTPS();
			this->SendPacket_SendPacketTPS();
			//this->SendPacket_DatabaseWriteTPS();
			//this->SendPacket_DatabaseMsgCount();
			this->SendPacket_AuthThreadTPS();
			this->SendPacket_GameThreadTPS();
			this->SendPacket_PacketUseCount();
		}

		_iAcceptCounter = 0;
		_iSendPacketCounter = 0;
		_iRecvPacketCounter = 0;
		_iAuthThLoopCounter = 0;
		_iGameThLoopCounter = 0;

		Sleep(dfTHREAD_UPDATE_TICK_MONITOR);
	}

	return true;
}

void CGameServer::SendPacket_SessionCount(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_SESSION;
	*pSendPacket << (int)this->_iTotalSessionCounter;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	this->_lanClient_Monitoring->SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CGameServer::SendPacket_AuthPlayer(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_AUTH_PLAYER;
	*pSendPacket << (int)this->_iAuthThSessionCounter;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	this->_lanClient_Monitoring->SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CGameServer::SendPacket_GamePlayer(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_GAME_PLAYER;
	*pSendPacket << (int)this->_iGameThSessionCounter;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	this->_lanClient_Monitoring->SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CGameServer::SendPacket_AcceptTPS(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_ACCEPT_TPS;
	*pSendPacket << (int)this->_iAcceptTPS;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	this->_lanClient_Monitoring->SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CGameServer::SendPacket_RecvPacketTPS(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_PACKET_PROC_TPS;
	*pSendPacket << (int)this->_iRecvPacketTPS;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	this->_lanClient_Monitoring->SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CGameServer::SendPacket_SendPacketTPS(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_PACKET_SEND_TPS;
	*pSendPacket << (int)this->_iSendPacketTPS;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	this->_lanClient_Monitoring->SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CGameServer::SendPacket_DatabaseWriteTPS(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_DB_WRITE_TPS;
	//*pSendPacket << (int)this->;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	this->_lanClient_Monitoring->SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CGameServer::SendPacket_DatabaseMsgCount(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_DB_WRITE_MSG;
	//*pSendPacket << (int)this->;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	this->_lanClient_Monitoring->SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CGameServer::SendPacket_AuthThreadTPS(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_AUTH_THREAD_FPS;
	*pSendPacket << (int)this->_iAuthThLoopTPS;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	this->_lanClient_Monitoring->SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CGameServer::SendPacket_GameThreadTPS(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_GAME_THREAD_FPS;
	*pSendPacket << (int)this->_iGameThLoopTPS;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	this->_lanClient_Monitoring->SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CGameServer::SendPacket_PacketUseCount(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_PACKET_POOL;
	*pSendPacket << (int)CPacket::PacketPool.GetUseCount();
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	this->_lanClient_Monitoring->SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}