#include "stdafx.h"
#include "LanClient_Login.h"

CLanClient_Login::CLanClient_Login(CGameServer *pGameServer)
{
	this->_pGameServer = pGameServer;

	wcscpy_s(this->_szServerGroupName, dfSERVER_GROUP_NAME_LEN, g_Config.szServerGroupName);
}

CLanClient_Login::~CLanClient_Login()
{

}

void CLanClient_Login::OnEnterJoinServer(void)
{

}

void CLanClient_Login::OnLeaveServer(void)
{

}

void CLanClient_Login::OnRecv(CPacket *pRecvPacket)
{

}

void CLanClient_Login::OnSend(int iSendSize)
{

}