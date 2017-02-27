#pragma once

typedef __int64 CLIENT_ID;

#define MAKECLIENTID(index, id) (((__int64)index << 48) | id)
#define EXTRACTCLIENTINDEX(ClientID) ((__int64)ClientID >> 48)
#define EXTRACTCLIENTID(ClientID) ((__int64)ClientID & 0x00FFFFFF)

struct st_ACCEPT_CLIENT_INFO
{
	SOCKET _clientSock;
	SOCKADDR_IN _clientAddr;
};

class CMMOServer;

class CSESSION
{
public:
	enum en_SESSION_MODE
	{
		MODE_NONE = 0,						// 미 접속 상태
		MODE_AUTH,							// Accept 후 세션 등록
		MODE_AUTH_TO_GAME,					// 인증 처리 후 로그인 완료
		MODE_GAME,							// 인증 모드에서 게임 모드로 전환
		MODE_LOGOUT_IN_AUTH,				// AuthThread에서 close
		MODE_LOGOUT_IN_GAME,				// GameThread에서 close
		MODE_WAIT_LOGOUT,					// 세션 릴리즈
	};
public:
	bool SendPacket(CPacket *pSendPacket);
	bool Disconnect(void);
	void SetMode_Game(void);

	void CompleteRecv(int recvBytes);
	void CompleteSend(void);


protected:
	virtual bool OnAuth_ClientJoin(void) = 0;
	virtual bool OnAuth_PacketProc(void) = 0;
	virtual bool OnAuth_ClientLeave(bool bToGame) = 0;

	virtual bool OnGame_ClientJoin(void) = 0;
	virtual bool OnGame_PacketProc(void) = 0;
	virtual bool OnGame_ClientLeave(void) = 0;
	virtual bool OnGame_ClientRelease(void) = 0;

protected:
	int		_iSessionMode;
	bool	_bAuthToGame;
	bool	_bLogout;

private:
	CLIENT_ID _clientID;
	st_ACCEPT_CLIENT_INFO *_connectInfo;

	CStreamQueue	_recvQ;
	CLockFreeQueue<CPacket *>	_sendQ;
	CLockFreeQueue<CPacket *>	_completeRecvQ;

	OVERLAPPED		_recvOverlap;
	OVERLAPPED		_sendOverlap;

	long	_IOCount;
	long	_iSending;
	int		_iSendCount;

	friend class CMMOServer;
};

class CPlayer : public CSESSION
{
protected:
	virtual bool OnAuth_ClientJoin(void) override;
	virtual bool OnAuth_PacketProc(void) override;
	virtual bool OnAuth_ClientLeave(bool bToGame) override;

	virtual bool OnGame_ClientJoin(void) override;
	virtual bool OnGame_PacketProc(void) override;
	virtual bool OnGame_ClientLeave(void) override;
	virtual bool OnGame_ClientRelease(void) override;
};