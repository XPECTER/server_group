#include "stdafx.h"
#include "Config.h"
#include "GameServer.h"

CGameServer::CGameServer(int iClientMax) : CMMOServer(iClientMax)
{
	InitializeSRWLock(&this->_sessionKeyMapLock);

	// LanClient
	this->_lanClient_Monitoring = new CLanClient_Monitoring(this);
	this->_lanClient_Login = new CLanClient_Login(this);
	this->_lanClient_Agent = new CLanClient_Agent(this);

	// Database
	this->_database_Account = new AccountDB(g_Config.szAccountDBIP, g_Config.szAccountDBUser, g_Config.szAccountDBPassword, g_Config.szAccountDBName, g_Config.iAccountDBPort);
	this->_database_Game = new GameDB(g_Config.szGameDBIP, g_Config.szGameDBUser, g_Config.szGameDBPassword, g_Config.szGameDBName, g_Config.iGameDBPort);
	this->_database_Log = new LogDB(g_Config.szLogDBIP, g_Config.szLogDBUser, g_Config.szLogDBPassword, g_Config.szLogDBName, g_Config.iLogDBPort);

	// GameTh ��Ʈ��Ʈ �뵵
	this->_updateTick = time(NULL);

	// JPS
	this->_jps = new CJumpPointSearch(dfMAP_TILE_X_MAX, dfMAP_TILE_Y_MAX);
	this->_jps->JumpPointSearch_Init();
	this->_jps->LoadTextMap(L"Map.txt");

	// ����͸� ����
	this->_iDatabaseWriteTPS = 0;
	this->_iDatabaseWriteCounter = 0;

#pragma region player
	// �÷��̾� �迭 �Ҵ�
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

	// DB status �ʱ�ȭ
	this->_database_Account->QueryDB(enDB_ACCOUNT_READ_STATUS_INIT, NULL, NULL);
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
	return pServer->DatabaseWriteThread_update();
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
					this->_database_Account->QueryDB(pMsg->Type_Message, pMsg->Message, NULL);
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