#include "stdafx.h"
#include "Config.h"
#include "CommonProtocol.h"
#include "MMOServer.h"
#include "LanClient_Game.h"
#include "GameServer.h"

CGameServer::CGameServer(int iClientMax) : CMMOServer(iClientMax)
{
	this->_loginServer_Client = new CLanClient_Game;

	this->pPlayerArray = new CPlayer[iClientMax];

	for (int i = 0; i < iClientMax; ++i)
		this->SetSessionArray(i, (void *)&this->pPlayerArray[i]);
}

CGameServer::~CGameServer()
{

}

//bool CGameServer::Start(void)
//{
//	CMMOServer::Start(g_Config.szNetBindIP, g_Config.iNetBindPort, g_Config.bNetNagleOpt, g_Config.iNetThreadNum);
//	return true;
//}

void CGameServer::OnAuth_Update(void)
{
	return;
}

void CGameServer::OnGame_Update(void)
{
	return;
}

bool CGameServer::CPlayer::OnAuth_ClientJoin(void)
{
	return true;
}

bool CGameServer::CPlayer::OnAuth_PacketProc(void)
{
	CPacket *pRecvPacket = NULL;
	WORD type;

	if (this->_completeRecvQ.GetUseSize() > 0)
	{
		for (int iCnt = 0; iCnt < dfAUTH_PACKET_PROC_REPEAT; ++iCnt)
		{
			this->_completeRecvQ.Dequeue(&pRecvPacket);
			*pRecvPacket >> type;

			switch (type)
			{
				case en_PACKET_CS_GAME_REQ_LOGIN:
				{
					PacketProc_Login(pRecvPacket);
				}

				default:
				{
					SYSLOG(L"SYSTEM", LOG::LEVEL_DEBUG, L"Wrong type packet");
					CCrashDump::Crash();
				}
			}
		}
	}

	return true;
}

bool CGameServer::CPlayer::OnAuth_ClientLeave(bool bToGame)
{
	return true;
}

bool CGameServer::CPlayer::OnGame_ClientJoin(void)
{
	this->_iSessionMode = en_SESSION_MODE::MODE_GAME;
	return true;
}

bool CGameServer::CPlayer::OnGame_PacketProc(void)
{
	while (true)
	{
		break;
	}

	return true;
}

bool CGameServer::CPlayer::OnGame_ClientLeave(void)
{
	return true;
}

bool CGameServer::CPlayer::OnGame_ClientRelease(void)
{
	return true;
}

void  CGameServer::CPlayer::PacketProc_Login(CPacket *pRecvPacket)
{
	return;
}