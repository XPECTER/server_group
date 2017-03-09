#include "stdafx.h"
#include "GameServer.h"
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
	return;
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

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_LOGIN;
	*pSendPacket << (BYTE)dfMONITOR_SERVER_TYPE_GAME;
	pSendPacket->Enqueue((char *)this->_szServerGroupName, dfSERVER_GROUP_NAME_LEN * 2);

	HEADER header;
	header.wSize = (WORD)(pSendPacket->GetPayloadUseSize());
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CLanClient_Monitoring::SendPacket_SessionCount(int iTotalSessionCount)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_SESSION;
	*pSendPacket << iTotalSessionCount;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CLanClient_Monitoring::SendPacket_AuthPlayer(int iAuthThreadSessionCount)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_AUTH_PLAYER;
	*pSendPacket << iAuthThreadSessionCount;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CLanClient_Monitoring::SendPacket_GamePlayer(int iGameThreadSessionCount)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_GAME_PLAYER;
	*pSendPacket << iGameThreadSessionCount;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CLanClient_Monitoring::SendPacket_AcceptTPS(int iAcceptTPS)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_ACCEPT_TPS;
	*pSendPacket << iAcceptTPS;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CLanClient_Monitoring::SendPacket_RecvPacketTPS(int iRecvPacketTPS)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_PACKET_PROC_TPS;
	*pSendPacket << iRecvPacketTPS;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CLanClient_Monitoring::SendPacket_SendPacketTPS(int iSendPacketTPS)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_PACKET_SEND_TPS;
	*pSendPacket << iSendPacketTPS;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CLanClient_Monitoring::SendPacket_DatabaseWriteTPS(int iDatabaseWriteTPS)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_DB_WRITE_TPS;
	*pSendPacket << iDatabaseWriteTPS;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CLanClient_Monitoring::SendPacket_DatabaseMsgCount(int iDatabaseMsgCount)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_DB_WRITE_MSG;
	*pSendPacket << iDatabaseMsgCount;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CLanClient_Monitoring::SendPacket_AuthThreadTPS(int iAuthThreadLoopTPS)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_AUTH_THREAD_FPS;
	*pSendPacket << iAuthThreadLoopTPS;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CLanClient_Monitoring::SendPacket_GameThreadTPS(int iGameThreadLoopTPS)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_GAME_THREAD_FPS;
	*pSendPacket << iGameThreadLoopTPS;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CLanClient_Monitoring::SendPacket_PacketUseCount(int iPacketUseCount)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE;
	*pSendPacket << (BYTE)dfMONITOR_DATA_TYPE_GAME_PACKET_POOL;
	*pSendPacket << iPacketUseCount;
	*pSendPacket << (int)(time(NULL));

	HEADER header;
	header.wSize = pSendPacket->GetPayloadUseSize();
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}