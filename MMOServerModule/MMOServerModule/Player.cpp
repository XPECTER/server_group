#include "stdafx.h"
#include "GameServer.h"

CGameServer::CPlayer::CPlayer()
{
	
}

CGameServer::CPlayer::~CPlayer()
{

}

void CGameServer::CPlayer::Player_Init(CGameServer *pGameServer)
{
	// 배열 초기화에 이니셜라이저를 못넣어서 이렇게 했는데
	// 방법이 있을까??
	this->_clientID = -1;
	this->_connectInfo = NULL;
	this->_recvQ.ClearBuffer();
	this->_iSessionMode = CSession::MODE_NONE;
	this->_bAuthToGame = false;
	this->_bLogout = false;
	//ZeroMemory(&this->_recvOverlap, sizeof(OVERLAPPED));
	//ZeroMemory(&this->_sendOverlap, sizeof(OVERLAPPED));
	this->_IOCount = 0;
	this->_iSending = FALSE;
	this->_iSendCount = 0;

	this->_heartBeatTick = time(NULL);
	this->_pGameServer = pGameServer;
}

void CGameServer::CPlayer::CheckHeartBeat(void)
{
	if (time(NULL) - this->_heartBeatTick > dfCLIENT_HEARTBEAT_TICK)
		this->Disconnect();

	return;
}

void CGameServer::CPlayer::Action_Move()
{

}

void CGameServer::CPlayer::Action_Attack()
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

	for (int iCnt = 0; iCnt < dfAUTH_PACKET_PROC_REPEAT; ++iCnt)
	{
		if (this->_completeRecvQ.GetUseSize() > 0)
		{
			this->_completeRecvQ.Dequeue(&pRecvPacket);
			*pRecvPacket >> type;

			switch (type)
			{
				case en_PACKET_CS_GAME_REQ_LOGIN:
				{
					PacketProc_Login(pRecvPacket);
					break;
				}

				case en_PACKET_CS_GAME_REQ_CHARACTER_SELECT:
				{
					PacketProc_CharacterSelect(pRecvPacket);
					break;
				}

				case en_PACKET_CS_GAME_REQ_HEARTBEAT:
				{
					PacketProc_ClientHeartBeat(pRecvPacket);
					break;
				}

				default:
				{
					SYSLOG(L"SYSTEM", LOG::LEVEL_DEBUG, L"Wrong type packet");
					CCrashDump::Crash();
				}
			}

			pRecvPacket->Free();
		}
		else
			break;
	}

	return true;
}

bool CGameServer::CPlayer::OnAuth_ClientLeave(bool bToGame)
{
	return true;
}

bool CGameServer::CPlayer::OnGame_ClientJoin(void)
{
	// 여기서 캐릭터 생성을 한다.
	if (dfDUMMY_ACCOUNTNO_LIMIT < this->_accountNo)
	{
		if (1 == this->_byParty)
		{
			this->_serverX = dfCREATE_PLAYER_X_PARTY1;
			this->_serverY = dfCREATE_PLAYER_Y_PARTY1;
		}
		else
		{
			this->_serverX = dfCREATE_PLAYER_X_PARTY2;
			this->_serverY = dfCREATE_PLAYER_Y_PARTY2;
		}
	}
	else
	{
		this->_serverX = dfCREATE_PLAYER_X_DUMMY;
		this->_serverY = dfCREATE_PLAYER_Y_DUMMY;
	}
	
	this->_clientPosX = TILE_to_POS_X(this->_serverX);
	this->_clientPosY = TILE_to_POS_Y(this->_serverY);

	this->_rotation = ((rand() % 8) + 1);
	
	this->_iHP = dfHP_MAX;

	// 여기서 내 주변 섹터 유저에게도 날려줘야 한다.
	this->_pGameServer->_sector->MoveSector(-1, -1, TILE_to_SECTOR_X(this->_serverX), TILE_to_SECTOR_Y(this->_serverY), this->_clientID, this);
	this->_pGameServer->_field->AddTileObject(this->_serverX, this->_serverY, this->_clientID);

	SendPacket_CreateCharacter();
	return true;
}

bool CGameServer::CPlayer::OnGame_PacketProc(void)
{
	CPacket *pRecvPacket = NULL;
	WORD type;

	for (int iCnt = 0; iCnt < dfGAME_PACKET_PROC_REPEAT; ++iCnt)
	{
		if (this->_completeRecvQ.GetUseSize() > 0)
		{
			this->_completeRecvQ.Dequeue(&pRecvPacket);
			*pRecvPacket >> type;

			switch (type)
			{
				case en_PACKET_CS_GAME_REQ_ECHO:
				{
					PacketProc_ReqEcho(pRecvPacket);
					break;
				}

				case en_PACKET_CS_GAME_REQ_HEARTBEAT:
				{
					PacketProc_ClientHeartBeat(pRecvPacket);
					break;
				}

				case en_PACKET_CS_GAME_REQ_MOVE_CHARACTER:
				{
					PacketProc_MoveCharacter(pRecvPacket);
					break;
				}

				case en_PACKET_CS_GAME_REQ_STOP_CHARACTER:
				{
					PacketProc_StopCharacter(pRecvPacket);
					break;
				}

				default:
				{
					SYSLOG(L"SYSTEM", LOG::LEVEL_DEBUG, L"Wrong type packet");
					CCrashDump::Crash();
				}
			}

			pRecvPacket->Free();
		}
		else
			break;
	}

	return true;
}

bool CGameServer::CPlayer::OnGame_ClientLeave(void)
{
	st_DBWRITER_MSG *pMsg = this->_pGameServer->_databaseMsgPool.Alloc();
	pMsg->Type_DB = dfDBWRITER_TYPE_ACCOUNT;
	pMsg->Type_Message = enDB_ACCOUNT_WRITE_STATUS_LOGOUT;

	stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in data;
	data.AccountNo = this->_accountNo;
	memcpy_s(pMsg->Message, dfDBWRITER_MSG_MAX, &data, sizeof(stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in));

	this->_pGameServer->_databaseMsgQueue.Enqueue(pMsg);
	return true;
}

bool CGameServer::CPlayer::OnGame_ClientRelease(void)
{
	this->_clientID = -1;
	this->_connectInfo = NULL;
	this->_recvQ.ClearBuffer();
	this->_iSessionMode = CSession::MODE_NONE;
	this->_bAuthToGame = false;
	this->_bLogout = false;
	this->_iSendCount = 0;

	CPacket *pPacket = NULL;
	while (0 < this->_completeRecvQ.GetUseSize())
	{
		this->_completeRecvQ.Dequeue(&pPacket);
		pPacket->Free();
	}

	while (0 < this->_sendQ.GetUseSize())
	{
		this->_sendQ.Dequeue(&pPacket);
		pPacket->Free();
	}

	return true;
}

void  CGameServer::CPlayer::PacketProc_Login(CPacket *pRecvPacket)
{
	__int64 iAccountNo;
	char SessionKey[64];

	*pRecvPacket >> iAccountNo;
	pRecvPacket->Dequeue(SessionKey, 64);

	// 1. 세션 키 검사 후 없다면 로그인 실패 바로 전송
	// 2. DB에서 accountNo로 검색 후 로그인 성공 여부 전송
	this->_heartBeatTick = time(NULL);
	
	CPacket *pSendPacket = CPacket::Alloc();
	if (dfDUMMY_ACCOUNTNO_LIMIT < iAccountNo)
	{
		if (!this->_pGameServer->CheckSessionKey(iAccountNo, SessionKey))
		{
			MakePacket_ResLogin(pSendPacket, 0, iAccountNo);
			SendPacket(pSendPacket);
			pSendPacket->Free();

			return;
		}
	}

	// DB검색
	stDB_ACCOUNT_READ_USER_in in;
	stDB_ACCOUNT_READ_USER_out out;
	in.AccountNo = iAccountNo;
	
	this->_pGameServer->_database_Account->QueryDB(enDB_ACCOUNT_READ_USER, &in, &out);

	if (dfGAME_LOGIN_FAIL == out.Status || dfGAME_LOGIN_VERSION_MISS == out.Status)
	{
		MakePacket_ResLogin(pSendPacket, out.Status, iAccountNo);
		SendPacket(pSendPacket);
		pSendPacket->Free();

		// 패킷 하나 보내고 끊는 로직이 필요하다.
	}
	else
	{
		MakePacket_ResLogin(pSendPacket, out.Status, iAccountNo);
		SendPacket(pSendPacket);
		pSendPacket->Free();

		// 유저 정보 세팅
		this->_accountNo = iAccountNo;
		wcscpy_s(this->_szNickname, dfNICK_MAX_LEN, out.szNick);
		this->_byParty = out.Party;
	}
	
	/*if (dfDUMMY_ACCOUNTNO_LIMIT >= iAccountNo)
		SetMode_Game();*/

	return;
}

void CGameServer::CPlayer::MakePacket_ResLogin(CPacket *pSendPacket, BYTE byStatus, __int64 iAccountNo)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_LOGIN;
	*pSendPacket << byStatus;
	*pSendPacket << iAccountNo;

	return;
}

void CGameServer::CPlayer::PacketProc_CharacterSelect(CPacket *pRecvPacket)
{
	BYTE characterType;
	BYTE byResult;

	*pRecvPacket >> characterType;

	this->_heartBeatTick = time(NULL);
	byResult = 0;
	
	if (1 == this->_byParty)
	{
		if (1 <= characterType && 3 >= characterType)
			byResult = 1;
	}
	else if (2 == this->_byParty)
	{
		if (3 <= characterType && 5 >= characterType)
			byResult = 1;
	}
	else
	{
		SYSLOG(L"PACKET", LOG::LEVEL_DEBUG, L"unknown party num");
		CCrashDump::Crash();
	}

	CPacket *pSendPacket = CPacket::Alloc();
	MakePacket_ResCharacterSelect(pSendPacket, byResult);
	SendPacket(pSendPacket);
	pSendPacket->Free();

	if (1 == byResult)
	{
		this->_characterType = characterType;
		SetMode_Game();
	}

	return;
}

void CGameServer::CPlayer::MakePacket_ResCharacterSelect(CPacket *pSendPacket, BYTE byStatus)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_CHARACTER_SELECT;
	*pSendPacket << byStatus;
}

void CGameServer::CPlayer::PacketProc_ReqEcho(CPacket *pRecvPacket)
{
	__int64 iAccountNo;
	__int64 SendTick;

	this->_heartBeatTick = time(NULL);

	*pRecvPacket >> iAccountNo;
	*pRecvPacket >> SendTick;

	CPacket *pSendPacket = CPacket::Alloc();
	MakePacket_ResEcho(pSendPacket, iAccountNo, SendTick);
	SendPacket(pSendPacket);
	pSendPacket->Free();

	return;
}

void CGameServer::CPlayer::MakePacket_ResEcho(CPacket *pSendPacket, __int64 iAccountNo, __int64 SendTick)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_ECHO;
	*pSendPacket << iAccountNo;
	*pSendPacket << SendTick;

	return;
}

void CGameServer::CPlayer::PacketProc_ClientHeartBeat(CPacket *pRecvPacket)
{
	this->_heartBeatTick = time(NULL);
	return;
}

void CGameServer::CPlayer::SendPacket_CreateCharacter(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	MakePacket_CreateCharacter(pSendPacket, this->_clientID, this->_characterType, this->_szNickname, this->_clientPosX, this->_clientPosY, this->_rotation, this->_iHP, this->_byParty);
	SendPacket(pSendPacket);
	pSendPacket->Free();

	return;
}

void CGameServer::CPlayer::MakePacket_CreateCharacter(CPacket *pSendPacket, __int64 clientID, BYTE characterType, wchar_t *szNickname, float PosX, float PosY, WORD rotation, int hp, BYTE party)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_CREATE_MY_CHARACTER;
	*pSendPacket << clientID;
	*pSendPacket << characterType;
	pSendPacket->Enqueue((char *)szNickname, dfNICK_BYTE_LEN);
	*pSendPacket << PosX;
	*pSendPacket << PosY;
	*pSendPacket << rotation;
	*pSendPacket << hp;
	*pSendPacket << party;

	return;
}

void CGameServer::CPlayer::PacketProc_MoveCharacter(CPacket *pRecvPacket)
{
	__int64 clientID;
	float startX;
	float startY;
	float destX;
	float destY;

	*pRecvPacket >> clientID >> startX >> startY >> destX >> destY;

	if (this->CheckErrorRange(startX, startY))
	{
		SendPacket_Sync();
	}
	else
	{
		this->_clientPosX = startX;
		this->_clientPosY = startY;
		this->_serverX = POS_to_TILE_X(startX);
		this->_serverY = POS_to_TILE_Y(startY);
	}

	PATH path[dfPATH_POINT_MAX];
	int iPathCount;
	
	CPacket *pSendPacket = CPacket::Alloc();
	if (this->_pGameServer->_jps->FindPath(this->_serverX, this->_serverY, POS_to_TILE_X(destX), POS_to_TILE_Y(destY), path, &iPathCount))
	{
		MakePacket_ResMoveCharacter(pSendPacket, clientID, (BYTE)iPathCount, path);
	}
	else
	{
		// 캐릭터 정지 패킷 보내기
		// 사유 : 길을 못 찾음
		MakePacket_StopCharacter(pSendPacket, clientID, this->_clientPosX, this->_clientPosY, 0xFFFF);
	}

	SendPacket(pSendPacket);
	pSendPacket->Free();

	return;
}

void CGameServer::CPlayer::MakePacket_ResMoveCharacter(CPacket *pSendPacket, __int64 clientID, BYTE pathCount, PATH *pPath)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_MOVE_CHARACTER;
	*pSendPacket << clientID;
	*pSendPacket << pathCount;
	pSendPacket->Enqueue((char *)pPath, sizeof(PATH) * pathCount);

	return;
}

void CGameServer::CPlayer::PacketProc_StopCharacter(CPacket *pRecvPacket)
{
	__int64 clientID;
	float PosX;
	float PosY;
	WORD rotation;

	*pRecvPacket >> clientID >> PosX >> PosY >> rotation;

	if (this->CheckErrorRange(PosX, PosY))
	{
		SendPacket_Sync();
	}
	else
	{
		this->_clientPosX = PosX;
		this->_clientPosY = PosY;
		this->_serverX = POS_to_TILE_X(PosX);
		this->_serverY = POS_to_TILE_Y(PosY);
	}

	return;
}

void CGameServer::CPlayer::MakePacket_StopCharacter(CPacket *pSendPacket, __int64 clientID, float PosX, float PosY, WORD dir)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_STOP_CHARACTER;
	*pSendPacket << clientID;
	*pSendPacket << PosX;
	*pSendPacket << PosY;
	*pSendPacket << dir;

	return;
}

void CGameServer::CPlayer::SendPacket_Sync(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	MakePacket_Sync(pSendPacket, this->_clientID, this->_serverX, this->_serverY);
	SendPacket(pSendPacket);
	pSendPacket->Free();

	return;
}

void CGameServer::CPlayer::MakePacket_Sync(CPacket *pSendPacket, __int64 clientID, short tileX, short tileY)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_CHARACTER_SYNC;
	*pSendPacket << clientID;
	*pSendPacket << ((float)TILE_to_POS_X(tileX));
	*pSendPacket << ((float)TILE_to_POS_Y(tileY));

	return;
}

bool CGameServer::CPlayer::CheckErrorRange(float PosX, float PosY)
{
	if (PosX > this->_clientPosX + dfPOSITON_ERROR_RANGE_X ||
		PosX < this->_clientPosX - dfPOSITON_ERROR_RANGE_X ||
		PosY > this->_clientPosY + dfPOSITON_ERROR_RANGE_Y ||
		PosY < this->_clientPosY - dfPOSITON_ERROR_RANGE_Y)
	{
		return true;
	}
	else
		return false;
}