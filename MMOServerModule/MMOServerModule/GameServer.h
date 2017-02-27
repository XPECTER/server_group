#pragma once



class CGameServer : public CMMOServer
{
public:
	CGameServer(int iClientMax);
	~CGameServer();

	bool Start(void);
protected:
	virtual void OnAuth_Update(void) override;
	virtual void OnGame_Update(void) override;

private:
	CLanClient_Game *_loginServer_Client;
};