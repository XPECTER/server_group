#pragma once

class CGameServer;

class CLanClient_Login : public CLanClient
{
public:
	CLanClient_Login(CGameServer *pServer);
	virtual ~CLanClient_Login();

	void SendPacket_HeartBeat(BYTE type);
protected:
	virtual void OnEnterJoinServer(void) override;			// 서버와의 연결 성공 후
	virtual void OnLeaveServer(void) override;				// 서버와의 연결이 끊어졌을 때

	virtual void OnRecv(CPacket *pRecvPacket) override;		// 패킷(메시지) 수신, 1 Message 당 1 이벤트.
	virtual void OnSend(int iSendSize) override;			// 패킷(메시지) 송신 완료 이벤트.

private:
	void SendPacket_LoginToLoginServer(void);

	void PacketProc_RequestNewClientLogin(CPacket *pRecvPacket);
	void MakePacket_ResponseNewClientLogin(CPacket *pSendPacket, __int64 accountNo, __int64 parameter);

private:
	CGameServer *_pGameServer;
};