#pragma once

#include "MMOServer.h"
#include "LanClient_Login.h"
#include "LanClient_Agent.h"
#include "LanClient_Monitoring.h"
#include "Field.h"
#include "JumpPointSearch.h"
#include "DBConnector.h"

#define dfAUTH_PACKET_PROC_REPEAT 1
#define dfGAME_PACKET_PROC_REPEAT 3

#define dfTHREAD_UPDATE_TICK_MONITOR 999
#define dfTHREAD_UPDATE_TICK_DATABASE 5

#define dfSESSION_KEY_LEN 64

#define dfSESSION_KEY_TIMEOUT_TICK 20000
#define dfGAMETHREAD_HEARTBEAT_TICK 3000
#define dfDATABASETHREAD_HEARTBEAT_TICK 3000
#define dfCLIENT_HEARTBEAT_TICK 40000


struct st_SESSION_KEY
{
	char	_sessionKey[64];
	__int64	_timeoutTick;
};

class CGameServer : public CMMOServer
{
protected:
	class CPlayer : public CMMOServer::CSession
	{
	public:
		CPlayer();
		virtual ~CPlayer();

		void Player_Init(CGameServer *pGameServer);
		void CheckHeartBeat(void);

	protected:
		virtual bool OnAuth_ClientJoin(void) override;
		virtual bool OnAuth_PacketProc(void) override;
		virtual bool OnAuth_ClientLeave(bool bToGame) override;

		virtual bool OnGame_ClientJoin(void) override;
		virtual bool OnGame_PacketProc(void) override;
		virtual bool OnGame_ClientLeave(void) override;
		virtual bool OnGame_ClientRelease(void) override;

		// ��Ŷ ó����
	private:
		//////////////////////////////////////////////////////////
		// �α��� ��Ŷ ó����
		void PacketProc_Login(CPacket *pRecvPacket);

		// �α��� ��Ŷ ó���ο��� ���� ��Ŷ ���� �� ���
		void MakePacket_ResLogin(CPacket *pSendPacket, BYTE byStatus, __int64 iAccountNo);
		//////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////
		// ĳ���� ���� ��Ŷ ó����
		void PacketProc_CharacterSelect(CPacket *pRecvPacket);

		// ĳ���� ���� ��Ŷ ó���ο��� ���� ��Ŷ ���� �� ���
		void MakePacket_ResCharacterSelect(CPacket *pSendPacket, BYTE byStatus);
		//////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////
		// ���� ��Ŷ ó����
		void PacketProc_ReqEcho(CPacket *pRecvPacket);

		// ���� ��Ŷ ó���ο��� ���� ��Ŷ ���� �� ���
		void MakePacket_ResEcho(CPacket *pSendPacket, __int64 iAccountNo, __int64 SendTick);
		//////////////////////////////////////////////////////////

		// ��Ʈ��Ʈ ó��
		void PacketProc_ClientHeartBeat(CPacket *pRecvPacket);
	
		//////////////////////////////////////////////////////////
		// ĳ���� ���� ��Ŷ ������
		void SendPacket_CreateCharacter(void);

		// ĳ���� ���� ��Ŷ �����
		void MakePacket_CreateCharacter(CPacket *pSendPacket, __int64 clientID, BYTE characterType, wchar_t *szNickname, float PosX, float PosY, WORD rotation, int hp, BYTE party);
		//////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////
		// ĳ���� �̵� ��Ŷ ó����
		void PacketProc_MoveCharacter(CPacket *pRecvPacket);

		// ĳ���� �̵� ��Ŷ ó���ο��� ���� ��Ŷ ���� �� ���
		void MakePacket_ResMoveCharacter(CPacket *pSendPacket, __int64 clientID, BYTE pathCount, PATH *pPath);
		//////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////
		// ĳ���� ���� ��û ��Ŷ ó����
		void PacketProc_StopCharacter(CPacket *pRecvPacket);

		// ĳ���� ���� ��û ���� ��Ŷ 
		void MakePacket_StopCharacter(CPacket *pSendPacket, __int64 clientID, float PosX, float PosY, WORD dir);
		//////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////
		// Sync ��Ŷ ������
		void SendPacket_Sync(void);

		// Sync ��Ŷ �����
		void MakePacket_Sync(CPacket *pSendPacket, __int64 clientID, short tileX, short tileY);
		//////////////////////////////////////////////////////////

		// ��Ÿ �ʿ��� ����
	private:
		bool CheckErrorRange(float PosX, float PosY);

	private:
		CGameServer *_pGameServer;
		
		__int64 _accountNo;
		__int64 _heartBeatTick;

		// AuthTh���� ����
		wchar_t	_szNickname[dfNICK_MAX_LEN];			// �г���
		
		BYTE	_byParty;				// ��Ƽ ��ȣ (1�� �Ǵ� 2��)
		BYTE	_characterType;			// ĳ���� Ÿ�� (1�� ��Ƽ�� 1, 2, 3 / 2�� ��Ƽ�� 3, 4, 5)
		

		// GameTh���� ����
		short	_serverX;					// ���� X ��ǥ
		short	_serverY;					// ���� Y ��ǥ
		float	_clientPosX;				// Ŭ���̾�Ʈ X ��ǥ
		float	_clientPosY;				// Ŭ���̾�Ʈ Y ��ǥ
		short	_sectorX;					// ���� X ��ǥ
		short	_sectorY;					// ���� Y ��ǥ

		WORD	_rotation;					// ����

		int		_iHP;						// ü��

		PATH	_path[dfPATH_POINT_MAX];	// �̵��� ���
	};

public:
	CGameServer(int iClientMax);
	~CGameServer();

	bool Start(void);
	void Stop(void);

	/////////////////////////////////////////////////////////////
	// ����Ű ����
	/////////////////////////////////////////////////////////////
	void AddSessionKey(__int64 accountNo, char *sessionKey);
	bool CheckSessionKey(__int64 accountNo, char *sessionKey);

protected:
	virtual void OnAuth_Update(void) override;
	virtual void OnGame_Update(void) override;
	virtual void OnHeartBeat(void) override;

private:
	void Schedule_SessionKey(void);
	void Schedule_Client(void);

private:
	CLanClient_Login *_lanClient_Login;
	CLanClient_Agent *_lanClient_Agent;
	CLanClient_Monitoring *_lanClient_Monitoring;

	// ���� �÷��̾ ������ �迭
	CPlayer *pPlayerArray;

	// �α��� �����κ��� ���� Ű �ڷᱸ��
	std::map<__int64, st_SESSION_KEY *> _sessionKeyMap;
	SRWLOCK _sessionKeyMapLock;

	// ��Ʈ��Ʈ��
	__int64 _updateTick;

	// �����ͺ��̽�
	AccountDB	*_database_Account;
	GameDB		*_database_Game;
	LogDB		*_database_Log;

	// �����ͺ��̽� �޽��� Ǯ
	CMemoryPool<st_DBWRITER_MSG> _databaseMsgPool;

	// �����ͺ��̽� �޽��� ť
	CLockFreeQueue<st_DBWRITER_MSG *> _databaseMsgQueue;

	// ��ã�� 
	CJumpPointSearch *_jps;

	//����͸� �뵵
public:
	long _iDatabaseWriteTPS;

private:
	long _iDatabaseWriteCounter;

private:
	// ����͸� ������
	HANDLE						_hMonitorThread;
	static unsigned __stdcall	MonitorThreadFunc(void *lpParam);
	bool						MonitorThread_update(void);

	// �����ͺ��̽� Write ������
	HANDLE						_hDatabaseWriteThread;
	static unsigned __stdcall	DatabaseWriteThread(void *lpParam);
	bool						DatabaseWriteThread_update(void);
};