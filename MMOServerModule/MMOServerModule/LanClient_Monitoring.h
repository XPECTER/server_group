#pragma once

class CGameServer;

class CLanClient_Monitoring : public CLanClient
{
public:
	CLanClient_Monitoring(CGameServer *pServer);
	virtual ~CLanClient_Monitoring();


protected:
	virtual void OnEnterJoinServer(void) override;			// �������� ���� ���� ��
	virtual void OnLeaveServer(void) override;				// �������� ������ �������� ��

	virtual void OnRecv(CPacket *pRecvPacket) override;		// ��Ŷ(�޽���) ����, 1 Message �� 1 �̺�Ʈ.
	virtual void OnSend(int iSendSize) override;			// ��Ŷ(�޽���) �۽� �Ϸ� �̺�Ʈ.

private:
	void SendPacket_LoginToMonitoringServer();
	void MakePacket_LoginToMonitoringServer(CPacket *pSendPacket);

private:
	CGameServer *_pGameServer;
};