#include "stdafx.h"
#include "MMOServer.h"

CMMOServer::CMMOServer()
{
	_bStop = false;

	_listenSock = INVALID_SOCKET;
	_hIOCP = INVALID_HANDLE_VALUE;

	_hAcceptThread = INVALID_HANDLE_VALUE;
	_hAuthThread = INVALID_HANDLE_VALUE;
	_hWorkerThread = nullptr;
	_hSendThread = INVALID_HANDLE_VALUE;
	_hGameThread = INVALID_HANDLE_VALUE;
	_hMonitorThread = INVALID_HANDLE_VALUE;
}

CMMOServer::~CMMOServer()
{

}

bool CMMOServer::Start(wchar_t *szIP, int iPort, bool bNagleOpt, int iThreadNum, int iClientMax)
{
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

	//WaitForMultipleObjects()
	return true;
}

bool CMMOServer::Session_Init(int iClientMax)
{
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

unsigned __stdcall CMMOServer::AcceptThreadFunc(void *lpParam)
{
	CMMOServer *pServer = (CMMOServer *)lpParam;
	return pServer->AcceptThread_update();
}

bool CMMOServer::AcceptThread_update(void)
{
	int addrLen;
	st_ACCEPT_CLIENT_INFO *pClientInfo = nullptr;

	if (SOCKET_ERROR == listen(_listenSock, SOMAXCONN))
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"listen() Error. ErrorNo : %d", WSAGetLastError());
		return false;
	}

	while (true)
	{
		pClientInfo = _clientInfoPool.Alloc();
		pClientInfo->_clientSock = accept(_listenSock, (SOCKADDR *)&pClientInfo->_clientAddr, &addrLen);

		if (INVALID_SOCKET == pClientInfo->_clientSock)
		{
			// 진짜 접속 못 받은 것과 Stop()을 호출해서 종료하는 것을 구분
			if (true == _bStop)
				break;
			else
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"accept() Error. ErrorNo : %d", WSAGetLastError());
				_clientInfoPool.Free(pClientInfo);
				continue;
			}
		}

		_clientInfoQueue.Enqueue(pClientInfo);
	}

	return true;
}

unsigned __stdcall CMMOServer::AuthThreadFunc(void *lpParam)
{
	CMMOServer *pServer = (CMMOServer *)lpParam;
	return pServer->AuthThread_update();
}

bool CMMOServer::AuthThread_update(void)
{
	st_ACCEPT_CLIENT_INFO *pClientInfo = nullptr;

	while (true)
	{
		if (0 < _clientInfoQueue.GetUseSize())
		{
			_clientInfoQueue.Dequeue(&pClientInfo);

			if (nullptr == pClientInfo)
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"ClientInfo is null in AuthThread");
				CCrashDump::Crash();
			}



			/*if (NULL == CreateIoCompletionPort((HANDLE)pClientInfo->_clientSock, this->_hIOCP, (ULONG_PTR)pSession, 0))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"CreateIoCompletionPort() Error. ErrorNo : %d", WSAGetLastError());
				closesocket(pSession->ClientSock);
			}*/
			
		}

		Sleep(2);			// 2 ~ 10ms 사이
	}

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

	while (true)
	{
		dwTransferedBytes = 0;
		ZeroMemory(overlap, sizeof(OVERLAPPED));

		bGQCSResult = GetQueuedCompletionStatus(this->_hIOCP, &dwTransferedBytes, (PULONG_PTR)pSession, &overlap, INFINITE);

		// 
		if (NULL == overlap && 0 == dwTransferedBytes && NULL == pSession)
		{
			PostQueuedCompletionStatus(_hIOCP, 0, NULL, NULL);
			return true;
		}
	}

	return true;
}

unsigned __stdcall CMMOServer::SendThreadFunc(void *lpParam)
{
	CMMOServer *pServer = (CMMOServer *)lpParam;
	return pServer->SendThread_update();
}

bool CMMOServer::SendThread_update(void)
{
	while (true)
	{
		Sleep(1);
	}

	return true;
}

unsigned __stdcall CMMOServer::GameThreadFunc(void *lpParam)
{
	CMMOServer *pServer = (CMMOServer *)lpParam;
	return pServer->GameThread_update();
}

bool CMMOServer::GameThread_update(void)
{
	while (true)
	{
		break;
	}

	return true;
}

unsigned __stdcall CMMOServer::MonitorThreadFunc(void *lpParam)
{
	CMMOServer *pServer = (CMMOServer *)lpParam;
	return pServer->MonitorThread_update();
}

bool CMMOServer::MonitorThread_update(void)
{
	while (true)
	{
		_iSendPacketTPS = _iSendPacketCounter;
		_iRecvPacketTPS = _iRecvPacketCounter;

		_iSendPacketCounter = 0;
		_iRecvPacketCounter = 0;

		Sleep(999);
	}

	return true;
}