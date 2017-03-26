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
		// 상속받은 인터페이스
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
		
		// 패킷 관련 함수
	private:
#pragma region packetproc_auth
		void PacketProc_Login(CPacket *pRecvPacket);			// 게임서버로 로그인
		
		void PacketProc_CharacterSelect(CPacket *pRecvPacket);	// 캐릭터 선택
#pragma endregion packetproc_auth

#pragma region packetproc_game
		void PacketProc_MoveCharacter(CPacket *pRecvPacket);	// 캐릭터 이동
		
		void PacketProc_StopCharacter(CPacket *pRecvPacket);	// 캐릭터 정지

		void PacketProc_Attack1(CPacket *pRecvPacket);			// 1번 공격 요청

		void PacketProc_Attack2(CPacket *pRecvPacket);			// 2번 공격 요청
#pragma endregion packetproc_game

#pragma region packetproc_etc
		void PacketProc_ClientHeartBeat(CPacket *pRecvPacket);	// 클라이언트 하트비트
		
		void PacketProc_ReqEcho(CPacket *pRecvPacket);			// 더미 에코
#pragma endregion packetproc_etc

#pragma region makepacket_auth
		//////////////////////////////////////////////////////////
		//	로그인 응답 패킷 만들기
		//	인자
		//		CPacket*	: 내용을 담을 패킷의 포인터
		//		BYTE		: 로그인 가능 여부(0 - 실패, 1 - 1번파티, 2 - 2번파티, 3 - 버전 다름)
		//		__int64		: Account번호
		//////////////////////////////////////////////////////////
		void MakePacket_ResLogin(CPacket *pSendPacket, BYTE byStatus, __int64 iAccountNo);

		//////////////////////////////////////////////////////////
		//	캐릭터 선택 응답 패킷 만들기
		//	인자
		//		CPacket*	: 내용을 담을 패킷의 포인터
		//		BYTE		: 캐릭터 선택 결과(0 - 실패, 1 - 성공)
		//	
		//	1번 파티는 캐릭터 번호가 1, 2, 3이면 성공
		//	2번 파티는 캐릭터 번호가 3, 4, 5이면 성공
		//	그 외는 모두 실패
		//////////////////////////////////////////////////////////
		void MakePacket_ResCharacterSelect(CPacket *pSendPacket, BYTE byStatus);
#pragma endregion makepacket_auth
		
#pragma region makepacket_game
		//////////////////////////////////////////////////////////
		//	캐릭터 생성 패킷 만들기
		//	인자
		//		CPacket*	: 내용을 담을 패킷의 포인터
		//		CLIENT_ID	: 유저 클라이언트 번호
		//		BYTE		: 캐릭터 번호	
		//		wchar_t*	: 닉네임
		//		float		: 클라이언트 X좌표
		//		float		: 클라이언트 Y좌표
		//		WORD		: 캐릭터가 바라보는 방향(1 부터 8까지)
		//		int			: 캐릭터 HP
		//		BYTE		: 파티 번호(1 또는 2)
		//////////////////////////////////////////////////////////
		void MakePacket_CreateCharacter(CPacket *pSendPacket, CLIENT_ID clientID, BYTE characterType, wchar_t *szNickname, float PosX, float PosY, WORD rotation, int hp, BYTE party);
		
		//////////////////////////////////////////////////////////
		//	다른 캐릭터 생성 패킷 만들기
		//	인자
		//		CPacket*	: 내용을 담을 패킷의 포인터
		//		CLIENT_ID	: 유저 클라이언트 번호
		//		BYTE		: 캐릭터 번호	
		//		wchar_t*	: 닉네임
		//		float		: 클라이언트 X좌표
		//		float		: 클라이언트 Y좌표
		//		WORD		: 캐릭터가 바라보는 방향(1 부터 8까지)
		//		BYTE		: 캐릭터 만드는 이유
		//		BYTE		: 사망 여부
		//		int			: 캐릭터 HP
		//		BYTE		: 파티 번호(1 또는 2)
		//
		//		캐릭터 만드는 이유가 0이면 섹터 이동으로 인한 생성
		//		1이면 신규 접속으로 인한 생성
		//////////////////////////////////////////////////////////
		void MakePacket_CreateOtherCharacter(CPacket *pSendPacket, __int64 clientID, BYTE characterType, wchar_t *szNickname, float PosX, float PosY, WORD rotation, BYTE respawn, BYTE die, int hp, BYTE party);

		void MakePacket_RemoveObject(CPacket *pSendPacket, CLIENT_ID clientID);

		void MakePacket_ResMoveCharacter(CPacket *pSendPacket, __int64 clientID, BYTE pathCount, PATH *pPath);
		
		void MakePacket_StopCharacter(CPacket *pSendPacket, __int64 clientID, float PosX, float PosY, WORD dir);

		void MakePacket_Sync(CPacket *pSendPacket, __int64 clientID, short tileX, short tileY);
#pragma endregion makepacket_game

#pragma region makepacket_etc
		//////////////////////////////////////////////////////////
		//	에코 응답 패킷
		//	인자
		//		CPacket*	: 내용을 담을 패킷의 포인터
		//		__int64		: 에코 패킷에 있던 Account번호
		//		__int64		: 에코 패킷에 있던 SendTick
		//
		//	에코 패킷을 받으면 내용 그대로 재조합
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
	
		// 기타 필요한 로직
	private:
		bool CheckErrorRange(float PosX, float PosY);

		void MoveStop(bool bSend);

	private:
		CGameServer *_pGameServer;
		
		__int64 _accountNo;
		__int64 _heartBeatTick;

		// AuthTh에서 세팅
		wchar_t	_szNickname[dfNICK_MAX_LEN];	// 닉네임
		
		BYTE	_byParty;						// 파티 번호 (1번 또는 2번)
		BYTE	_characterType;					// 캐릭터 타입 (1번 파티는 1, 2, 3 / 2번 파티는 3, 4, 5)

		// GameTh에서 세팅
		int		_serverX_curr;					// 현재 타일 X 좌표
		int		_serverY_curr;					// 현재 타일 Y 좌표
		int		_serverX_prev;					// 이전 타일 X 좌표 
		int		_serverY_prev;					// 이전 타일 Y 좌표

		float	_clientPosX;					// 클라이언트 X 좌표
		float	_clientPosY;					// 클라이언트 Y 좌표
			
		int		_sectorX_prev;					// 이전 섹터 X 좌표
		int		_sectorY_prev;					// 이전 섹터 Y 좌표
		int		_sectorX_curr;					// 현재 섹터 X 좌표
		int		_sectorY_curr;					// 현재 섹터 Y 좌표

		en_DIRECTION	_rotation;				// 방향

		bool	_bMove;							// 현재 이동중인지 여부
		PATH	_path[dfPATH_POINT_MAX];		// 이동할 경로
		int		_path_curr;						// 현재 경로
		int		_pathCount;						// 경로 개수
		ULONGLONG _nextTileTime;				// 다음 타일로 이동할 시간
		
		PATH	*_pPath;						// 경로를 지정할 포인터
		int		_goal_X;
		int		_goal_Y;

		int		_iHP;							// 체력
		BYTE	_byDie;							// 사망 플래그

		CLIENT_ID	_targetID;					// 타켓 ID
		CPlayer		*_targetPtr;				// 타켓 포인터. 포인터만 쓰면 위험할 수 있으므로 두 개를 같이 사용
		int		_iAttackType;					// 공격 타입 (1번, 2번이 있고 3번은 추가 될지 모르겠음)
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
		//	함수 : MoveSector
		//	인자 : 
		//	(in)	int			iCurrX		: 이동 전 X축 섹터 좌표
		//	(in)	int			iCurrY		: 이동 전 Y축 섹터 좌표
		//	(in)	int			iMoveX		: 이동할 X축 섹터 좌표
		//	(in)	int			iMoveY		: 이동할 Y축 섹터 좌표
		//	(in)	CLIENT_ID	clientID	: 섹터 내부 map에 넣을 키
		//	(in)	CPlayer		*pPlayer	: 섹터 내부 map에 넣을 내용
		//
		//	이동 전 섹터 좌표에서 내용을 빼내어 이동할 섹터 좌표에 넣어준다.
		//	이동 전 좌표에 (-1, -1)을 넣으면 신규 유저로 섹터에 추가만 하고,
		//	이동할 좌표에 (-1, -1)을 넣으면 퇴장할 유저로 섹터에서 제거만 한다.
		/////////////////////////////////////////////////////////////////////
		bool MoveSector(int iCurrX, int iCurrY, int iMoveX, int iMoveY, CLIENT_ID clientID, CPlayer *pPlayer);
		
		/////////////////////////////////////////////////////////////////////
		//	함수 : GetAroundSector
		//	인자 : 
		//	(in)	int iSectorX			: 이동 전 X축 타일 좌표
		//	(in)	int iSectorY			: 이동 전 Y축 타일 좌표
		//	(out)	stAROUND_SECTOR *around	: 주변 섹터 좌표를 저장할 구조체
		//
		//	섹터 좌표를 넣으면 주변 (3X3) 섹터의 좌표를 얻어준다.
		/////////////////////////////////////////////////////////////////////
		void GetAroundSector(int iSectorX, int iSectorY, stAROUND_SECTOR *around);
		
		/////////////////////////////////////////////////////////////////////
		//	함수 : GetUpdateSector
		//	인자 : 
		//	(in)	stAROUND_SECTOR *removeSector	: 삭제할 섹터 좌표를 저장할 구조체
		//	(in)	stAROUND_SECTOR *addSector	: 추가할 섹터 좌표를 저장할 구조체
		//
		//	두 구조체를 비교하여 removeSector에는 삭제할 오브젝트를 알려주는 패킷(캐릭터 삭제 등)
		//	을 보낼 섹터 좌표를 구해주고, addSector에는 추가할 오브젝트를 알려주는 패킷(캐릭터 생성 등)
		//	을 보낼 섹터 좌표를 구해준다.
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
	// 세션키 관리 함수
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
	// 섹터단위 패킷 보내기
	/////////////////////////////////////////////////////////////
	void SendPacket_SectorOne(CPacket *pSendPacket, int iSectorX, int iSectorY, CPlayer *pException);
	void SendPacket_SectorAround(CPacket *pSendPacket, int iSectorX, int iSectorY, CPlayer *pException);
	void SendPacket_SectorSwitch(CPacket *pSendPacket_remove, int iRemoveSectorX, int iRemoveSectorY, CPacket *pSendPacket_add, int iAddSectorX, int iAddSectorY, CPlayer *pexception);

private:
	CLanClient_Login *_lanClient_Login;						// 로그인 서버로 접속할 클라이언트
	CLanClient_Agent *_lanClient_Agent;						// 에이전트로 접속할 클라이언트
	CLanClient_Monitoring *_lanClient_Monitoring;			// 모니터링 서버로 접속할 클라이언트

	CPlayer *_pPlayerArray;									// 실제 플레이어를 생성할 배열

	std::map<__int64, st_SESSION_KEY *> _sessionKeyMap;		// 로그인 서버로부터 받은 키를 저장할 자료구조
	SRWLOCK _sessionKeyMapLock;								// 로그인 클라이언트와 세션키 맵을 같이 사용할 것이기 때문에 lock이 필요하다.

	__int64 _updateTick;									// 하트비트용

	AccountDB	*_database_Account;							// Account Database
	GameDB		*_database_Game;							// Game Database
	LogDB		*_database_Log;								// Log Database
	
	CMemoryPool<st_DBWRITER_MSG> _databaseMsgPool;			// 데이터베이스 메시지 풀
	CLockFreeQueue<st_DBWRITER_MSG *> _databaseMsgQueue;	// 데이터베이스 메시지 큐
	
	CJumpPointSearch *_jps;									// 길찾기 알고리즘

	CField<CLIENT_ID> *_field;								// ClientID를 관리하는 필드(충돌처리용)
		
	CSector *_sector;										// ClientID와 CPlayer *를 관리하는 Sector

	//모니터링 용도
public:
	long _iDatabaseWriteTPS;

private:
	long _iDatabaseWriteCounter;

private:
	// 모니터링 스레드
	HANDLE						_hMonitorThread;
	static unsigned __stdcall	MonitorThreadFunc(void *lpParam);
	bool						MonitorThread_update(void);

	// 데이터베이스 Write 스레드
	HANDLE						_hDatabaseWriteThread;
	static unsigned __stdcall	DatabaseWriteThread(void *lpParam);
	bool						DatabaseWriteThread_update(void);
};