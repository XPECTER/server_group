#pragma once

#include "MMOServer.h"
#include "LanClient_Login.h"
#include "LanClient_Agent.h"
#include "LanClient_Monitoring.h"
#include "Field.h"
#include "JumpPointSearch.h"
#include "DBConnector.h"
#include "Pattern.h"

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

//enum en_CHARACTER_TYPE
//{
//	en_CHARACTER_TYPE_ORC
//};

class CGameServer : public CMMOServer
{
protected:
	class CPlayer : public CMMOServer::CSession
	{
	public:
		CPlayer();
		virtual ~CPlayer();

		//////////////////////////////////////////////////////////
		//	
		//////////////////////////////////////////////////////////
		void Player_Init(CGameServer *pGameServer);

		//////////////////////////////////////////////////////////
		//	
		//////////////////////////////////////////////////////////
		void Action_Move();

		//////////////////////////////////////////////////////////
		//	
		//////////////////////////////////////////////////////////
		void Action_Attack();

		//////////////////////////////////////////////////////////
		//	
		//////////////////////////////////////////////////////////
		void CheckHeartBeat(void);
		// ��ӹ��� �������̽�
	protected:
#pragma region event
		//////////////////////////////////////////////////////////
		//	
		//////////////////////////////////////////////////////////
		virtual bool OnAuth_ClientJoin(void) override;

		//////////////////////////////////////////////////////////
		//	
		//////////////////////////////////////////////////////////
		virtual bool OnAuth_PacketProc(void) override;

		//////////////////////////////////////////////////////////
		//	
		//////////////////////////////////////////////////////////
		virtual bool OnAuth_ClientLeave(bool bToGame) override;

		//////////////////////////////////////////////////////////
		//	
		//////////////////////////////////////////////////////////
		virtual bool OnGame_ClientJoin(void) override;

		//////////////////////////////////////////////////////////
		//	
		//////////////////////////////////////////////////////////
		virtual bool OnGame_PacketProc(void) override;

		//////////////////////////////////////////////////////////
		//	
		//////////////////////////////////////////////////////////
		virtual bool OnGame_ClientLeave(void) override;

		//////////////////////////////////////////////////////////
		//	
		//////////////////////////////////////////////////////////
		virtual bool OnGame_ClientRelease(void) override;
#pragma endregion event
		
		// ��Ŷ ���� �Լ�
	private:
#pragma region packetproc_auth
		void PacketProc_Login(CPacket *pRecvPacket);			// ���Ӽ����� �α���
		
		void PacketProc_CharacterSelect(CPacket *pRecvPacket);	// ĳ���� ����
#pragma endregion packetproc_auth

#pragma region packetproc_game
		void PacketProc_MoveCharacter(CPacket *pRecvPacket);	// ĳ���� �̵�
		
		void PacketProc_StopCharacter(CPacket *pRecvPacket);	// ĳ���� ����

		void PacketProc_Attack1(CPacket *pRecvPacket);			// 1�� ���� ��û

		void PacketProc_Attack2(CPacket *pRecvPacket);			// 2�� ���� ��û
#pragma endregion packetproc_game

#pragma region packetproc_etc
		void PacketProc_ClientHeartBeat(CPacket *pRecvPacket);	// Ŭ���̾�Ʈ ��Ʈ��Ʈ
		
		void PacketProc_ReqEcho(CPacket *pRecvPacket);			// ���� ����
#pragma endregion packetproc_etc

#pragma region makepacket_auth
		//////////////////////////////////////////////////////////
		//	�α��� ���� ��Ŷ �����
		//	����
		//		CPacket*	: ������ ���� ��Ŷ�� ������
		//		BYTE		: �α��� ���� ����(0 - ����, 1 - 1����Ƽ, 2 - 2����Ƽ, 3 - ���� �ٸ�)
		//		__int64		: Account��ȣ
		//////////////////////////////////////////////////////////
		void MakePacket_ResLogin(CPacket *pSendPacket, BYTE byStatus, __int64 iAccountNo);

		//////////////////////////////////////////////////////////
		//	ĳ���� ���� ���� ��Ŷ �����
		//	����
		//		CPacket*	: ������ ���� ��Ŷ�� ������
		//		BYTE		: ĳ���� ���� ���(0 - ����, 1 - ����)
		//	
		//	1�� ��Ƽ�� ĳ���� ��ȣ�� 1, 2, 3�̸� ����
		//	2�� ��Ƽ�� ĳ���� ��ȣ�� 3, 4, 5�̸� ����
		//	�� �ܴ� ��� ����
		//////////////////////////////////////////////////////////
		void MakePacket_ResCharacterSelect(CPacket *pSendPacket, BYTE byStatus);
#pragma endregion makepacket_auth
		
#pragma region makepacket_game
		//////////////////////////////////////////////////////////
		//	ĳ���� ���� ��Ŷ �����
		//	����
		//		CPacket*	: ������ ���� ��Ŷ�� ������
		//		CLIENT_ID	: ���� Ŭ���̾�Ʈ ��ȣ
		//		BYTE		: ĳ���� ��ȣ	
		//		wchar_t*	: �г���
		//		float		: Ŭ���̾�Ʈ X��ǥ
		//		float		: Ŭ���̾�Ʈ Y��ǥ
		//		WORD		: ĳ���Ͱ� �ٶ󺸴� ����(1 ���� 8����)
		//		int			: ĳ���� HP
		//		BYTE		: ��Ƽ ��ȣ(1 �Ǵ� 2)
		//////////////////////////////////////////////////////////
		void MakePacket_CreateCharacter(CPacket *pSendPacket, CLIENT_ID clientID, BYTE characterType, wchar_t *szNickname, float PosX, float PosY, WORD rotation, int hp, BYTE party);
		
		//////////////////////////////////////////////////////////
		//	�ٸ� ĳ���� ���� ��Ŷ �����
		//	����
		//		CPacket*	: ������ ���� ��Ŷ�� ������
		//		CLIENT_ID	: ���� Ŭ���̾�Ʈ ��ȣ
		//		BYTE		: ĳ���� ��ȣ	
		//		wchar_t*	: �г���
		//		float		: Ŭ���̾�Ʈ X��ǥ
		//		float		: Ŭ���̾�Ʈ Y��ǥ
		//		WORD		: ĳ���Ͱ� �ٶ󺸴� ����(1 ���� 8����)
		//		BYTE		: ĳ���� ����� ����
		//		BYTE		: ��� ����
		//		int			: ĳ���� HP
		//		BYTE		: ��Ƽ ��ȣ(1 �Ǵ� 2)
		//
		//		ĳ���� ����� ������ 0�̸� ���� �̵����� ���� ����
		//		1�̸� �ű� �������� ���� ����
		//////////////////////////////////////////////////////////
		void MakePacket_CreateOtherCharacter(CPacket *pSendPacket, __int64 clientID, BYTE characterType, wchar_t *szNickname, float PosX, float PosY, WORD rotation, BYTE respawn, BYTE die, int hp, BYTE party);

		void MakePacket_RemoveObject(CPacket *pSendPacket, CLIENT_ID clientID);

		void MakePacket_ResMoveCharacter(CPacket *pSendPacket, __int64 clientID, BYTE pathCount, PATH *pPath);
		
		void MakePacket_StopCharacter(CPacket *pSendPacket, __int64 clientID, float PosX, float PosY, WORD dir);

		void MakePacket_Sync(CPacket *pSendPacket, __int64 clientID, short tileX, short tileY);
#pragma endregion makepacket_game

#pragma region makepacket_etc
		//////////////////////////////////////////////////////////
		//	���� ���� ��Ŷ
		//	����
		//		CPacket*	: ������ ���� ��Ŷ�� ������
		//		__int64		: ���� ��Ŷ�� �ִ� Account��ȣ
		//		__int64		: ���� ��Ŷ�� �ִ� SendTick
		//
		//	���� ��Ŷ�� ������ ���� �״�� ������
		//////////////////////////////////////////////////////////
		void MakePacket_ResEcho(CPacket *pSendPacket, __int64 iAccountNo, __int64 SendTick);
#pragma endregion makepacket_etc
		
#pragma region sendpacket
		void SendPacket_NewCreateCharacter(void);

		void SendPacket_RemoveObject_Disconnect(void);

		void SendPacket_MoveSector(void);

		void SendPacket_MoveStop(void);

		void SendPacket_Sync(void);

#pragma endregion sendpacket
	
		// ��Ÿ �ʿ��� ����
	private:
		bool CheckErrorRange(float PosX, float PosY);

		void MoveStop(bool bSend);

	private:
		CGameServer *_pGameServer;
		
		__int64 _accountNo;
		__int64 _heartBeatTick;

		// AuthTh���� ����
		wchar_t	_szNickname[dfNICK_MAX_LEN];	// �г���
		
		BYTE	_byParty;						// ��Ƽ ��ȣ (1�� �Ǵ� 2��)
		BYTE	_characterType;					// ĳ���� Ÿ�� (1�� ��Ƽ�� 1, 2, 3 / 2�� ��Ƽ�� 3, 4, 5)

		// GameTh���� ����
		int		_serverX_curr;					// ���� Ÿ�� X ��ǥ
		int		_serverY_curr;					// ���� Ÿ�� Y ��ǥ
		int		_serverX_prev;					// ���� Ÿ�� X ��ǥ 
		int		_serverY_prev;					// ���� Ÿ�� Y ��ǥ

		float	_clientPosX;					// Ŭ���̾�Ʈ X ��ǥ
		float	_clientPosY;					// Ŭ���̾�Ʈ Y ��ǥ
			
		int		_sectorX_prev;					// ���� ���� X ��ǥ
		int		_sectorY_prev;					// ���� ���� Y ��ǥ
		int		_sectorX_curr;					// ���� ���� X ��ǥ
		int		_sectorY_curr;					// ���� ���� Y ��ǥ

		en_DIRECTION	_rotation;				// ����

		bool	_bMove;							// ���� �̵������� ����
		PATH	_path[dfPATH_POINT_MAX];		// �̵��� ���
		int		_path_curr;						// ���� ���
		int		_pathCount;						// ��� ����
		ULONGLONG _nextTileTime;				// ���� Ÿ�Ϸ� �̵��� �ð�
		
		PATH	*_pPath;						// ��θ� ������ ������
		int		_goal_X;
		int		_goal_Y;

		int		_iHP;							// ü��
		BYTE	_byDie;							// ��� �÷���

		CLIENT_ID	_targetID;					// Ÿ�� ID
		CPlayer		*_targetPtr;				// Ÿ�� ������. �����͸� ���� ������ �� �����Ƿ� �� ���� ���� ���
		int		_iAttackType;					// ���� Ÿ�� (1��, 2���� �ְ� 3���� �߰� ���� �𸣰���)
	};

	typedef std::map<CLIENT_ID, CPlayer *> sectorMap;

//private:
public:
	struct stSECTOR_POS
	{
		int _iSectorX;
		int _iSectorY;
	};

	struct stAROUND_SECTOR
	{
		int _iCount;
		stSECTOR_POS _around[9];
	};

	class CSector
	{
	public:
		CSector(int iWidth, int iHeight);
		~CSector();

		/////////////////////////////////////////////////////////////////////
		//	�Լ� : MoveSector
		//	���� : 
		//	(in)	int			iCurrX		: �̵� �� X�� ���� ��ǥ
		//	(in)	int			iCurrY		: �̵� �� Y�� ���� ��ǥ
		//	(in)	int			iMoveX		: �̵��� X�� ���� ��ǥ
		//	(in)	int			iMoveY		: �̵��� Y�� ���� ��ǥ
		//	(in)	CLIENT_ID	clientID	: ���� ���� map�� ���� Ű
		//	(in)	CPlayer		*pPlayer	: ���� ���� map�� ���� ����
		//
		//	�̵� �� ���� ��ǥ���� ������ ������ �̵��� ���� ��ǥ�� �־��ش�.
		//	�̵� �� ��ǥ�� (-1, -1)�� ������ �ű� ������ ���Ϳ� �߰��� �ϰ�,
		//	�̵��� ��ǥ�� (-1, -1)�� ������ ������ ������ ���Ϳ��� ���Ÿ� �Ѵ�.
		/////////////////////////////////////////////////////////////////////
		bool MoveSector(int iCurrX, int iCurrY, int iMoveX, int iMoveY, CLIENT_ID clientID, CPlayer *pPlayer);
		
		/////////////////////////////////////////////////////////////////////
		//	�Լ� : GetAroundSector
		//	���� : 
		//	(in)	int iSectorX			: �̵� �� X�� Ÿ�� ��ǥ
		//	(in)	int iSectorY			: �̵� �� Y�� Ÿ�� ��ǥ
		//	(out)	stAROUND_SECTOR *around	: �ֺ� ���� ��ǥ�� ������ ����ü
		//
		//	���� ��ǥ�� ������ �ֺ� (3X3) ������ ��ǥ�� ����ش�.
		/////////////////////////////////////////////////////////////////////
		void GetAroundSector(int iSectorX, int iSectorY, stAROUND_SECTOR *around);
		
		/////////////////////////////////////////////////////////////////////
		//	�Լ� : GetUpdateSector
		//	���� : 
		//	(in)	stAROUND_SECTOR *removeSector	: ������ ���� ��ǥ�� ������ ����ü
		//	(in)	stAROUND_SECTOR *addSector	: �߰��� ���� ��ǥ�� ������ ����ü
		//
		//	�� ����ü�� ���Ͽ� removeSector���� ������ ������Ʈ�� �˷��ִ� ��Ŷ(ĳ���� ���� ��)
		//	�� ���� ���� ��ǥ�� �����ְ�, addSector���� �߰��� ������Ʈ�� �˷��ִ� ��Ŷ(ĳ���� ���� ��)
		//	�� ���� ���� ��ǥ�� �����ش�.
		/////////////////////////////////////////////////////////////////////
		void GetUpdateSector(stAROUND_SECTOR *removeSector, stAROUND_SECTOR *addSector);

		sectorMap* GetList(int iSectorX, int iSectorY);
	private:
		bool CheckRange(int iSectorX, int iSectorY);

		bool InsertPlayer_Sector(int iSectorX, int iSectorY, CLIENT_ID clientID, CPlayer *pPlayer);
		bool DeletePlayer_Sector(int iSectorX, int iSectorY, CLIENT_ID clientID, CPlayer *pPlayer);
		
	private:
		int _iWidth;
		int _iHeight;

		sectorMap **_map;
	};

public:
	CGameServer(int iClientMax);
	~CGameServer();

	bool Start(void);
	void Stop(void);

	/////////////////////////////////////////////////////////////
	// ����Ű ���� �Լ�
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

	/////////////////////////////////////////////////////////////
	// ���ʹ��� ��Ŷ ������
	/////////////////////////////////////////////////////////////
	void SendPacket_SectorOne(CPacket *pSendPacket, int iSectorX, int iSectorY, CPlayer *pException);
	void SendPacket_SectorAround(CPacket *pSendPacket, int iSectorX, int iSectorY, CPlayer *pException);
	void SendPacket_SectorSwitch(CPacket *pSendPacket_remove, int iRemoveSectorX, int iRemoveSectorY, CPacket *pSendPacket_add, int iAddSectorX, int iAddSectorY, CPlayer *pexception);

private:
	CLanClient_Login *_lanClient_Login;						// �α��� ������ ������ Ŭ���̾�Ʈ
	CLanClient_Agent *_lanClient_Agent;						// ������Ʈ�� ������ Ŭ���̾�Ʈ
	CLanClient_Monitoring *_lanClient_Monitoring;			// ����͸� ������ ������ Ŭ���̾�Ʈ

	CPlayer *_pPlayerArray;									// ���� �÷��̾ ������ �迭

	std::map<__int64, st_SESSION_KEY *> _sessionKeyMap;		// �α��� �����κ��� ���� Ű�� ������ �ڷᱸ��
	SRWLOCK _sessionKeyMapLock;								// �α��� Ŭ���̾�Ʈ�� ����Ű ���� ���� ����� ���̱� ������ lock�� �ʿ��ϴ�.

	__int64 _updateTick;									// ��Ʈ��Ʈ��

	AccountDB	*_database_Account;							// Account Database
	GameDB		*_database_Game;							// Game Database
	LogDB		*_database_Log;								// Log Database
	
	CMemoryPool<st_DBWRITER_MSG> _databaseMsgPool;			// �����ͺ��̽� �޽��� Ǯ
	CLockFreeQueue<st_DBWRITER_MSG *> _databaseMsgQueue;	// �����ͺ��̽� �޽��� ť
	
	CJumpPointSearch *_jps;									// ��ã�� �˰���

	CField<CLIENT_ID> *_field;								// ClientID�� �����ϴ� �ʵ�(�浹ó����)
		
	CSector *_sector;										// ClientID�� CPlayer *�� �����ϴ� Sector

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