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
	virtual bool OnConnectionRequest(wchar_t *szClientIP, int iPort) override;   // accept 직후 요청이 왔다는 것을 컨텐츠에게 알려주고 IP Table에서 접속 거부된 IP면 false. 아니면 true

	virtual void OnClientJoin(ClientID clientID) override;						// 컨텐츠에게 유저가 접속했다는 것을 알려줌. WSARecv전에 호출
	virtual void OnClientLeave(ClientID clientID) override;						// 컨텐츠에게 유저가 떠났다는 것을 알려줌. ClientRelease에서 상단 하단 상관없음.

	virtual void OnRecv(ClientID clientID, CPacket *pRecvPacket) override;		// 패킷 수신 완료 후
	virtual void OnSend(ClientID clientID, int sendSize) override;				// 패킷 송신 완료 후

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