#pragma once

#define MAKECLIENTID(index, id) (((__int64)index << 48) | id)
#define EXTRACTCLIENTINDEX(ClientID) ((__int64)ClientID >> 48)
#define EXTRACTCLIENTID(ClientID) ((__int64)ClientID & 0x00FFFFFF)

#define INVALID_SESSION_INDEX -1

#define dfTHREAD_UPDATE_TICK_AUTH 3
#define dfTHREAD_UPDATE_TICK_GAME 10
#define dfTHREAD_UPDATE_TICK_SEND 1

struct st_ACCEPT_CLIENT_INFO
{
	SOCKET _clientSock;
	SOCKADDR_IN _clientAddr;
};

class CMMOServer
{
public:
	enum define
	{
		dfTHREAD_UPDATE_TIME_AUTH = 3,
		dfTHREAD_UPDATE_TIME_GAME = 10,
		dfTHREAD_UPDATE_TIME_SEND = 1,
	};

protected:
	class CSession
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
		bool SendPacket(CPacket *pSendPacket);
		bool Disconnect(void);
		void SetMode_Game(void);

		long CompleteRecv(int recvBytes);
		void CompleteSend(void);


	protected:
		virtual bool OnAuth_ClientJoin(void) = 0;
		virtual bool OnAuth_PacketProc(void) = 0;
		virtual bool OnAuth_ClientLeave(bool bToGame) = 0;

		virtual bool OnGame_ClientJoin(void) = 0;
		virtual bool OnGame_PacketProc(void) = 0;
		virtual bool OnGame_ClientLeave(void) = 0;
		virtual bool OnGame_ClientRelease(void) = 0;

	private:
		void SendPost(void);
		void RecvPost(bool incrementFlag);

	public:
		CLIENT_ID _clientID;
		st_ACCEPT_CLIENT_INFO *_connectInfo;

		CStreamQueue	_recvQ;
		CLockFreeQueue<CPacket *>	_sendQ;
		CLockFreeQueue<CPacket *>	_completeRecvQ;

		int		_iSessionMode;
		bool	_bAuthToGame;
		bool	_bLogout;

		OVERLAPPED		_recvOverlap;
		OVERLAPPED		_sendOverlap;

		long	_IOCount;
		long	_iSending;
		int		_iSendCount;

		friend class CMMOServer;
	};
	
	

public:
	CMMOServer(int iClientMax);
	virtual ~CMMOServer();

	bool Start(wchar_t *szIP, int iPort, bool bNagleOpt, int iThreadNum);
	bool Stop();

protected:
	virtual bool SetSessionArray(int index, void *pSession);

	virtual void OnAuth_Update(void) = 0;
	virtual void OnGame_Update(void) = 0;
	virtual void OnHeartBeat(void) = 0;

	void SendPacket_BraodCast(CPacket *pSendPacket);

private:
	bool Session_Init(void);
	bool Thread_Init(int iThreadNum);

public:
	bool _bStop;

protected:
	// 연결받을 클라이언트 최대 수치
	int _iClientMax;

private:
	// 실행중인가
	bool _bStart;

	// 서버 listen소켓
	SOCKET _listenSock;

	// IOCP 핸들
	HANDLE _hIOCP;

	// 클라이언트에게 부여할 unique번호
	unsigned __int64 _iClientID;

	// 클라이언트 연결 정보를 관리할 풀
	CMemoryPool<st_ACCEPT_CLIENT_INFO> _clientInfoPool;

	// 클라이언트 연결 정보를 AuthTh에게 넘겨주기 위한 큐
	CLockFreeQueue<st_ACCEPT_CLIENT_INFO *> _clientInfoQueue;

	// 세션 포인터를 저장할 배열
	CSession **_pSessionArray;

	// 세션 포인터 배열의 인덱스를 저장
	CLockFreeStack<int> _sessionIndexStack;

	//////////////////////////////////////////////////////
	// 모니터링 변수
	//////////////////////////////////////////////////////
public:
	unsigned __int64 _iAcceptTotal;		// 현재까지 Accept받은 수
	long _iAcceptTPS;					// 초당 Accept 횟수
	long _iSendPacketTPS;				// 초당 Send한 Packet 수
	long _iRecvPacketTPS;				// 초당 Recv한 Packet 수

	long _iTotalSessionCounter;			// 현재 사용중인 세션 수

	long _iAuthThLoopTPS;				// 초당 AuthTh 루프 횟수
	long _iAuthThSessionCounter;		// Auth모드 세션 수

	long _iGameThLoopTPS;				// 초당 GameTh 루프 횟수
	long _iGameThSessionCounter;		// Game모드 세션 수
	
protected:
	long _iAcceptCounter;				// Accept 횟수 카운터
	long _iSendPacketCounter;			// Send한 Packet 수 카운터
	long _iRecvPacketCounter;			// Recv한 Packet 수 카운터
	
	long _iAuthThLoopCounter;			// AuthTh 루프 수 카운터
	long _iGameThLoopCounter;			// AuthTh 루프 수 카운터

	//////////////////////////////////////////////////////
	// Thread 관련
	//////////////////////////////////////////////////////
private:
	// Accept용 스레드
	HANDLE						_hAcceptThread;
	static unsigned __stdcall	AcceptThreadFunc(void *lpParam);
	bool						AcceptThread_update(void);

	// 인증 처리 스레드
	HANDLE						_hAuthThread;
	static unsigned __stdcall	AuthThreadFunc(void *lpParam);
	bool						AuthThread_update(void);
	
	// IOCP 처리 스레드
	HANDLE						*_hWorkerThread;
	static unsigned __stdcall	WorkerThreadFunc(void *lpParam);
	bool						WorkerThread_update(void);
	
	// Send용 스레드
	HANDLE						_hSendThread;
	static unsigned __stdcall	SendThreadFunc(void *lpParam);
	bool						SendThread_update(void);
	
	// 게임 컨텐츠 처리 스레드
	HANDLE						_hGameThread;
	static unsigned __stdcall	GameThreadFunc(void *lpParam);
	bool						GameThread_update(void);
};