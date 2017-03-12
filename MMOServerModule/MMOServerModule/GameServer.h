#pragma once

#include "MMOServer.h"
#include "LanClient_Login.h"
#include "LanClient_Agent.h"
#include "LanClient_Monitoring.h"
#include "DBConnector.h"

#define dfDUMMY_ACCOUNTNO_LIMIT 999999

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

	private:
		void PacketProc_Login(CPacket *pRecvPacket);
		void MakePacket_ResLogin(CPacket *pSendPacket, BYTE byStatus, __int64 iAccountNo);
	
		void PacketProc_CharacterSelect(CPacket *pRecvPacket);
		void MakePacket_ResCharacterSelect(CPacket *pSendPacket, BYTE byStatus);

		void PacketProc_ReqEcho(CPacket *pRecvPacket);
		void MakePacket_ResEcho(CPacket *pSendPacket, __int64 iAccountNo, __int64 SendTick);

		void PacketProc_ClientHeartBeat(CPacket *pRecvPacket);
	
	private:
		CGameServer *_pGameServer;

		BYTE _byParty;
		__int64 _accountNo;
		__int64 _heartBeatTick;
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
	CDatabase _database;

	// 데이터베이스 메시지 풀
	CMemoryPool<st_DBWRITER_MSG> _databaseMsgPool;

	// 데이터베이스 메시지 큐
	CLockFreeQueue<st_DBWRITER_MSG *> _databaseMsgQueue;

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