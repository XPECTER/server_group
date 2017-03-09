#pragma once

class CGameServer;

class CLanClient_Login : public CLanClient
{
public:
	CLanClient_Login(CGameServer *pServer);
	virtual ~CLanClient_Login();

	void SendPacket_HeartBeat(BYTE type);
protected:
	virtual void OnEnterJoinServer(void) override;			// �������� ���� ���� ��
	virtual void OnLeaveServer(void) override;				// �������� ������ �������� ��

	virtual void OnRecv(CPacket *pRecvPacket) override;		// ��Ŷ(�޽���) ����, 1 Message �� 1 �̺�Ʈ.
	virtual void OnSend(int iSendSize) override;			// ��Ŷ(�޽���) �۽� �Ϸ� �̺�Ʈ.

private:
	void SendPacket_LoginToLoginServer(void);

	void PacketProc_RequestNewClientLogin(CPacket *pRecvPacket);
	void MakePacket_ResponseNewClientLogin(CPacket *pSendPacket, __int64 accountNo, __int64 parameter);

private:
	CGameServer *_pGameServer;
};