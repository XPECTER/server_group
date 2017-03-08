#include "stdafx.h"
#include "LanClient_Agent.h"

CLanClient_Agent::CLanClient_Agent(CGameServer *pGameServer)
{
	this->_pGameServer = pGameServer;

	wcscpy_s(this->_szServerGroupName, dfSERVER_GROUP_NAME_LEN, g_Config.szServerGroupName);
}

CLanClient_Agent::~CLanClient_Agent()
{

}

void CLanClient_Agent::OnEnterJoinServer(void)
{

}

void CLanClient_Agent::OnLeaveServer(void)
{

}

void CLanClient_Agent::OnRecv(CPacket *pRecvPacket)
{

}

void CLanClient_Agent::OnSend(int iSendSize)
{

}