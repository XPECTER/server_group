#pragma once

#include "MMOServer.h"
#include "LanClient_Login.h"
#include "LanClient_Agent.h"
#include "LanClient_Monitoring.h"

#define dfAUTH_PACKET_PROC_REPEAT 1
#define dfGAME_PACKET_PROC_REPEAT 3

#define dfTHREAD_UPDATE_TICK_MONITOR 999

class CGameServer : public CMMOServer
{
protected:
	class CPlayer : public CMMOServer::CSession
	{
	public:
		void Player_Init(void);

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
		void MakePacket_ResLogin(BYTE iStatus, __int64 iAccountNo, CPacket *pSendPacket);
	
		void PacketProc_ReqEcho(CPacket *pRecvPacket);
		void MakePacket_ResEcho(__int64 iAccountNo, __int64 SendTick, CPacket *pSendPacket);
	};

public:
	CGameServer(int iClientMax);
	~CGameServer();

	bool Start(void);
protected:
	virtual void OnAuth_Update(void) override;
	virtual void OnGame_Update(void) override;

private:
	void SendPacket_SessionCount(void);
	void SendPacket_AuthPlayer(void);
	void SendPacket_GamePlayer(void);
	void SendPacket_AcceptTPS(void);
	void SendPacket_RecvPacketTPS(void);
	void SendPacket_SendPacketTPS(void);
	void SendPacket_DatabaseWriteTPS(void);
	void SendPacket_DatabaseMsgCount(void);
	void SendPacket_AuthThreadTPS(void);
	void SendPacket_GameThreadTPS(void);
	void SendPacket_PacketUseCount(void);

private:
	CLanClient_Login *_lanClient_Login;
	CLanClient_Agent *_lanClient_Agent;
	CLanClient_Monitoring *_lanClient_Monitoring;

	// 실제 플레이어를 생성할 배열
	CPlayer *pPlayerArray;

private:
	// 모니터링 스레드
	HANDLE						_hMonitorThread;
	static unsigned __stdcall	MonitorThreadFunc(void *lpParam);
	bool						MonitorThread_update(void);
};