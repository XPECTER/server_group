#include "stdafx.h"
#include "MMOSession.h"
#include "MMOServer.h"

CMMOServer::CMMOServer()
{
	_bStop = false;

	_iClientMax = 0;
	_iClientID = 0;

	_listenSock = INVALID_SOCKET;
	_hIOCP = INVALID_HANDLE_VALUE;

	_hAcceptThread = INVALID_HANDLE_VALUE;
	_hAuthThread = INVALID_HANDLE_VALUE;
	_hWorkerThread = NULL;
	_hSendThread = INVALID_HANDLE_VALUE;
	_hGameThread = INVALID_HANDLE_VALUE;
	_hMonitorThread = INVALID_HANDLE_VALUE;
}

CMMOServer::~CMMOServer()
{
	delete[] this->_pSessionArray;
}

bool CMMOServer::Start(wchar_t *szIP, int iPort, bool bNagleOpt, int iThreadNum, int iClientMax)
{
#pragma region print_serverinfo
	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L" ##\t    ServerBindIP : %s", szIP);
	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L" ##\t  ServerBindPort : %d", iPort);
	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L" ##\t        NagleOpt : %s", bNagleOpt == true? L"TRUE" : L"FALSE");
	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L" ##\t   Thread Amount : %d", iThreadNum);
	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L" ##\t       ClientMax : %d", iClientMax);
	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L" ##\t      PacketCode : 0x%X", g_Config.iPacketCode);
	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L" ##\t     PacketKey_1 : 0x%X", g_Config.iPacketKey1);
	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L" ##\t     PacketKey_2 : 0x%X", g_Config.iPacketKey2);
	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L" ##\t       LOG_LEVEL : %s", g_Config.iLogLevel == 0? L"LEVEL_DEBUG" : g_Config.iLogLevel == 1? L"LEVEL_WARNG" : L"LEVEL_ERROR");
#pragma endregion print_serverinfo

	WSADATA wsa;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsa))
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"WSAStartUp() Error. ErrorNo : %d", WSAGetLastError());
		return false;
	}

	if (INVALID_SOCKET == (_listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"socket() Error. ErrorNo : %d", WSAGetLastError());
		return false;
	}

#pragma region setsockopt
	linger lingerOpt;
	lingerOpt.l_onoff = 1;
	lingerOpt.l_linger = 0;

	if (setsockopt(_listenSock, SOL_SOCKET, SO_LINGER, (char *)&lingerOpt, sizeof(lingerOpt)))
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"setsockopt() linger Error. ErrorNo : %d", WSAGetLastError());
	}

	if (true == bNagleOpt)
	{
		if (0 != setsockopt(_listenSock, IPPROTO_TCP, TCP_NODELAY, (char *)true, sizeof(bool)))
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"setsockopt() nodelay Error. ErrorNo : %d", WSAGetLastError());
			return false;
		}
	}
#pragma endregion setsockopt

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(iPort);
	InetPton(AF_INET, szIP, &serverAddr.sin_addr.s_addr);

	if (0 != bind(_listenSock, (SOCKADDR *)&serverAddr, sizeof(serverAddr)))
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"bind() Error. ErrorNo : %d", WSAGetLastError());
		return false;
	}

	if (NULL == (_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, iThreadNum)))
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"CreateIoCompletionPort() Error. ErrorNo : %d", WSAGetLastError());
		return false;
	}

	Session_Init(iClientMax);
	Thread_Init(iThreadNum);
	return true;
}

bool CMMOServer::Stop()
{
	_bStop = true;
	closesocket(_listenSock);

	PostQueuedCompletionStatus(_hIOCP, 0, NULL, NULL);

	// 스레드 정리
	//WaitForMultipleObjects()
	return true;
}

bool CMMOServer::Session_Init(int iClientMax)
{
	_iClientMax = iClientMax;
	_pSessionArray = new CSESSION*[iClientMax];

	// 비어있는 Session Index 세팅
	for (int i = iClientMax - 1; i >= 0; i--)
		_sessionIndexStack.Push(i);

	return true;
}

bool CMMOServer::Thread_Init(int iThreadNum)
{
	_hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, AcceptThreadFunc, this, NULL, NULL);
	if (NULL == _hAcceptThread)
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Create AcceptTh failed");
		return false;
	}

	_hAuthThread = (HANDLE)_beginthreadex(NULL, 0, AuthThreadFunc, this, NULL, NULL);
	if (NULL == _hAuthThread)
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Create AuthTh failed");
		return false;
	}

	_hWorkerThread = new HANDLE[iThreadNum];
	for (int i = 0; i < iThreadNum; ++i)
	{
		_hWorkerThread[i] = (HANDLE)_beginthreadex(NULL, 0, WorkerThreadFunc, this, NULL, NULL);
		if (NULL == _hWorkerThread[i])
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Create WorkerTh failed");
			return false;
		}
	}

	_hSendThread = (HANDLE)_beginthreadex(NULL, 0, SendThreadFunc, this, NULL, NULL);
	if (NULL == _hSendThread)
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Create SendTh failed");
		return false;
	}

	_hGameThread = (HANDLE)_beginthreadex(NULL, 0, GameThreadFunc, this, NULL, NULL);
	if (NULL == _hGameThread)
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Create GameTh failed");
		return false;
	}

	_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThreadFunc, this, NULL, NULL);
	if (NULL == _hMonitorThread)
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Create MonitoringTh failed");
		return false;
	}

	return true;
}

void CMMOServer::SendPost(CSESSION *pSession)
{
	if (InterlockedCompareExchange((long *)&pSession->_iSending, TRUE, FALSE))
		return;

	if (CSESSION::MODE_AUTH != pSession->_iSessionMode &&
		CSESSION::MODE_GAME != pSession->_iSessionMode)
	{
		InterlockedExchange((long *)&pSession->_iSending, FALSE);
		return;
	}

	if (0 >= pSession->_sendQ.GetUseSize())
	{
		InterlockedExchange((long *)&pSession->_iSending, FALSE);
		return;
	}

	InterlockedIncrement(&pSession->_IOCount);

	WSABUF wsabuf[200];
	int iPacketCount = min(200, pSession->_sendQ.GetUseSize());
	CPacket *pPacket = NULL;

	for (int i = 0; i < iPacketCount; ++i)
	{
		if (pSession->_sendQ.Peek(&pPacket, i))
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
	ZeroMemory(&pSession->_sendOverlap, sizeof(OVERLAPPED));
	pSession->_iSendCount = iPacketCount;

	if (SOCKET_ERROR == WSASend(pSession->_connectInfo->_clientSock, wsabuf, iPacketCount, NULL, lpFlag, &pSession->_sendOverlap, NULL))
	{
		int iError = WSAGetLastError();
		if (WSA_IO_PENDING != iError)
		{
			if (WSAENOBUFS == iError)
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_WARNING, L"WSAENOBUFS. Check nonpagedpool");
				CCrashDump::Crash();
			}

			pSession->Disconnect();

			if (0 == InterlockedDecrement(&pSession->_IOCount))
			{
				pSession->_bLogout = true;
			}
		}
	}

	return;
}

void CMMOServer::RecvPost(CSESSION *pSession, bool incrementFlag)
{
	WSABUF wsabuf[2];
	int wsabufCount;

	if (TRUE == incrementFlag)
		InterlockedIncrement(&pSession->_IOCount);

	wsabuf[0].len = pSession->_recvQ.GetNotBrokenPutsize();
	wsabuf[0].buf = pSession->_recvQ.GetWriteBufferPtr();
	wsabufCount = 1;

	int isBrokenLen = pSession->_recvQ.GetFreeSize() - pSession->_recvQ.GetNotBrokenPutsize();
	if (0 < isBrokenLen)
	{
		wsabuf[1].len = isBrokenLen;
		wsabuf[1].buf = pSession->_recvQ.GetBufferPtr();
		wsabufCount = 2;
	}

	DWORD lpFlag = 0;

	ZeroMemory(&pSession->_recvOverlap, sizeof(OVERLAPPED));
	if (SOCKET_ERROR == WSARecv(pSession->_connectInfo->_clientSock, wsabuf, wsabufCount, NULL, &lpFlag, &pSession->_recvOverlap, NULL))
	{
		int iError = WSAGetLastError();
		if (WSA_IO_PENDING != iError)
		{
			if (WSAENOBUFS == iError)
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_WARNING, L"WSAENOBUFS. Check nonpagedpool");
				CCrashDump::Crash();
			}

			pSession->Disconnect();

			if (0 == InterlockedDecrement(&pSession->_IOCount))
			{
				pSession->_bLogout = true;
			}
		}
	}

	return;
}

unsigned __stdcall CMMOServer::AcceptThreadFunc(void *lpParam)
{
	CMMOServer *pServer = (CMMOServer *)lpParam;
	return pServer->AcceptThread_update();
}

bool CMMOServer::AcceptThread_update(void)
{
	int addrLen;
	st_ACCEPT_CLIENT_INFO *pClientInfo = NULL;

	if (SOCKET_ERROR == listen(_listenSock, SOMAXCONN))
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"listen() Error. ErrorNo : %d", WSAGetLastError());
		return false;
	}

	while (true)
	{
		pClientInfo = _clientInfoPool.Alloc();
		pClientInfo->_clientSock = accept(_listenSock, (SOCKADDR *)&pClientInfo->_clientAddr, &addrLen);
		_iAcceptTotal++;

		if (INVALID_SOCKET == pClientInfo->_clientSock)
		{
			_clientInfoPool.Free(pClientInfo);

			// 진짜 접속 못 받은 것과 Stop()을 호출해서 종료하는 것을 구분
			if (true == _bStop)
			{
				break;
			}
			else
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"accept() Error. ErrorNo : %d", WSAGetLastError());
				continue;
			}
		}

		_clientInfoQueue.Enqueue(pClientInfo);
		InterlockedIncrement(&_iAcceptCounter);
	}

	return true;
}

unsigned __stdcall CMMOServer::AuthThreadFunc(void *lpParam)
{
	CMMOServer *pServer = (CMMOServer *)lpParam;

	while (!pServer->_bStop)
	{
		pServer->AuthThread_update();
		Sleep(dfTHREAD_UPDATE_TIME_AUTH);
	}

	return 0;
}

bool CMMOServer::AuthThread_update(void)
{
	st_ACCEPT_CLIENT_INFO *pClientConnectInfo = NULL;
	CSESSION *pSession = NULL;
	int iBlankIndex = 0;

	while (_clientInfoQueue.GetUseSize())
	{
		_clientInfoQueue.Dequeue(&pClientConnectInfo);

		if (NULL == pClientConnectInfo)
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"ClientInfo is null in AuthThread");
			CCrashDump::Crash();
		}

		/*if (NULL == CreateIoCompletionPort((HANDLE)pClientInfo->_clientSock, this->_hIOCP, (ULONG_PTR)pSession, 0))
		{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"CreateIoCompletionPort() Error. ErrorNo : %d", WSAGetLastError());
		closesocket(pSession->ClientSock);
		}*/

		this->_sessionIndexStack.Pop(&iBlankIndex);

		if (0 == iBlankIndex)
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_DEBUG, L"Session index is FULL");
			this->_clientInfoPool.Free(pClientConnectInfo);
			continue;
		}

		pSession = this->_pSessionArray[iBlankIndex];

		pSession->_iSessionMode = CSESSION::MODE_AUTH;
		pSession->_clientID = MAKECLIENTID(iBlankIndex, _iClientID);
		pSession->_connectInfo = pClientConnectInfo;
		pSession->_iSendCount = 0;

		InterlockedIncrement(&pSession->_IOCount);
		pSession->OnAuth_ClientJoin();
		RecvPost(pSession, false);

		_iClientID++;
	}

	for (int i = 0; i < _iClientMax; ++i)
	{
		if (NULL != (this->_pSessionArray[i]))
		{
			pSession = (this->_pSessionArray[i]);

			if (CSESSION::MODE_AUTH == pSession->_iSessionMode)
				pSession->OnAuth_PacketProc();
		}
		else
			continue;
	}

	OnAuth_Update();

	return true;
}

unsigned __stdcall CMMOServer::WorkerThreadFunc(void *lpParam)
{
	CMMOServer *pServer = (CMMOServer *)lpParam;
	return pServer->WorkerThread_update();
}

bool CMMOServer::WorkerThread_update(void)
{
	OVERLAPPED *overlap = new OVERLAPPED;
	BOOL bGQCSResult;
	DWORD dwTransferedBytes = 0;
	CSESSION *pSession = NULL;
	long IOCount = 0;

	while (true)
	{
		dwTransferedBytes = 0;
		ZeroMemory(overlap, sizeof(OVERLAPPED));

		bGQCSResult = GetQueuedCompletionStatus(this->_hIOCP, &dwTransferedBytes, (PULONG_PTR)pSession, &overlap, INFINITE);

		// 종료 메시지
		if (NULL == overlap && 0 == dwTransferedBytes && NULL == pSession)
		{
			PostQueuedCompletionStatus(_hIOCP, 0, NULL, NULL);
			return 0;
		}

		// 하트비트
		/*if (NULL == overlap & 1 == dwTransferedBytes && (CSESSION *)1 == pSession)
		{
			continue;
		}*/

		if (NULL != overlap)
		{
			if (0 == dwTransferedBytes)
			{
				pSession->Disconnect();
			}
			else if (overlap == &pSession->_recvOverlap)
			{
				pSession->CompleteRecv(dwTransferedBytes);
				RecvPost(pSession, true);
			}
			else if (overlap == &pSession->_sendOverlap)
			{
				pSession->CompleteSend();
			}
		}

		IOCount = InterlockedDecrement(&pSession->_IOCount);
		if (0 == IOCount)
		{
			pSession->_bLogout = true;
		}
		else if (0 > IOCount)
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_DEBUG, L"IOCount Sync Error");
		}
	}
}

unsigned __stdcall CMMOServer::SendThreadFunc(void *lpParam)
{
	CMMOServer *pServer = (CMMOServer *)lpParam;

	while (!pServer->_bStop)
	{
		pServer->SendThread_update();
		Sleep(dfTHREAD_UPDATE_TIME_SEND);
	}

	return 0;
}

bool CMMOServer::SendThread_update(void)
{
	CSESSION *pSession = NULL;

	for (int i = 0; i < _iClientMax; ++i)
	{
		if (NULL != (this->_pSessionArray[i]))
		{
			pSession = (this->_pSessionArray[i]);
			this->SendPost(pSession);
		}
		else
			continue;
	}

	return true;
}

unsigned __stdcall CMMOServer::GameThreadFunc(void *lpParam)
{
	CMMOServer *pServer = (CMMOServer *)lpParam;

	while (!pServer->_bStop)
	{
		pServer->GameThread_update();
		Sleep(dfTHREAD_UPDATE_TIME_GAME);
	}

	return 0;
}

bool CMMOServer::GameThread_update(void)
{
	CSESSION *pSession = NULL;

	for (int i = 0; i < _iClientMax; ++i)
	{
		if (NULL != (this->_pSessionArray[i]))
		{
			pSession = (this->_pSessionArray[i]);

			if (CSESSION::MODE_AUTH_TO_GAME == pSession->_iSessionMode)
				pSession->OnGame_ClientJoin();
		}
		else
			continue;
	}

	return true;
}

unsigned __stdcall CMMOServer::MonitorThreadFunc(void *lpParam)
{
	CMMOServer *pServer = (CMMOServer *)lpParam;

	while (!pServer->_bStop)
	{
		pServer->MonitorThread_update();
		Sleep(dfTHREAD_UPDATE_TIME_MONITOR);
	}

	return 0;
}

bool CMMOServer::MonitorThread_update(void)
{
	_iAcceptTPS = _iAcceptCounter;
	_iSendPacketTPS = _iSendPacketCounter;
	_iRecvPacketTPS = _iRecvPacketCounter;

	_iAcceptCounter = 0;
	_iSendPacketCounter = 0;
	_iRecvPacketCounter = 0;
	
	return true;
}