#pragma once

class CGameServer;

class CLanClient_Login : public CLanClient
{
public:
	CLanClient_Login(CGameServer *pServer);
	virtual ~CLanClient_Login();

protected:
	virtual void OnEnterJoinServer(void) override;			// �������� ���� ���� ��
	virtual void OnLeaveServer(void) override;				// �������� ������ �������� ��

	virtual void OnRecv(CPacket *pRecvPacket) override;		// ��Ŷ(�޽���) ����, 1 Message �� 1 �̺�Ʈ.
	virtual void OnSend(int iSendSize) override;			// ��Ŷ(�޽���) �۽� �Ϸ� �̺�Ʈ.

private:
	CGameServer *_pGameServer;
};