#pragma once

class CGameServer;

class CLanClient_Monitoring : public CLanClient
{
public:
	CLanClient_Monitoring(CGameServer *pServer);
	virtual ~CLanClient_Monitoring();

	void SendPacket_SessionCount(int iTotalSessionCount);
	void SendPacket_AuthPlayer(int iAuthThreadSessionCount);
	void SendPacket_GamePlayer(int iGameThreadSessionCount);
	void SendPacket_AcceptTPS(int iAcceptTPS);
	void SendPacket_RecvPacketTPS(int iRecvPacketTPS);
	void SendPacket_SendPacketTPS(int iSendPacketTPS);
	void SendPacket_DatabaseWriteTPS(int iDatabaseWriteTPS);
	void SendPacket_DatabaseMsgCount(int iDatabaseMsgCount);
	void SendPacket_AuthThreadTPS(int iAuthThreadLoopTPS);
	void SendPacket_GameThreadTPS(int iGameThreadLoopTPS);
	void SendPacket_PacketUseCount(int iPacketUseCount);
protected:
	virtual void OnEnterJoinServer(void) override;			// 서버와의 연결 성공 후
	virtual void OnLeaveServer(void) override;				// 서버와의 연결이 끊어졌을 때

	virtual void OnRecv(CPacket *pRecvPacket) override;		// 패킷(메시지) 수신, 1 Message 당 1 이벤트.
	virtual void OnSend(int iSendSize) override;			// 패킷(메시지) 송신 완료 이벤트.

private:
	void SendPacket_LoginToMonitoringServer();

private:
	CGameServer *_pGameServer;
};