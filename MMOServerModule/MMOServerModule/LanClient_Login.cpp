#include "stdafx.h"
#include "GameServer.h"
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
	_bConnected = true;
	SendPacket_LoginToLoginServer();
	return;
}

void CLanClient_Login::OnLeaveServer(void)
{
	_bConnected = false;
}

void CLanClient_Login::OnRecv(CPacket *pRecvPacket)
{
	WORD type;
	*pRecvPacket >> type;

	switch (type)
	{
	case en_PACKET_SS_REQ_NEW_CLIENT_LOGIN:
		PacketProc_RequestNewClientLogin(pRecvPacket);
		break;

	default:
		SYSLOG(L"PACKET", LOG::LEVEL_ERROR, L"wrong type packet on lanclient");
		break;
	}

	return;
}

void CLanClient_Login::OnSend(int iSendSize)
{

}

void CLanClient_Login::SendPacket_LoginToLoginServer(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_LOGINSERVER_LOGIN;
	*pSendPacket << (BYTE)dfSERVER_TYPE_GAME;
	pSendPacket->Enqueue((char *)this->_szServerGroupName, dfSERVER_GROUP_NAME_LEN * 2);

	HEADER header;
	header.wSize = (WORD)(pSendPacket->GetPayloadUseSize());
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CLanClient_Login::SendPacket_HeartBeat(BYTE type)
{
	CPacket *pSendPacket = CPacket::Alloc();
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_HEARTBEAT;
	*pSendPacket << (BYTE)type;

	HEADER header;
	header.wSize = (WORD)(pSendPacket->GetPayloadUseSize());
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CLanClient_Login::PacketProc_RequestNewClientLogin(CPacket *pRecvPacket)
{
	__int64 accountNo;
	char sessionKey[64] = { 0, };
	__int64 parameter;

	*pRecvPacket >> accountNo;
	pRecvPacket->Dequeue(sessionKey, 64);
	*pRecvPacket >> parameter;

	this->_pGameServer->AddSessionKey(accountNo, sessionKey);

	CPacket *pSendPacket = CPacket::Alloc();
	MakePacket_ResponseNewClientLogin(pSendPacket, accountNo, parameter);
	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CLanClient_Login::MakePacket_ResponseNewClientLogin(CPacket *pSendPacket, __int64 accountNo, __int64 parameter)
{
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_RES_NEW_CLIENT_LOGIN;
	*pSendPacket << accountNo;
	*pSendPacket << parameter;

	HEADER header;
	header.wSize = (WORD)(pSendPacket->GetPayloadUseSize());
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	return;
}