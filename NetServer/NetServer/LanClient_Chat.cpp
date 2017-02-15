#include "stdafx.h"
#include "LanClient_Chat.h"
#include "ChatServer.h"
#include "main.h"



CLanClient_Chat::CLanClient_Chat(CChatServer *pServerPtr)
{
	this->_pChatServer = pServerPtr;
	this->_RecvAddSessionPacket = 0;
}

CLanClient_Chat::~CLanClient_Chat()
{

}

void CLanClient_Chat::OnEnterJoinServer(void)
{
	SendPacket_LoginServerLogin();
	SYSLOG(L"SYSTEM", LOG::LEVEL_DEBUG, L"the chatserver is connected to loginserver");

	return;
}

void CLanClient_Chat::OnLeaveServer(void)
{

}

void CLanClient_Chat::OnRecv(CPacket *pRecvPacket)
{
	WORD type;
	*pRecvPacket >> type;

	switch (type)
	{
		case en_PACKET_SS_REQ_NEW_CLIENT_LOGIN:
			InterlockedIncrement64(&this->_RecvAddSessionPacket);
			PacketProc_RequestNewClientLogin(pRecvPacket);
			break;

		default:
			SYSLOG(L"PACKET", LOG::LEVEL_ERROR, L"wrong type packet on lanclient");
			break;
	}

	return;
}

void CLanClient_Chat::OnSend(int iSendSize)
{

}

void CLanClient_Chat::SendPacket_LoginServerLogin(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	MakePacket_LoginServerLogin(pSendPacket);
	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CLanClient_Chat::SendPacket_HeartBeat(BYTE threadType)
{
	CPacket *pSendPacket = CPacket::Alloc();
	MakePacket_HeartBeat(pSendPacket, threadType);
	SendPacket(pSendPacket);
	pSendPacket->Free();
	return;
}

void CLanClient_Chat::MakePacket_LoginServerLogin(CPacket *pSendPacket)
{
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_LOGINSERVER_LOGIN;
	*pSendPacket << (BYTE)dfSERVER_TYPE_CHAT;
	pSendPacket->Enqueue((char *)g_ConfigData._szServerGroupName, dfSERVER_NAME_LEN * 2);

	HEADER header;
	header.wSize = (WORD)(pSendPacket->GetPayloadUseSize());
	pSendPacket->InputHeader((char *)&header, sizeof(header));

	return;
}

void CLanClient_Chat::MakePacket_ResponseNewClientLogin(CPacket *pSendPacket, __int64 accountNo, __int64 parameter)
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

void CLanClient_Chat::MakePacket_HeartBeat(CPacket *pSendPacket, BYTE threadType)
{
	pSendPacket->SetHeaderSize(2);

	*pSendPacket << (WORD)en_PACKET_SS_HEARTBEAT;
	*pSendPacket << threadType;

	HEADER header;
	header.wSize = (WORD)(pSendPacket->GetPayloadUseSize());
	pSendPacket->InputHeader((char *)&header, sizeof(header));
	return;
}

void CLanClient_Chat::PacketProc_RequestNewClientLogin(CPacket *pRecvPacket)
{
	__int64 accountNo;
	char sessionKey[64] = { 0, };
	__int64 parameter;

	*pRecvPacket >> accountNo;
	pRecvPacket->Dequeue(sessionKey, 64);
	*pRecvPacket >> parameter;

 	this->_pChatServer->AddSessionKey(accountNo, sessionKey);

	CPacket *pSendPacket = CPacket::Alloc();
	
	MakePacket_ResponseNewClientLogin(pSendPacket, accountNo, parameter);
	SendPacket(pSendPacket);

	pSendPacket->Free();
	SYSLOG(L"PACKET", LOG::LEVEL_ERROR , L"%05d", accountNo);
	return;
}