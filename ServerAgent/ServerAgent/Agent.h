#pragma once

class CAgent : public CLanServer
{
public:
	CAgent();

	bool GameServer_Start();
	bool GameServer_Shutdown();
	bool GameServer_Terminate();

	bool ChatServer_Start();
	bool ChatServer_Terminate();

	bool InitialPdh(void);
	bool UpdatePdh(void);

protected:
	virtual bool OnConnectionRequest(wchar_t *szClientIP, int iPort) override;   // accept ���� ��û�� �Դٴ� ���� ���������� �˷��ְ� IP Table���� ���� �źε� IP�� false. �ƴϸ� true

	virtual void OnClientJoin(ClientID clientID) override;						// ���������� ������ �����ߴٴ� ���� �˷���. WSARecv���� ȣ��
	virtual void OnClientLeave(ClientID clientID) override;						// ���������� ������ �����ٴ� ���� �˷���. ClientRelease���� ��� �ϴ� �������.

	virtual void OnRecv(ClientID clientID, CPacket *pRecvPacket) override;		// ��Ŷ ���� �Ϸ� ��
	virtual void OnSend(ClientID clientID, int sendSize) override;				// ��Ŷ �۽� �Ϸ� ��

private:
	void SendPacket_AvailableMemory();
	void SendPacket_NonpagedMemory();

	void SendPacket_ReceivedBytes();
	void SendPacket_SentBytes();

private:
	
	CMonitoringClient _lanClient_Monitoring;

	CCpuUsage _usageGameServer;
	CCpuUsage _usageChatServer;

	HANDLE _hGameServer;
	HANDLE _hChatServer;

	PDH_HQUERY _pdhQuery;

	PDH_HCOUNTER _pdhCounterThreadCounterGameServer;
	PDH_HCOUNTER _pdhCounterWorkingMemoryGameServer;

	long	_pdhValueThreadCounterGameServer;
	double	_pdhValueWorkingMemoryGameServer;

	PDH_HCOUNTER _pdhCounterThreadCounterChatServer;
	PDH_HCOUNTER _pdhCounterWorkingMemoryChatServer;

	long	_pdhValueThreadCounterChatServer;
	double	_pdhValueWorkingMemoryChatServer;

	PDH_HCOUNTER _pdhCounterNonpagedMemory;
	PDH_HCOUNTER _pdhCounterAvailableMemory;

	long _pdhValueNonpagedMemory;
	long _pdhValueAvailableMemory;

	PDH_HCOUNTER _pdhCounterNetworkRecv;
	PDH_HCOUNTER _pdhCounterNetworkSend;

	long _pdhValueReceivedBytes;
	long _pdhValueSentBytes;
};