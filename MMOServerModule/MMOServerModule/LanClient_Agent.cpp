#include "stdafx.h"
#include "GameServer.h"
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
	WORD type;

	*pRecvPacket >> type;

	switch (type)
	{
		case en_PACKET_SS_AGENT_GAMESERVER_SHUTDOWN:
		{
			// Stop을 호출해야하는데 불완전한 포인터 형식이라서 안된다네?
			this->_pGameServer->Stop();
			break;
		}

		default:
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Wrong type packet");
			CCrashDump::Crash();
		}
	}
}

void CLanClient_Agent::OnSend(int iSendSize)
{

}