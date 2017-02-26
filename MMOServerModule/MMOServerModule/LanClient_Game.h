#pragma once

class CLanClient_Game : public CLanClient
{
public:
	CLanClient_Game();
	virtual ~CLanClient_Game();

protected:
	virtual void OnEnterJoinServer(void) override;										// �������� ���� ���� ��
	virtual void OnLeaveServer(void) override;											// �������� ������ �������� ��

	virtual void OnRecv(CPacket *pRecvPacket) override;										// ��Ŷ(�޽���) ����, 1 Message �� 1 �̺�Ʈ.
	virtual void OnSend(int iSendSize) override;											// ��Ŷ(�޽���) �۽� �Ϸ� �̺�Ʈ.
};