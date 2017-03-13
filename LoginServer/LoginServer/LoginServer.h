#pragma once

#define dfDUMMY_ACCOUNTNO_MAX 999999

#define dfPLAYER_TIMEOUT_TICK 5000 
#define dfSERVER_TIMEOUT_TICK 30000
#define dfUPDATE_TICK 1000

#define dfADDR_LEN 16
#define dfID_LEN 20
#define dfNICK_LEN 20

class CLanServer_Login;

class CLoginServer : public CNetServer
{
	struct st_PLAYER
	{
		CLIENT_ID _clientID;

		__int64 _accountNo;
		char _sessionKey[64];
		unsigned __int64 _timeoutTick;

		long _bGameServerRecv;
		long _bChatServerRecv;

		long _bRecvFlag;				// �α��� ��Ŷ�� �޾Ҵ��� ����
		long _bSendFlag;				// �α��� ��Ŷ�� ���´��� ����
	};

public:

	CLoginServer();
	virtual ~CLoginServer();

	bool Start(void);

protected: 
	virtual bool OnConnectionRequest(wchar_t *szClientIP, int iPort) override;   // accept ���� ��û�� �Դٴ� ���� ���������� �˷��ְ� IP Table���� ���� �źε� IP�� false. �ƴϸ� true
	
	virtual void OnClientJoin(CLIENT_ID clientID) override;						// ���������� ������ �����ߴٴ� ���� �˷���. WSARecv���� ȣ��
	virtual void OnClientLeave(CLIENT_ID clientID) override;						// ���������� ������ �����ٴ� ���� �˷���. ClientRelease���� ��� �ϴ� �������.
		   
	virtual void OnRecv(CLIENT_ID clientID, CPacket *pPacket) override;			// ��Ŷ ���� �Ϸ� ��
	virtual void OnSend(CLIENT_ID clientID, int sendsize) override;				// ��Ŷ �۽� �Ϸ� ��
	
	virtual void OnWorkerThreadBegin(void) override;									// ��Ŀ������ GQCS �ٷ� �ϴܿ��� ȣ��
	virtual void OnWorkerThreadEnd(void) override;									// ��Ŀ������ 1���� ���� ��

	virtual void OnError(int errorNo, CLIENT_ID clientID, WCHAR *errstr) override;

	//-----------------------------------------------------------
	// �α��μ��� �����췯 ����
	//-----------------------------------------------------------
	static unsigned __stdcall	UpdateThread(void *pParam);

	void				Schedule_PlayerTimeout(void);
	void				Schedule_ServerTimeout(void);

public:

	bool	ResponseCheck(__int64 accountNo, int serverType);

	/////////////////////////////////////////////////////////////
	// ����� �����Լ���.  
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// OnClientJoin, OnClientLeave ���� ȣ���.
	/////////////////////////////////////////////////////////////
	bool				InsertPlayer(CLIENT_ID ClientID);
	bool				RemovePlayer(CLIENT_ID ClientID);

	/////////////////////////////////////////////////////////////
	// OnRecv ���� �α������� ó�� �� ���,  UpdateThread ���� �����ð� ���� ������ ���.
	/////////////////////////////////////////////////////////////
	bool				DisconnectPlayer(CLIENT_ID ClientID);

	/////////////////////////////////////////////////////////////
	// ���� ����� �� 
	/////////////////////////////////////////////////////////////
	int					GetSessionCount(void) { return this->_playerPool.GetUseCount(); }
	int					GetPlayerAllocCount(void) { return this->_playerPool.GetAllocCount(); }
	int					GetPlayerUseCount(void) { return this->_playerPool.GetUseCount(); }
	int					GetLanSendPacketCount(void);
	int					GetLanRecvPacketCount(void);

	/////////////////////////////////////////////////////////////
	// White IP ����
	/////////////////////////////////////////////////////////////

	//..WhiteIP ����� �����ϱ� �˻�, Ȯ�� �� �� �ִ� ���


private:
	st_PLAYER* FindPlayer(CLIENT_ID clientID);

	void SendPacket_ResponseLogin(st_PLAYER *pPlayer, BYTE byState);

protected:

	// �α��� ��û ��Ŷó��
	bool				PacketProc_ReqLogin(CLIENT_ID ClientID, CPacket *pPacket);

	// ��Ŷ ������
	void				MakePacket_ResLogin(CPacket *pPacket, __int64 iAccountNo, WCHAR *szID, WCHAR *szNickname, BYTE byStatus);
	void				MakePacket_NewClientLogin(CPacket *pPacket, __int64 iAccountNo, char *sessionKey, __int64 iParam);


protected:

	//-------------------------------------------------------------
	// ������ ����.
	// 
	// �����ڴ� �� ����Ʈ�� ������.  ������ ����ȭ�� ���� SRWLock �� ����Ѵ�.
	//-------------------------------------------------------------
	//list<CPlayer *>			_PlayerList;
	SRWLOCK				_PlayerList_srwlock;


	//-------------------------------------------------------------
	// ��Ÿ ����͸��� ����,������ �Լ�.
	//-------------------------------------------------------------
public :
	long				_Monitor_LoginSuccessTPS;
	long				_Monitor_UpdateTPS;
	long				_Monitor_LoginWait;					// �α��� ��Ŷ ���� �� ������� ��

	long long			_Monitor_LoginProcessTime_Max;		// �α��� ó�� �ð� �ִ�
	long long			_Monitor_LoginProcessTime_Min;		// �α��� ó�� �ð� �ּ�
	long long			_Monitor_LoginProcessTime_Total;	// �� ��
	long long			_Monitor_LoginProcessCall_Total;	// �α��� ó�� ��û �� ��

	long long			_OnSendCallCount;
	double				_Monitor_LoginProcessTime_Avg;

	double				GetLoginProcessAvg(void);

	
private:

	long				_Monitor_UpdateCounter;
	long				_Monitor_LoginSuccessCounter;

	// LanServer (connected to GameServer, ChatServer)
	CLanServer_Login	*_lanserver_Login;

	std::map<CLIENT_ID, st_PLAYER *> _playerMap;
	CMemoryPool<st_PLAYER> _playerPool;

	SRWLOCK _srwLock;

	// ������ ����
private:
	// ����͸� ������
	HANDLE				_hMonitorTPSThread;
	static unsigned __stdcall	MonitorTPSThreadFunc(void *lpParam);
	bool				MonitorTPSThread_update(void);

	// ������Ʈ ������
	HANDLE				_hUpdateThread;
	static unsigned __stdcall	UpdateThreadFunc(void *lpParam);
	bool				UpdateThread_update(void);
};
