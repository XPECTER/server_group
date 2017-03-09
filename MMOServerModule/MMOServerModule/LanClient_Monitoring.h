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
	virtual void OnEnterJoinServer(void) override;			// �������� ���� ���� ��
	virtual void OnLeaveServer(void) override;				// �������� ������ �������� ��

	virtual void OnRecv(CPacket *pRecvPacket) override;		// ��Ŷ(�޽���) ����, 1 Message �� 1 �̺�Ʈ.
	virtual void OnSend(int iSendSize) override;			// ��Ŷ(�޽���) �۽� �Ϸ� �̺�Ʈ.

private:
	void SendPacket_LoginToMonitoringServer();

private:
	CGameServer *_pGameServer;
};