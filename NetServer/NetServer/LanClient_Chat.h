#pragma once

class CChatServer;

class CLanClient_Chat : public CLanClient
{


public:
	CLanClient_Chat(CChatServer *pServerPtr);
	virtual ~CLanClient_Chat();

	virtual void OnEnterJoinServer(void) override;										// �������� ���� ���� ��
	virtual void OnLeaveServer(void) override;											// �������� ������ �������� ��

	virtual void OnRecv(CPacket *pPacket) override;										// ��Ŷ(�޽���) ����, 1 Message �� 1 �̺�Ʈ.
	virtual void OnSend(int iSendSize) override;

	//virtual void OnWorkerThreadBegin(void) override;										// ��Ŀ������ 1ȸ ���� ����.
	//virtual void OnWorkerThreadEnd(void) override;										// ��Ŀ������ 1ȸ ���� ���� ��

	//virtual void OnError(int iErrorCode, WCHAR *szError) override;						// ���� 

public:
	void SendPacket_LoginServerLogin(void);
	void SendPacket_HeartBeat(BYTE threadType);

private:
	void MakePacket_LoginServerLogin(CPacket *pSendPacket);
	void MakePacket_ResponseNewClientLogin(CPacket *pSendPacket, __int64 accountNo, __int64 parameter);
	void MakePacket_HeartBeat(CPacket *pSendPacket, BYTE threadType);

	void PacketProc_RequestNewClientLogin(CPacket *pRecvPacket);

public:
	__int64		_RecvAddSessionPacket;

private:
	CChatServer *_pChatServer;

	
};