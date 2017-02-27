#pragma once

class CMMOServer
{
public:
	enum define
	{
		dfTHREAD_UPDATE_TIME_AUTH = 5,
		dfTHREAD_UPDATE_TIME_GAME = 10,
		dfTHREAD_UPDATE_TIME_SEND = 1,
		dfTHREAD_UPDATE_TIME_MONITOR = 998,
	};

public:
	CMMOServer(int iClientMax);
	virtual ~CMMOServer();

	bool Start(wchar_t *szIP, int iPort, bool bNagleOpt, int iThreadNum);
	bool Stop();

	bool SetSessionArray(int index, CSESSION *pSession);

protected:
	virtual void OnAuth_Update(void) = 0;
	virtual void OnGame_Update(void) = 0;

private:
	bool Session_Init(void);
	bool Thread_Init(int iThreadNum);

	void SendPost(CSESSION *pSession);
	void RecvPost(CSESSION *pSession, bool incrementFlag);

public:
	bool _bStop;

private:
	SOCKET _listenSock;
	HANDLE _hIOCP;

	int _iClientMax;
	unsigned __int64 _iClientID;

	CLockFreeStack<int> _sessionIndexStack;

	CMemoryPool<st_ACCEPT_CLIENT_INFO> _clientInfoPool;
	CLockFreeQueue<st_ACCEPT_CLIENT_INFO *> _clientInfoQueue;

	CSESSION **_pSessionArray;

	//////////////////////////////////////////////////////
	// 모니터링 함수
public:
	int GetSessionCount(void);
	int GetPlayerCount(void);

	//////////////////////////////////////////////////////
	// 모니터링 변수
public:
	unsigned __int64 _iAcceptTotal;
	long _iAcceptTPS;
	long _iSendPacketTPS;
	long _iRecvPacketTPS;

private:
	long _iAcceptCounter;
	long _iSendPacketCounter;
	long _iRecvPacketCounter;

	//////////////////////////////////////////////////////
	// Thread 관련
private:
	HANDLE						_hAcceptThread;
	static unsigned __stdcall	AcceptThreadFunc(void *lpParam);
	bool						AcceptThread_update(void);

	HANDLE						_hAuthThread;
	static unsigned __stdcall	AuthThreadFunc(void *lpParam);
	bool						AuthThread_update(void);
	
	HANDLE						*_hWorkerThread;
	static unsigned __stdcall	WorkerThreadFunc(void *lpParam);
	bool						WorkerThread_update(void);
	
	HANDLE						_hSendThread;
	static unsigned __stdcall	SendThreadFunc(void *lpParam);
	bool						SendThread_update(void);
	
	HANDLE						_hGameThread;
	static unsigned __stdcall	GameThreadFunc(void *lpParam);
	bool						GameThread_update(void);

	HANDLE						_hMonitorThread;
	static unsigned __stdcall	MonitorThreadFunc(void *lpParam);
	bool						MonitorThread_update(void);
};