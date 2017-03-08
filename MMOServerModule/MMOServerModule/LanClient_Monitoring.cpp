#include "stdafx.h"
#include "LanClient_Monitoring.h"

CLanClient_Monitoring::CLanClient_Monitoring(CGameServer *pGameServer)
{
	_bConnected = false;

	this->_pGameServer = pGameServer;
	wcscpy_s(this->_szServerGroupName, dfSERVER_GROUP_NAME_LEN, g_Config.szServerGroupName);
}

CLanClient_Monitoring::~CLanClient_Monitoring()
{

}

void CLanClient_Monitoring::OnEnterJoinServer(void)
{
	_bConnected = true;
	SendPacket_LoginToMonitoringServer();
}

void CLanClient_Monitoring::OnLeaveServer(void)
{
	_bConnected = false;
}

void CLanClient_Monitoring::OnRecv(CPacket *pRecvPacket)
{

}

void CLanClient_Monitoring::OnSend(int iSendSize)
{

}

void CLanClient_Monitoring::SendPacket_LoginToMonitoringServer()
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	MakePacket_LoginToMonitoringServer(pSendPacket);
	SendPacket(pSendPacket);
	pSendPacket->Free();
}

void CLanClient_Monitoring::MakePacket_LoginToMonitoringServer(CPacket *pSendPacket)
{
	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_LOGIN;
	*pSendPacket << (BYTE)dfMONITOR_SERVER_TYPE_GAME;
	pSendPacket->Enqueue((char *)this->_szServerGroupName, dfSERVER_GROUP_NAME_LEN * 2);

	HEADER header;
	header.wSize = (WORD)(pSendPacket->GetPayloadUseSize());
	pSendPacket->InputHeader((char *)&header, sizeof(header));
	return;
}