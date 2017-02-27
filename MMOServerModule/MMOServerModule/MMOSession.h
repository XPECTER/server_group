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
		MODE_NONE = 0,						// �� ���� ����
		MODE_AUTH,							// Accept �� ���� ���
		MODE_AUTH_TO_GAME,					// ���� ó�� �� �α��� �Ϸ�
		MODE_GAME,							// ���� ��忡�� ���� ���� ��ȯ
		MODE_LOGOUT_IN_AUTH,				// AuthThread���� close
		MODE_LOGOUT_IN_GAME,				// GameThread���� close
		MODE_WAIT_LOGOUT,					// ���� ������
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