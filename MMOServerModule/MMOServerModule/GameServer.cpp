#include "stdafx.h"
#include "Config.h"
#include "GameServer.h"

CGameServer::CPlayer::CPlayer()
{
	
}

CGameServer::CPlayer::~CPlayer()
{

}

void CGameServer::CPlayer::Player_Init(CGameServer *pGameServer)
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

	this->_heartBeatTick = time(NULL);
	this->_pGameServer = pGameServer;
}

void CGameServer::CPlayer::CheckHeartBeat(void)
{
	if (time(NULL) - this->_heartBeatTick > dfCLIENT_HEARTBEAT_TICK)
		this->Disconnect();

	return;
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

				case en_PACKET_CS_GAME_REQ_CHARACTER_SELECT:
				{
					PacketProc_CharacterSelect(pRecvPacket);
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

				case en_PACKET_CS_GAME_REQ_HEARTBEAT:
				{
					PacketProc_ClientHeartBeat(pRecvPacket);
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
	st_DBWRITER_MSG *pMsg = this->_pGameServer->_databaseMsgPool.Alloc();
	pMsg->Type_DB = dfDBWRITER_TYPE_ACCOUNT;
	pMsg->Type_Message = enDB_ACCOUNT_WRITE_STATUS_LOGOUT;

	stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in data;
	data.AccountNo = this->_accountNo;
	memcpy_s(pMsg->Message, dfDBWRITER_MSG_MAX, &data, sizeof(stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in));

	this->_pGameServer->_databaseMsgQueue.Enqueue(pMsg);
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
	this->_heartBeatTick = time(NULL);
	
	CPacket *pSendPacket = CPacket::Alloc();
	if (dfDUMMY_ACCOUNTNO_LIMIT < iAccountNo)
	{
		if (!this->_pGameServer->CheckSessionKey(iAccountNo, SessionKey))
		{
			MakePacket_ResLogin(pSendPacket, 0, iAccountNo);
			SendPacket(pSendPacket);
			pSendPacket->Free();

			return;
		}
	}

	// DB검색
	this->_byParty = 1;
	MakePacket_ResLogin(pSendPacket, 1, iAccountNo);
	SendPacket(pSendPacket);
	pSendPacket->Free();

	SetMode_Game();
	return;
}

void CGameServer::CPlayer::MakePacket_ResLogin(CPacket *pSendPacket, BYTE byStatus, __int64 iAccountNo)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_LOGIN;
	*pSendPacket << byStatus;
	*pSendPacket << iAccountNo;

	return;
}

void CGameServer::CPlayer::PacketProc_CharacterSelect(CPacket *pRecvPacket)
{
	BYTE characterType;
	BYTE byResult;

	*pRecvPacket >> characterType;

	this->_heartBeatTick = time(NULL);
	byResult = 0;
	
	if (1 == this->_byParty)
	{
		if (1 <= characterType && 3 >= characterType)
			byResult = 1;
	}
	else if (2 == this->_byParty)
	{
		if (3 <= characterType && 5 >= characterType)
			byResult = 1;
	}
	else
	{
		SYSLOG(L"PACKET", LOG::LEVEL_DEBUG, L"unknown party num");
		CCrashDump::Crash();
	}

	CPacket *pSendPacket = CPacket::Alloc();
	MakePacket_ResCharacterSelect(pSendPacket, byResult);
	SendPacket(pSendPacket);
	pSendPacket->Free();

	if (1 == byResult)
		SetMode_Game();

	return;
}

void CGameServer::CPlayer::MakePacket_ResCharacterSelect(CPacket *pSendPacket, BYTE byStatus)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_CHARACTER_SELECT;
	*pSendPacket << byStatus;
}

void CGameServer::CPlayer::PacketProc_ReqEcho(CPacket *pRecvPacket)
{
	__int64 iAccountNo;
	__int64 SendTick;

	this->_heartBeatTick = time(NULL);

	*pRecvPacket >> iAccountNo;
	*pRecvPacket >> SendTick;

	CPacket *pSendPacket = CPacket::Alloc();
	MakePacket_ResEcho(pSendPacket, iAccountNo, SendTick);
	SendPacket(pSendPacket);
	pSendPacket->Free();

	return;
}

void CGameServer::CPlayer::MakePacket_ResEcho(CPacket *pSendPacket, __int64 iAccountNo, __int64 SendTick)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_ECHO;
	*pSendPacket << iAccountNo;
	*pSendPacket << SendTick;

	return;
}

void CGameServer::CPlayer::PacketProc_ClientHeartBeat(CPacket *pRecvPacket)
{
	this->_heartBeatTick = time(NULL);
	return;
}




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


CGameServer::CGameServer(int iClientMax) : CMMOServer(iClientMax)
{
	InitializeSRWLock(&this->_sessionKeyMapLock);

	// LanClient
	this->_lanClient_Monitoring = new CLanClient_Monitoring(this);
	this->_lanClient_Login = new CLanClient_Login(this);
	this->_lanClient_Agent = new CLanClient_Agent(this);

	// GameTh 하트비트 용도
	this->_updateTick = time(NULL);

	// 모니터링 변수
	this->_iDatabaseWriteTPS = 0;
	this->_iDatabaseWriteCounter = 0;

#pragma region player
	// 플레이어 배열 할당
	this->pPlayerArray = new CPlayer[iClientMax];

	for (int i = 0; i < iClientMax; ++i)
	{
		this->pPlayerArray[i].Player_Init(this);
		this->SetSessionArray(i, (void *)&this->pPlayerArray[i]);
	}
#pragma endregion player

#pragma region createthread
	_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThreadFunc, this, NULL, NULL);
	if (NULL == _hMonitorThread)
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Create MonitoringTh failed");
		CCrashDump::Crash();
	}

	_hDatabaseWriteThread = (HANDLE)_beginthreadex(NULL, 0, DatabaseWriteThread, this, NULL, NULL);
	if (NULL == _hDatabaseWriteThread)
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Create DBWriteTh failed");
		CCrashDump::Crash();
	}
#pragma endregion createthread
}

CGameServer::~CGameServer()
{

}

bool CGameServer::Start(void)
{
	CMMOServer::Start(g_Config.szNetBindIP, g_Config.iNetBindPort, g_Config.bNetNagleOpt, g_Config.iNetThreadNum);
	this->_lanClient_Monitoring->Connect(NULL, g_Config.szMonitoringServerIP, g_Config.iMonitoringServerPort, 2, false);
	this->_lanClient_Login->Connect(NULL, g_Config.szLoginServerIP, g_Config.iLoginServerPort, 2, false);
	//this->_lanClient_Agent->Connect(NULL, g_Config.szAgentServerIP, g_Config.iAgentServerPort, 2, false);
	
	return true;
}

void CGameServer::Stop(void)
{
	CMMOServer::Stop();
	this->_lanClient_Login->Disconnect();
	this->_lanClient_Monitoring->Disconnect();
	this->_lanClient_Agent->Disconnect();

	return;
}

void CGameServer::OnAuth_Update(void)
{
	Schedule_SessionKey();
	Schedule_Client();
	return;
}

void CGameServer::OnGame_Update(void)
{
	if (time(NULL) - this->_updateTick > dfGAMETHREAD_HEARTBEAT_TICK)
		this->_lanClient_Login->SendPacket_HeartBeat(dfTHREAD_TYPE_GAME);
	return;
}

void CGameServer::OnHeartBeat(void)
{
	this->_lanClient_Login->SendPacket_HeartBeat(dfTHREAD_TYPE_WORKER);
}

void CGameServer::AddSessionKey(__int64 accountNo, char *sessionKey)
{
	AcquireSRWLockExclusive(&this->_sessionKeyMapLock);

	auto iter = this->_sessionKeyMap.find(accountNo);
	if (iter == this->_sessionKeyMap.end())
	{
		st_SESSION_KEY *newNode = new st_SESSION_KEY;
		memcpy_s(newNode->_sessionKey, dfSESSION_KEY_LEN, sessionKey, dfSESSION_KEY_LEN);
		newNode->_timeoutTick = time(NULL);

		this->_sessionKeyMap.insert(std::pair<__int64, st_SESSION_KEY *>(accountNo, newNode));
	}
	else
	{
		memcpy_s(iter->second->_sessionKey, dfSESSION_KEY_LEN, sessionKey, dfSESSION_KEY_LEN);
		iter->second->_timeoutTick = time(NULL);
	}
	
	ReleaseSRWLockExclusive(&this->_sessionKeyMapLock);
	return;
}

bool CGameServer::CheckSessionKey(__int64 accountNo, char *sessionKey)
{
	bool bResult = false;

	AcquireSRWLockExclusive(&this->_sessionKeyMapLock);
	auto iter = this->_sessionKeyMap.find(accountNo);

	if (iter != this->_sessionKeyMap.end())
	{
		if (0 == memcmp(sessionKey, iter->second->_sessionKey, dfSESSION_KEY_LEN))
		{
			this->_sessionKeyMap.erase(iter);
			bResult = true;
		}
	}

	ReleaseSRWLockExclusive(&this->_sessionKeyMapLock);
	return bResult;
}

void CGameServer::Schedule_SessionKey(void)
{
	int deleteCount = 0;

	AcquireSRWLockExclusive(&this->_sessionKeyMapLock);

	for (auto iter = this->_sessionKeyMap.begin(); iter != this->_sessionKeyMap.end();)
	{
		if (dfSESSION_KEY_TIMEOUT_TICK < (time(NULL) - (iter->second->_timeoutTick)))
		{
			delete iter->second;
			this->_sessionKeyMap.erase(iter++);

			if (50 < (++deleteCount))
				break;
		}
		else
		{
			++iter;
		}
	}

	ReleaseSRWLockExclusive(&this->_sessionKeyMapLock);
	return;
}

void CGameServer::Schedule_Client(void)
{
	CPlayer *pPlayer = NULL;

	for (int i = 0; i < _iClientMax; ++i)
	{
		pPlayer = &pPlayerArray[i];

		if (CSession::MODE_AUTH == pPlayer->_iSessionMode || CSession::MODE_GAME == pPlayer->_iSessionMode)
			pPlayer->CheckHeartBeat();
	}

	return;
}


unsigned __stdcall CGameServer::MonitorThreadFunc(void *lpParam)
{
	CGameServer *pServer = (CGameServer *)lpParam;

	/*while (!pServer->_bStop)
	{
		pServer->MonitorThread_update();
		Sleep(dfTHREAD_UPDATE_TICK_MONITOR);
	}*/

	return pServer->MonitorThread_update();
}

bool CGameServer::MonitorThread_update(void)
{
	while (!_bStop)
	{
		this->_iAcceptTPS			= this->_iAcceptCounter;
		this->_iSendPacketTPS		= this->_iSendPacketCounter;
		this->_iRecvPacketTPS		= this->_iRecvPacketCounter;
		this->_iAuthThLoopTPS		= this->_iAuthThLoopCounter;
		this->_iGameThLoopTPS		= this->_iGameThLoopCounter;
		this->_iDatabaseWriteTPS	= this->_iDatabaseWriteCounter;

		if (_lanClient_Monitoring->_bConnected)
		{
			_lanClient_Monitoring->SendPacket_SessionCount(this->_iTotalSessionCounter);
			_lanClient_Monitoring->SendPacket_AuthPlayer(this->_iAuthThSessionCounter);
			_lanClient_Monitoring->SendPacket_GamePlayer(this->_iGameThSessionCounter);
			_lanClient_Monitoring->SendPacket_AcceptTPS(this->_iAcceptTPS);
			_lanClient_Monitoring->SendPacket_RecvPacketTPS(this->_iRecvPacketTPS);
			_lanClient_Monitoring->SendPacket_SendPacketTPS(this->_iSendPacketTPS);
			_lanClient_Monitoring->SendPacket_DatabaseWriteTPS(this->_iDatabaseWriteTPS);
			_lanClient_Monitoring->SendPacket_DatabaseMsgCount(this->_databaseMsgQueue.GetUseSize());
			_lanClient_Monitoring->SendPacket_AuthThreadTPS(this->_iAuthThLoopTPS);
			_lanClient_Monitoring->SendPacket_GameThreadTPS(this->_iGameThLoopTPS);
			_lanClient_Monitoring->SendPacket_PacketUseCount(CPacket::PacketPool.GetUseCount());
		}

		this->_iAcceptCounter			= 0;
		this->_iSendPacketCounter		= 0;
		this->_iRecvPacketCounter		= 0;
		this->_iAuthThLoopCounter		= 0;
		this->_iGameThLoopCounter		= 0;
		this->_iDatabaseWriteCounter	= 0;

		Sleep(dfTHREAD_UPDATE_TICK_MONITOR);
	}

	return true;
}

unsigned __stdcall CGameServer::DatabaseWriteThread(void *lpParam)
{
	CGameServer *pServer = (CGameServer *)lpParam;
	return 0; /*pServer->MonitorThread_update();*/
}

bool CGameServer::DatabaseWriteThread_update(void)
{
	time_t dbHeartbeatTick = time(NULL);
	time_t cmpTick = 0;
	st_DBWRITER_MSG *pMsg = NULL;

	while (!_bStop)
	{
		if (this->_databaseMsgQueue.Dequeue(&pMsg))
		{
			switch (pMsg->Type_DB)
			{
				case dfDBWRITER_TYPE_ACCOUNT:
				{
					break;
				}

				case dfDBWRITER_TYPE_GAME:
				{
					break;
				}

				case dfDBWRITER_TYPE_HEARTBEAT:
				{
					this->_lanClient_Login->SendPacket_HeartBeat(dfTHREAD_TYPE_DB);
					break;
				}

				default:
				{
					SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Wrong MsgType");
				}
			}

			cmpTick = time(NULL);
			if (cmpTick - dbHeartbeatTick >= dfDATABASETHREAD_HEARTBEAT_TICK)
			{
				this->_lanClient_Login->SendPacket_HeartBeat(dfTHREAD_TYPE_DB);
				dbHeartbeatTick = cmpTick;
			}
		}
		else
			Sleep(dfTHREAD_UPDATE_TICK_DATABASE);
	}

	return true;
}