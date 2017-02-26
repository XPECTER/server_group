#include "stdafx.h"
#include "MMOSession.h"
#include "MMOServer.h"
#include "LanClient_Game.h"
#include "GameServer.h"

CGameServer::CGameServer(int iClientMax)
{
	this->_loginServer_Client = new CLanClient_Game;
}

CGameServer::~CGameServer()
{

}

void CGameServer::OnAuth_Update(void)
{
	return;
}

void CGameServer::OnGame_Update(void)
{
	return;
}

