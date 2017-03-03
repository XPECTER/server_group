#pragma once

typedef __int64 CLIENT_ID;

#define MAKECLIENTID(index, id) (((__int64)index << 48) | id)
#define EXTRACTCLIENTINDEX(ClientID) ((__int64)ClientID >> 48)
#define EXTRACTCLIENTID(ClientID) ((__int64)ClientID & 0x00FFFFFF)

#define INVALID_SESSION_INDEX -1
class CMMOServer
{
public:
	enum define
	{
		dfTHREAD_UPDATE_TIME_AUTH = 3,
		dfTHREAD_UPDATE_TIME_GAME = 10,
		dfTHREAD_UPDATE_TIME_SEND = 1,
		dfTHREAD_UPDATE_TIME_MONITOR = 998,
	};

protected:
	struct st_ACCEPT_CLIENT_INFO
	{
		SOCKET _clientSock;
		SOCKADDR_IN _clientAddr;
	};

	class CSession
	{
	public:
		enum en_SESSION_MODE
		{
			MODE_NONE = 0,						// �� ���� ����
			MODE_AUTH,							// Accept �� ���� ���
			MODE_AUTH_TO_GAME,					// ���� ó�� �� �α��� �Ϸ�
			MODE_GAME,							// ���� ��忡�� ���� ���� ��ȯ
			MODE_LOGOUT_IN_AUTH,				// AuthThread���� close
			MODE_LOGOUT_IN_GAME,				// GameThread���� close
			MODE_WAIT_LOGOUT,					// ���� ������
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

private:
	bool Session_Init(void);
	bool Thread_Init(int iThreadNum);

	

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

	CSession **_pSessionArray;

	//////////////////////////////////////////////////////
	// ����͸� �Լ�
public:
	int GetSessionCount(void);
	int GetPlayerCount(void);

	//////////////////////////////////////////////////////
	// ����͸� ����
public:
	unsigned __int64 _iAcceptTotal;
	long _iAcceptTPS;
	long _iSendPacketTPS;
	long _iRecvPacketTPS;

	long _iTotalSessionCounter;

	long _iAuthThLoopTPS;
	long _iAuthThSessionCounter;

	long _iGameThLoopTPS;
	long _iGameThSessionCounter;
	
private:
	long _iAcceptCounter;
	long _iSendPacketCounter;
	long _iRecvPacketCounter;
	
	long _iAuthThLoopCounter;
	long _iGameThLoopCounter;

	//////////////////////////////////////////////////////
	// Thread ����
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