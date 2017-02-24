#pragma once

typedef __int64 CLIENT_ID;

struct st_ACCEPT_CLIENT_INFO
{
	SOCKET _clientSock;
	SOCKADDR_IN _clientAddr;
};

class CSESSION
{
protected:
	virtual bool OnAuth_ClientJoin() = 0;
	virtual bool OnAuth_PacketProc() = 0;
	virtual bool OnAuth_ClientLeave(bool bToGame) = 0;

	virtual bool OnGame_ClientJoin() = 0;
	virtual bool OnGame_PacketProc() = 0;
	virtual bool OnGame_ClientLeave() = 0;

private:
	CLIENT_ID _clientID;
	st_ACCEPT_CLIENT_INFO *_connectInfo;

	CStreamQueue	_recvQ;
	CLockFreeQueue<CPacket *>	_sendQ;

	OVERLAPPED		_recvOverlap;
	OVERLAPPED		_sendOverlap;

	__int64 _IOCount;
	long	_iSending;
	int		_iSendCount;


};

class CMMOServer
{
public:
	enum en_SESSION_MODE
	{
		MODE_NONE = 0,						// 미 접속 상태
		MODE_AUTH,							// Accept 후 세션 등록
		MODE_AUTH_TO_GAME,					// 인증 처리 후 로그인 완료
		MODE_GAME,							// 인증 모드에서 게임 모드로 전환
		MODE_LOGOUT_IN_AUTH,				// AuthThread에서 close
		MODE_LOGOUT_IN_GAME,				// GameThread에서 close
		MODE_WAIT_LOGOUT,					// 세션 릴리즈
	};

public:
	CMMOServer();
	~CMMOServer();

	bool Start(wchar_t *szIP, int iPort, bool bNagleOpt, int iThreadNum, int iClientMax);
	bool Stop();

private:
	bool Session_Init(int iClientMax);
	bool Thread_Init(int iThreadNum);

private:
	bool _bStop;

	SOCKET _listenSock;
	HANDLE _hIOCP;

	CLockFreeStack<int> _sessionIndexStack;

	CMemoryPool<st_ACCEPT_CLIENT_INFO> _clientInfoPool;
	CLockFreeQueue<st_ACCEPT_CLIENT_INFO *> _clientInfoQueue;

	// 모니터링 함수
public:
	int GetSessionCount(void);
	int GetPlayerCount(void);

	// 모니터링 변수
public:
	int _iSendPacketTPS;
	int _iRecvPacketTPS;

private:
	int _iSendPacketCounter;
	int _iRecvPacketCounter;


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