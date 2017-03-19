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

		// 패킷 처리부
	private:
		//////////////////////////////////////////////////////////
		// 로그인 패킷 처리부
		void PacketProc_Login(CPacket *pRecvPacket);

		// 로그인 패킷 처리부에서 응답 패킷 만들 때 사용
		void MakePacket_ResLogin(CPacket *pSendPacket, BYTE byStatus, __int64 iAccountNo);
		//////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////
		// 캐릭터 선택 패킷 처리부
		void PacketProc_CharacterSelect(CPacket *pRecvPacket);

		// 캐릭터 선택 패킷 처리부에서 응답 패킷 만들 때 사용
		void MakePacket_ResCharacterSelect(CPacket *pSendPacket, BYTE byStatus);
		//////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////
		// 에코 패킷 처리부
		void PacketProc_ReqEcho(CPacket *pRecvPacket);

		// 에코 패킷 처리부에서 응답 패킷 만들 때 사용
		void MakePacket_ResEcho(CPacket *pSendPacket, __int64 iAccountNo, __int64 SendTick);
		//////////////////////////////////////////////////////////

		// 하트비트 처리
		void PacketProc_ClientHeartBeat(CPacket *pRecvPacket);
	
		//////////////////////////////////////////////////////////
		// 캐릭터 생성 패킷 보내기
		void SendPacket_CreateCharacter(void);

		// 캐릭터 생성 패킷 만들기
		void MakePacket_CreateCharacter(CPacket *pSendPacket, __int64 clientID, BYTE characterType, wchar_t *szNickname, float PosX, float PosY, WORD rotation, int hp, BYTE party);
		//////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////
		// 캐릭터 이동 패킷 처리부
		void PacketProc_MoveCharacter(CPacket *pRecvPacket);

		// 캐릭터 이동 패킷 처리부에서 응답 패킷 만들 때 사용
		void MakePacket_ResMoveCharacter(CPacket *pSendPacket, __int64 clientID, BYTE pathCount, PATH *pPath);
		//////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////
		// 캐릭터 정지 요청 패킷 처리부
		void PacketProc_StopCharacter(CPacket *pRecvPacket);

		// 캐릭터 정지 요청 응답 패킷 
		void MakePacket_StopCharacter(CPacket *pSendPacket, __int64 clientID, float PosX, float PosY, WORD dir);
		//////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////
		// Sync 패킷 보내기
		void SendPacket_Sync(void);

		// Sync 패킷 만들기
		void MakePacket_Sync(CPacket *pSendPacket, __int64 clientID, short tileX, short tileY);
		//////////////////////////////////////////////////////////

		// 기타 필요한 로직
	private:
		bool CheckErrorRange(float PosX, float PosY);

	private:
		CGameServer *_pGameServer;
		
		__int64 _accountNo;
		__int64 _heartBeatTick;

		// AuthTh에서 세팅
		wchar_t	_szNickname[dfNICK_MAX_LEN];			// 닉네임
		
		BYTE	_byParty;				// 파티 번호 (1번 또는 2번)
		BYTE	_characterType;			// 캐릭터 타입 (1번 파티는 1, 2, 3 / 2번 파티는 3, 4, 5)
		

		// GameTh에서 세팅
		short	_serverX;					// 서버 X 좌표
		short	_serverY;					// 서버 Y 좌표
		float	_clientPosX;				// 클라이언트 X 좌표
		float	_clientPosY;				// 클라이언트 Y 좌표
		short	_sectorX;					// 섹터 X 좌표
		short	_sectorY;					// 섹터 Y 좌표

		WORD	_rotation;					// 방향

		int		_iHP;						// 체력

		PATH	_path[dfPATH_POINT_MAX];	// 이동할 경로
	};

public:
	CGameServer(int iClientMax);
	~CGameServer();

	bool Start(void);
	void Stop(void);

	/////////////////////////////////////////////////////////////
	// 세션키 관리
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

	// 실제 플레이어를 생성할 배열
	CPlayer *pPlayerArray;

	// 로그인 서버로부터 받은 키 자료구조
	std::map<__int64, st_SESSION_KEY *> _sessionKeyMap;
	SRWLOCK _sessionKeyMapLock;

	// 하트비트용
	__int64 _updateTick;

	// 데이터베이스
	AccountDB	*_database_Account;
	GameDB		*_database_Game;
	LogDB		*_database_Log;

	// 데이터베이스 메시지 풀
	CMemoryPool<st_DBWRITER_MSG> _databaseMsgPool;

	// 데이터베이스 메시지 큐
	CLockFreeQueue<st_DBWRITER_MSG *> _databaseMsgQueue;

	// 길찾기 
	CJumpPointSearch *_jps;

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