#include "stdafx.h"
#include "Config.h"
#include "MMOSession.h"
#include "MMOServer.h"
#include "LanClient_Game.h"
#include "GameServer.h"

CGameServer::CGameServer(int iClientMax) : CMMOServer(iClientMax)
{
	this->_loginServer_Client = new CLanClient_Game;
}

CGameServer::~CGameServer()
{

}

bool CGameServer::Start(void)
{
	CMMOServer::Start(g_Config.szNetBindIP, g_Config.iNetBindPort, g_Config.bNetNagleOpt, g_Config.iNetThreadNum);
	return true;
}

void CGameServer::OnAuth_Update(void)
{
	return;
}

void CGameServer::OnGame_Update(void)
{
	return;
}

