#include "stdafx.h"
#include "CommonProtocol.h"
#include "main.h"
#include "LoginServer.h"
#include "LanServer_Login.h"


CLoginServer::CLoginServer()
{
	InitializeSRWLock(&this->_srwLock);
	this->_lanserver_Login = new CLanServer_Login(this);

	_hMonitorTPSThread = (HANDLE)_beginthreadex(NULL, 0, MonitorTPSThreadFunc, this, NULL, NULL);
	_hUpdateThread = INVALID_HANDLE_VALUE;

	
}

CLoginServer::~CLoginServer()
{

}

bool CLoginServer::Start(void)
{
	if (!this->_lanserver_Login->Start(g_ConfigData._szLoginServerLanBindIP, 
		g_ConfigData._iLoginServerLanBindPort, 
		g_ConfigData._iLanServerWorkerThreadNum, 
		g_ConfigData._bLoginServerLanNagleOpt, 
		g_ConfigData._LoginServerLanClientMax))
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Login_LanServer start failed.");
		return false;
	}
	else
	{
		if (!CNetServer::Start(g_ConfigData._szLoginServerNetBindIP,
			g_ConfigData._iLoginServerNetBindPort,
			g_ConfigData._iNetServerWorkerThreadNum,
			g_ConfigData._bLoginServerNetNagleOpt,
			g_ConfigData._LoginServerNetClientMax))
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Login_NetServer start failed.");
			return false;
		}
	}

	return true;
}

bool CLoginServer::OnConnectionRequest(wchar_t *szClientIP, int iPort)
{
	return true;
}

void CLoginServer::OnClientJoin(CLIENT_ID clientID)
{
	this->InsertPlayer(clientID);
	return;
}

void CLoginServer::OnClientLeave(CLIENT_ID clientID)
{
	AcquireSRWLockExclusive(&this->_srwLock);
	st_PLAYER *pPlayer = FindPlayer(clientID);

	if (nullptr == pPlayer)
	{
		crashDump.Crash();
	}

	time_t t = time(NULL) - pPlayer->_timeoutTick;

	if (true != pPlayer->_bChatServerRecv)
		InterlockedDecrement(&this->_Monitor_LoginWait);
	ReleaseSRWLockExclusive(&this->_srwLock);
	
	InterlockedCompareExchange(&this->_Monitor_LoginProcessTime_Max, t, t > this->_Monitor_LoginProcessTime_Max);
	InterlockedCompareExchange(&this->_Monitor_LoginProcessTime_Min, t, t < this->_Monitor_LoginProcessTime_Min);
	InterlockedAdd64(&this->_Monitor_LoginProcessTime_Total, t);
	InterlockedIncrement64(&this->_Monitor_LoginProcessCall_Total);
	
	this->RemovePlayer(clientID);
	return;
}

void CLoginServer::OnRecv(CLIENT_ID clientID, CPacket *pPacket)
{
	WORD type;

	*pPacket >> type;

	switch (type)
	{
		case en_PACKET_CS_LOGIN_REQ_LOGIN:
			this->PacketProc_ReqLogin(clientID, pPacket);
			//SYSLOG(L"PACKET", LOG::LEVEL_DEBUG, L"0x%08x", clientID);
			break;

		default:
			break;
	}
}

void CLoginServer::OnSend(CLIENT_ID clientID, int sendsize)
{
	InterlockedIncrement64(&this->_OnSendCallCount);
	InterlockedIncrement(&this->_Monitor_LoginSuccessCounter);
	this->ClientDisconnect(clientID);
	return;
}

void CLoginServer::OnWorkerThreadBegin(void)
{
	return;
}

void CLoginServer::OnWorkerThreadEnd(void)
{
	return;
}

void CLoginServer::OnError(int errorNo, CLIENT_ID clientID, WCHAR *errstr)
{
	return;
}



bool CLoginServer::InsertPlayer(CLIENT_ID clientID)
{
	st_PLAYER *pPlayer = this->_playerPool.Alloc();
	pPlayer->_clientID = clientID;
	pPlayer->_accountNo = -1;
	pPlayer->_sessionKey = nullptr;
	pPlayer->_timeoutTick = time(NULL);
	pPlayer->_bChatServerRecv = false;
	pPlayer->_bGameServerRecv = false;
	pPlayer->_iSendCount = 0;

	AcquireSRWLockExclusive(&this->_srwLock);
	this->_playerMap.insert(std::pair<CLIENT_ID, st_PLAYER *>(clientID, pPlayer));
	ReleaseSRWLockExclusive(&this->_srwLock);

	return true;
}

bool CLoginServer::RemovePlayer(CLIENT_ID clientID)
{
	bool bResult;

	AcquireSRWLockExclusive(&this->_srwLock);

	auto iter = this->_playerMap.find(clientID);
	if (iter == this->_playerMap.end())
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"[CLIENT_ID : %d] not exist player in map", EXTRACTCLIENTID(clientID));
		bResult = false;
	}
	else
	{
		this->_playerPool.Free(iter->second);
		this->_playerMap.erase(iter);
		bResult = true;
	}

	ReleaseSRWLockExclusive(&this->_srwLock);
	return bResult;
}

CLoginServer::st_PLAYER* CLoginServer::FindPlayer(CLIENT_ID clientID)
{
	st_PLAYER *pPlayer = nullptr;

	auto iter = this->_playerMap.find(clientID);
	if (iter != this->_playerMap.end())
		pPlayer = iter->second;
	
	return pPlayer;
}


bool CLoginServer::PacketProc_ReqLogin(CLIENT_ID clientID, CPacket *pPacket)
{
	__int64 iAccountNo;
	char sessionKey[64] = { 0, };

	*pPacket >> iAccountNo;
	pPacket->Dequeue(sessionKey, 64);

	// 내가 Select한 SessionKey와 유저가 가져온 SessionKey가 다르면 컷
	//if (accountNo > dfDUMMY_ACCOUNTNO_MAX && SessionKey != DB에서 찾은 세션 키)
	//{
	//	SendPacket_LoginFailed()
	//	return false;
	//}

	// 세션키 복사 안하려면 패킷의 포인터를 바로 찔러라

	AcquireSRWLockShared(&this->_srwLock);
	st_PLAYER *pPlayer = FindPlayer(clientID);
	ReleaseSRWLockShared(&this->_srwLock);

	if (nullptr == pPlayer)
	{
		SYSLOG(L"PACKET", LOG::LEVEL_ERROR, L"do not found player");
		return false;
	}
	else
	{
		pPlayer->_timeoutTick = time(NULL);
		pPlayer->_accountNo = iAccountNo;
		pPlayer->_sessionKey = sessionKey;

		CPacket *pSendPacket = CPacket::Alloc();
		MakePacket_NewClientLogin(pSendPacket, pPlayer->_accountNo, pPlayer->_sessionKey, clientID);
		//SYSLOG(L"LOG", LOG::LEVEL_ERROR, L"AccountNo : %05d, clientID : 0x%016x", iAccountNo, EXTRACTCLIENTID(clientID));
		this->_lanserver_Login->SendPacket_ServerGroup(1, pSendPacket);
		pSendPacket->Free();

		InterlockedIncrement(&this->_Monitor_LoginWait);
		return true;
	}
}

bool CLoginServer::ResponseCheck(__int64 accountNo, int serverType)
{
	st_PLAYER *pPlayer = nullptr;
	AcquireSRWLockShared(&this->_srwLock);
	for (auto iter = _playerMap.begin(); iter != _playerMap.end(); ++iter)
	{
		pPlayer = iter->second;
		if (accountNo == pPlayer->_accountNo)
			break;
	}
	ReleaseSRWLockShared(&this->_srwLock);

	if (nullptr != pPlayer)
	{
		switch (serverType)
		{
			case dfSERVER_TYPE_GAME:
				pPlayer->_bGameServerRecv = true;
				break;

			case dfSERVER_TYPE_CHAT:
				pPlayer->_bChatServerRecv = true;
				break;
				
			default:
				SYSLOG(L"PACKET", LOG::LEVEL_DEBUG, L"wrong servertype recv");
				return false;
		}

		this->SendPacket_ResponseLogin(pPlayer, dfLOGIN_STATUS_OK);
		return true;

		//if (true == pPlayer->_bChatServerRecv)// && true == pPlayer->_bGameServerRecv)
		//{
		//	
		//	
		//}
	}
	else
	{
		SYSLOG(L"ERROR", LOG::LEVEL_DEBUG, L"Found Player is not exist");
		return false;
	}
}


void CLoginServer::SendPacket_ResponseLogin(st_PLAYER *pPlayer, BYTE byState)
{
	CPacket *pPacket = CPacket::Alloc();
	MakePacket_ResLogin(pPacket, pPlayer->_accountNo, L"", L"", byState);

	SendPacket(pPlayer->_clientID, pPacket);
	pPacket->Free();
}


void CLoginServer::MakePacket_NewClientLogin(CPacket *pSendPacket, __int64 iAccountNo, char *sessionKey, __int64 iParam)
{
	// LoginServer가 GameServer, ChatServer에게 보냄
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_REQ_NEW_CLIENT_LOGIN;
	*pSendPacket << iAccountNo;
	pSendPacket->Enqueue(sessionKey, 64);
	*pSendPacket << iParam;

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(HEADER));
	return;
}

void CLoginServer::MakePacket_ResLogin(CPacket *pSendPacket, __int64 iAccountNo, WCHAR *szID, WCHAR *szNickname, BYTE byStatus)
{
	// LoginServer가 클라이언트에게 보냄
	// 로그인 요청에 대한 응답

	pSendPacket->SetHeaderSize(5);
	*pSendPacket << (WORD)en_PACKET_CS_LOGIN_RES_LOGIN;
	*pSendPacket << iAccountNo;
	*pSendPacket << byStatus;

	// 로그인 실패일때는 나머지를 넣어줘야 하는가?

	// 여기에 DB가 들어가야함.
	// ID넣고
	// Nickname 넣고
	wchar_t id[20] = { 0, };
	pSendPacket->Enqueue((char *)id, dfID_LEN * 2); // ID
	pSendPacket->Enqueue((char *)id, dfNICK_LEN * 2); // NICK

	st_SERVER_LINK_INFO *pInfo = &g_ConfigData._serverLinkInfo[0];

	if (iAccountNo < dfDUMMY_ACCOUNTNO_MAX)
	{
		pSendPacket->Enqueue((char *)pInfo->_gameServerIP_Dummy, dfSERVER_ADDR_LEN * 2);
		*pSendPacket << (WORD)pInfo->_gameServerPort_Dummy;
		pSendPacket->Enqueue((char *)pInfo->_chatServerIP_Dummy, dfSERVER_ADDR_LEN * 2);
		*pSendPacket << (WORD)pInfo->_chatServerPort_Dummy;
	}
	else
	{
		pSendPacket->Enqueue((char *)pInfo->_gameServerIP, dfSERVER_ADDR_LEN * 2);
		*pSendPacket << (WORD)pInfo->_gameServerPort;
		pSendPacket->Enqueue((char *)pInfo->_chatServerIP, dfSERVER_ADDR_LEN * 2);
		*pSendPacket << (WORD)pInfo->_chatServerPort;
	}

	BYTE szPacketContent[165] = { 0, };
	memcpy_s(szPacketContent, 165, pSendPacket->GetBuffPtr(), 165);
	SYSLOG(L"DEBUG", LOG::LEVEL_ERROR, L"%x", szPacketContent);

	return;
}

unsigned __stdcall CLoginServer::UpdateThreadFunc(void *lpParam)
{
	CLoginServer *pServer = (CLoginServer *)lpParam;
	return pServer->UpdateThread_update();
}

bool CLoginServer::UpdateThread_update(void)
{
	__int64 updateTick = time(NULL);
	__int64 currTick = 0;

	while (true)
	{
		currTick = time(NULL);

		// Lock걸고 Disconnect하지마라. SRWLock은 분명 데드락이 걸릴 것이다.
		if (currTick - updateTick > dfUPDATE_TICK)
		{
			/*AcquireSRWLockExclusive(&_srwLock);

			for (auto iter = this->_playerMap.begin(); iter != this->_playerMap.end(); ++iter)
			{
				if (currTick - iter->second->_timeoutTick > dfPLAYER_TIMEOUT_TICK)
				{
					ClientDisconnect(iter->second->_clientID);
					SYSLOG(L"PLAYER", LOG::LEVEL_DEBUG, L"[%d]Player disconnected : timeout", iter->second->_accountNo);
				}
			}

			ReleaseSRWLockExclusive(&_srwLock);*/
			updateTick = time(NULL);
		}
		else
		{
			Sleep(50);
		}
	}

	return true;
}

unsigned __stdcall CLoginServer::MonitorTPSThreadFunc(void *lpParam)
{
	CLoginServer *pServer = (CLoginServer *)lpParam;
	return pServer->MonitorTPSThread_update();
}

bool CLoginServer::MonitorTPSThread_update(void)
{
	while (true)
	{
		Sleep(998);

		_Monitor_LoginSuccessTPS = _Monitor_LoginSuccessCounter;
		
		_Monitor_LoginSuccessCounter = 0;
	}
	return true;
}


int CLoginServer::GetLanSendPacketCount(void)
{
	return this->_lanserver_Login->_iTotalSendPacket;
}

int CLoginServer::GetLanRecvPacketCount(void)
{
	return this->_lanserver_Login->_iTotalRecvPacket;
}