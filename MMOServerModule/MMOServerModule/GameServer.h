#pragma once

#define dfAUTH_PACKET_PROC_REPEAT 1
#define dfGAME_PACKET_PROC_REPEAT 3

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

	//bool Start(void);
protected:
	virtual void OnAuth_Update(void) override;
	virtual void OnGame_Update(void) override;

private:
	CLanClient_Game *_loginServer_Client;

	// 실제 플레이어를 생성할 배열
	CPlayer *pPlayerArray;
};