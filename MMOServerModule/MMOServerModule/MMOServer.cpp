#include "stdafx.h"
#include "DBTypeEnum.h"
#include "MMOServer.h"

CMMOServer::CMMOServer(int iClientMax)
{
	_bStart = false;
	_bStop = false;

	_iClientMax = iClientMax;
	_iClientID = 0;

	_listenSock = INVALID_SOCKET;
	_hIOCP = INVALID_HANDLE_VALUE;

	_hAcceptThread = INVALID_HANDLE_VALUE;
	_hAuthThread = INVALID_HANDLE_VALUE;
	_hWorkerThread = NULL;
	_hSendThread = INVALID_HANDLE_VALUE;
	_hGameThread = INVALID_HANDLE_VALUE;

	_pSessionArray = new CSession*[_iClientMax];
}

CMMOServer::~CMMOServer()
{
	delete[] this->_pSessionArray;
}

bool CMMOServer::Start(wchar_t *szIP, int iPort, bool bNagleOpt, int iThreadNum)
{
	if (true == this->_bStart)
		return true;

#pragma region print_serverinfo
	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L" ############## GAME SERVER START ##############");
	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L" ##\t    ServerBindIP : %s", szIP);
	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L" ##\t  ServerBindPort : %d", iPort);
	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L" ##\t        NagleOpt : %s", bNagleOpt == true? L"TRUE" : L"FALSE");
	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L" ##\t   Thread Amount : %d", iThreadNum);
	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L" ##\t       ClientMax : %d", _iClientMax);
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

	Session_Init();
	Thread_Init(iThreadNum);

	_bStart = true;
	return true;
}

bool CMMOServer::Stop()
{
	_bStop = true;
	closesocket(_listenSock);

	PostQueuedCompletionStatus(_hIOCP, 0, NULL, NULL);

	// 스레드 정리
	//HANDLE 배열 세팅 
	//WaitForMultipleObjects()

	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"############ GAME SERVER STOP ############");
	_bStart = false;
	return true;
}

bool CMMOServer::Session_Init(void)
{
	// 비어있는 Session Index 세팅
	for (int i = _iClientMax - 1; i >= 0; i--)
		_sessionIndexStack.Push(i);

	return true;
}

bool CMMOServer::SetSessionArray(int index, void *pSession)
{
	CSession *pResistSession = (CSession *)pSession;
	this->_pSessionArray[index] = pResistSession;
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

	

	return true;
}

unsigned __stdcall CMMOServer::AcceptThreadFunc(void *lpParam)
{
	CMMOServer *pServer = (CMMOServer *)lpParam;
	return pServer->AcceptThread_update();
}

bool CMMOServer::AcceptThread_update(void)
{
	int addrLen = sizeof(SOCKADDR);
	st_ACCEPT_CLIENT_INFO *pClientConnectInfo = NULL;

	if (SOCKET_ERROR == listen(_listenSock, SOMAXCONN))
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"listen() Error. ErrorNo : %d", WSAGetLastError());
		return false;
	}

	while (true)
	{
		pClientConnectInfo = _clientInfoPool.Alloc();
		pClientConnectInfo->_clientSock = accept(_listenSock, (SOCKADDR *)&pClientConnectInfo->_clientAddr, &addrLen);
		_iAcceptTotal++;

		if (INVALID_SOCKET == pClientConnectInfo->_clientSock)
		{
			_clientInfoPool.Free(pClientConnectInfo);

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

		_clientInfoQueue.Enqueue(pClientConnectInfo);
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
	CSession *pSession = NULL;
	int iBlankIndex = INVALID_SESSION_INDEX;
	int iProcessCounter;

	iProcessCounter = 0;
	while (0 < _clientInfoQueue.GetUseSize())
	{
		_clientInfoQueue.Dequeue(&pClientConnectInfo);

		if (NULL == pClientConnectInfo)
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"ClientInfo is null in AuthThread");
			CCrashDump::Crash();
		}

		this->_sessionIndexStack.Pop(&iBlankIndex);

		if (INVALID_SESSION_INDEX == iBlankIndex)
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_DEBUG, L"Session index is FULL");
			closesocket(pClientConnectInfo->_clientSock);
			this->_clientInfoPool.Free(pClientConnectInfo);
			continue;
		}

		pSession = this->_pSessionArray[iBlankIndex];

		if (NULL == pSession)
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"not found session");
			CCrashDump::Crash();
		}

		pSession->_iSessionMode = CSession::MODE_AUTH;
		pSession->_clientID = MAKECLIENTID(iBlankIndex, _iClientID);
		pSession->_connectInfo = pClientConnectInfo;
		pSession->_iSendCount = 0;

		if (this->_hIOCP != CreateIoCompletionPort((HANDLE)pClientConnectInfo->_clientSock, this->_hIOCP, (ULONG_PTR)pSession, 0))
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"CreateIoCompletionPort() Error. ErrorNo : %d", GetLastError());
			closesocket(pClientConnectInfo->_clientSock);
			CCrashDump::Crash();
		}

		InterlockedIncrement(&pSession->_IOCount);
		pSession->OnAuth_ClientJoin();
		pSession->RecvPost(false);

		_iClientID++;
		_iAuthThSessionCounter++;
		InterlockedIncrement(&this->_iTotalSessionCounter);

		iProcessCounter++;
		if (200 <= iProcessCounter)
		{
			iProcessCounter = 0;
			break;
		}
	}

	for (int i = 0; i < _iClientMax; ++i)
	{
		if (NULL != (this->_pSessionArray[i]))
		{
			pSession = (this->_pSessionArray[i]);

			if (CSession::MODE_AUTH == pSession->_iSessionMode)
			{
				pSession->OnAuth_PacketProc();

				if (true == pSession->_bLogout && FALSE == pSession->_iSending)
					pSession->_iSessionMode = CSession::MODE_LOGOUT_IN_AUTH;
			}
		}
		else
			continue;
	}

	OnAuth_Update();

	/*for (int i = 0; i < _iClientMax; ++i)
	{
		if (NULL != (this->_pSessionArray[i]))
		{
			pSession = (this->_pSessionArray[i]);

			if (CSession::MODE_AUTH == pSession->_iSessionMode && true == pSession->_bLogout)
				pSession->_iSessionMode = CSession::MODE_LOGOUT_IN_AUTH;
		}
		else
			continue;
	}*/

	// 세션 배열을 돌면서 MODE_LOGOUT_IN_AUTH 인 얘들을 WAIT_LOGOUT으로 변경
	// 후 OnAuth_ClientLeave(false) 호출
	for (int i = 0; i < _iClientMax; ++i)
	{
		if (NULL != (this->_pSessionArray[i]))
		{
			pSession = (this->_pSessionArray[i]);

			if (CSession::MODE_LOGOUT_IN_AUTH == pSession->_iSessionMode && FALSE == pSession->_iSending)
			{
				pSession->_iSessionMode = CSession::MODE_WAIT_LOGOUT;
				pSession->OnAuth_ClientLeave(false);
				_iAuthThSessionCounter--;
			}
			else if (true == pSession->_bAuthToGame && CSession::MODE_AUTH == pSession->_iSessionMode)
			{
				pSession->_iSessionMode = CSession::MODE_AUTH_TO_GAME;
				pSession->OnAuth_ClientLeave(true);
				_iAuthThSessionCounter--;
			}
		}
		else
			continue;
	}

	this->_iAuthThLoopCounter++;
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
	CSession *pSession = NULL;
	long IOCount = 0;
	long iRecvPacketCount;

	while (true)
	{
		dwTransferedBytes = 0;
		ZeroMemory(overlap, sizeof(OVERLAPPED));

		bGQCSResult = GetQueuedCompletionStatus(this->_hIOCP, &dwTransferedBytes, (PULONG_PTR)&pSession, &overlap, INFINITE);

		// 종료 메시지
		if (NULL == overlap && 0 == dwTransferedBytes && NULL == pSession)
		{
			PostQueuedCompletionStatus(_hIOCP, 0, NULL, NULL);
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Thread %d is down %d", GetCurrentThreadId(), GetLastError());
			return 0;
		}

		// 하트비트
		/*if (NULL == overlap & 1 == dwTransferedBytes && (CSession *)1 == pSession)
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
				iRecvPacketCount = pSession->CompleteRecv(dwTransferedBytes);
				InterlockedAdd(&this->_iRecvPacketCounter, iRecvPacketCount);
				pSession->RecvPost(true);
			}
			else if (overlap == &pSession->_sendOverlap)
			{
				InterlockedAdd(&this->_iSendPacketCounter, pSession->_iSendCount);
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
	CSession *pSession = NULL;

	for (int i = 0; i < _iClientMax; ++i)
	{
		if (NULL != (this->_pSessionArray[i]))
		{
			pSession = (this->_pSessionArray[i]);
			pSession->SendPost();
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
	CSession *pSession = NULL;

	for (int i = 0; i < _iClientMax; ++i)
	{
		if (NULL != (this->_pSessionArray[i]))
		{
			pSession = (this->_pSessionArray[i]);

			if (CSession::MODE_AUTH_TO_GAME == pSession->_iSessionMode)
			{
				pSession->OnGame_ClientJoin();
				pSession->_iSessionMode = CSession::MODE_GAME;
				_iGameThSessionCounter++;
			}
		}
		else
			continue;
	}

	for (int i = 0; i < _iClientMax; ++i)
	{
		if (NULL != (this->_pSessionArray[i]))
		{
			pSession = (this->_pSessionArray[i]);

			if (CSession::MODE_GAME == pSession->_iSessionMode)
			{
				pSession->OnGame_PacketProc();

				if (true == pSession->_bLogout && FALSE == pSession->_iSending)
					pSession->_iSessionMode = CSession::MODE_LOGOUT_IN_GAME;
			}
		}
		else
			continue;
	}

	OnGame_Update();

	/*for (int i = 0; i < _iClientMax; ++i)
	{
		if (NULL != (this->_pSessionArray[i]))
		{
			pSession = (this->_pSessionArray[i]);

			if (CSession::MODE_GAME == pSession->_iSessionMode && true == pSession->_bLogout)
				pSession->_iSessionMode = CSession::MODE_LOGOUT_IN_GAME;
		}
		else
			continue;
	}*/

	for (int i = 0; i < _iClientMax; ++i)
	{
		if (NULL != (this->_pSessionArray[i]))
		{
			pSession = (this->_pSessionArray[i]);

			if (CSession::MODE_LOGOUT_IN_GAME == pSession->_iSessionMode)
			{
				pSession->_iSessionMode = CSession::MODE_WAIT_LOGOUT;
				this->_iGameThSessionCounter--;
				pSession->OnGame_ClientLeave();
			}
		}
		else
			continue;
	}

	int index;
	for (int i = 0; i < _iClientMax; ++i)
	{
		if (NULL != (this->_pSessionArray[i]))
		{
			pSession = (this->_pSessionArray[i]);

			if (CSession::MODE_WAIT_LOGOUT == pSession->_iSessionMode)
			{
				index = EXTRACTCLIENTINDEX(pSession->_clientID);
				closesocket(pSession->_connectInfo->_clientSock);
				_clientInfoPool.Free(pSession->_connectInfo);
				pSession->OnGame_ClientRelease();
				this->_sessionIndexStack.Push(index);
				
				InterlockedDecrement(&this->_iTotalSessionCounter);
			}
		}
		else
			continue;
	}

	this->_iGameThLoopCounter++;
	return true;
}



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
//	함수이름 : void CSession::CompleteRecv(int recvBytes)
//  인자
//		int recvBytes : WSARecv로 받은 바이트 수. GQCS로 받은 transfered
//
//	streamQ인 _recvQ의 write를 옮겨주고, 완성된 패킷이 있으면 복호화 해서
//	검증이 끝나면 _completeRecvQ에 넣어준다.
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
//	함수이름 : void CSession::CompleteSend(void)
//
//	_sendQ에 있는 패킷에 대해 free() 호출해준다. _iSendCount는 WSASend를
//	호출한 SendThread가 세팅해 줄 것이다. 
//	NetServer와 다르게 SendThread가 있으므로 따로 SendPost는 호출하지 않는다.
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

	if (CSession::MODE_AUTH != this->_iSessionMode &&
		CSession::MODE_GAME != this->_iSessionMode)
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