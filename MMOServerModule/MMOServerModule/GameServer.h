#pragma once

#define dfAUTH_PACKET_PROC_REPEAT 1
#define dfGAME_PACKET_PROC_REPEAT 3

class CGameServer : public CMMOServer
{
protected:
	class CPlayer : public CMMOServer::CSession
	{
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