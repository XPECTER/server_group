#pragma once

class CChatServer;

class CLanClient_Chat : public CLanClient
{


public:
	CLanClient_Chat(CChatServer *pServerPtr);
	virtual ~CLanClient_Chat();

	virtual void OnEnterJoinServer(void) override;										// 서버와의 연결 성공 후
	virtual void OnLeaveServer(void) override;											// 서버와의 연결이 끊어졌을 때

	virtual void OnRecv(CPacket *pPacket) override;										// 패킷(메시지) 수신, 1 Message 당 1 이벤트.
	virtual void OnSend(int iSendSize) override;

	//virtual void OnWorkerThreadBegin(void) override;										// 워커스레드 1회 돌기 직전.
	//virtual void OnWorkerThreadEnd(void) override;										// 워커스레드 1회 돌기 종료 후

	//virtual void OnError(int iErrorCode, WCHAR *szError) override;						// 에러 

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