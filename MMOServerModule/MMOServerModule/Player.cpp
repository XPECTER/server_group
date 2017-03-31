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
	// �迭 �ʱ�ȭ�� �̴ϼȶ������� ���־ �̷��� �ߴµ�
	// ����� ������??
	// Session Part
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

	// Contents Part
	this->_pGameServer = pGameServer;
	this->_accountNo = -1;
	this->_heartBeatTick = time(NULL);
	//this->_szNickname = NULL;
	this->_byParty = 0;
	this->_characterType = 0;
	this->_serverX_curr = -1;
	this->_serverY_curr = -1;
	this->_serverX_prev = -1;
	this->_serverY_prev = -1;

	this->_clientPosX = -1;
	this->_clientPosY = -1;

	this->_sectorX_curr = -1;
	this->_sectorY_curr = -1;
	this->_sectorX_prev = -1;
	this->_sectorY_prev = -1;
	
	this->_rotation = eMOVE_NN;
	//this->_bMove = false;
	//this->_path;
	this->_pathCount = 0;
	this->_pPath = NULL;
	
	this->_iHP = 0;
	this->_byDie = 0;
	
	this->_targetID = -1;
}

void CGameServer::CPlayer::CheckHeartBeat(void)
{
	if (time(NULL) - this->_heartBeatTick > dfCLIENT_HEARTBEAT_TICK)
		this->Disconnect();

	return;
}


/////////////////////////////////////////////////////////////////////
//	�Լ� : Action_Move()
//
//	1. ������ ���ӻ������� Ȯ��(��� ���� ��� Loop�� �ϱ� ����)
//	
//	2. ���� ��ΰ� �ִ��� Ȯ��
//
//	3. �̵��� �ð��� �Ǿ����� Ȯ��
//		3-1. ������ ��ǥ�� ���� ���� ĭ ���� ���ϱ�
//		3-2. ���� Ÿ�� ��ǥ ���ϱ�
//		3-3. ���� Ÿ�� �̵� �ð� ���ϱ�
//		3-4. Ÿ�� �̵�
//		3-5. Ÿ�� �̵� �� ���Ͱ� �ٲ������ ���� ��ǥ ����.
//		3-6. �ű� ���Ϳ��� ĳ���� ���� ��Ŷ, ���� �̵� ���̿��ٸ� �̵� ��Ŷ�� ����
//		3-7. ���� ���Ϳ��� ĳ���� ���� ��Ŷ ����
//		3-8. �ű� ���Ϳ� �ִ� ������ ������ ��� �ش� �������� ĳ���� ���� ��Ŷ, �̵� ���̿��ٸ� �̵� ��Ŷ���� ����
//
/////////////////////////////////////////////////////////////////////
void CGameServer::CPlayer::Action_Move()
{
	int iCurrTileX;
	int iCurrTileY;
	int iNextTileX;
	int iNextTileY;
	int iDestX;
	int iDestY;
	en_DIRECTION rotation;

	if (MODE_GAME != this->_iSessionMode)
		return;

	if (NULL == this->_pPath)
		return;

	if (this->_byDie)
		return;

	if (GetTickCount64() >= this->_nextTileTime)
	{
		iCurrTileX = this->_serverX_curr;
		iCurrTileY = this->_serverY_curr;
		iDestX = POS_to_TILE_X(this->_pPath[this->_path_curr].X);
		iDestY = POS_to_TILE_Y(this->_pPath[this->_path_curr].Y);

		if (iCurrTileX == iDestX && iCurrTileY == iDestY)
		{
			if (this->_pathCount == ++this->_path_curr)
			{
				this->_pPath = NULL;
				return;
			}

			iDestX = POS_to_TILE_X(this->_pPath[this->_path_curr].X);
			iDestY = POS_to_TILE_Y(this->_pPath[this->_path_curr].Y);
		}

		rotation = MoveDirection(iCurrTileX, iCurrTileY, iDestX, iDestY);
		this->_rotation = rotation;
		MoveTile(rotation, iCurrTileX, iCurrTileY, &iNextTileX, &iNextTileY);
		this->_nextTileTime = NextTileTime(rotation);

		if ((-1) != iNextTileX)
		{
			Move(iNextTileX, iNextTileY);
			SYSLOG(L"GAME", LOG::LEVEL_DEBUG, L" # Cooldinate / [ServerX : %d][ServerY : %d][ClientX : %f][ClientY : %f]", this->_serverX_curr, this->_serverY_curr, this->_clientPosX, this->_clientPosY);
		}
		else
			CCrashDump::Crash();
	}
	else
		return;
}


/////////////////////////////////////////////////////////////////////
//	�Լ� : Action_Attack()
//
//	1. ������ ���ӻ������� Ȯ��(��� ���� ��� Loop�� �ϱ� ����)
//	
//
/////////////////////////////////////////////////////////////////////
void CGameServer::CPlayer::Action_Attack()
{
	CField<CLIENT_ID> *pField = this->_pGameServer->_field;
	CSector *pSector = this->_pGameServer->_sector;
	BYTE characterType = this->_characterType;
	CGameServer *pServer = this->_pGameServer;
	int attackType = this->_attackType;
	CLIENT_ID clientID = this->_clientID;
	
	bool bDie = false;
	float clientPosX;
	float clientPosY;
	float fPushX;
	float fPushY;
	int tileX;
	int	tileY;
	int damageValue;

	CPacket *pSendPacket = NULL;
	std::list<CLIENT_ID> *list = NULL;
	UINT64 currTick = GetTickCount64();

	if (MODE_GAME != this->_iSessionMode)
		return;

	// ���� ���� ��ǥ ����. �� �۾��� �� ���� ������ ���°� ����.
	if (dfDUMMY_ACCOUNTNO_LIMIT < clientID && dfGUEST_ACCOUNTNO_OVER > clientID)
		pServer->CollectRealUserSectorPos(this->_sectorX_curr, this->_sectorY_curr);

	if ((-1) == this->_targetID)
		return;

	if (this->_byDie)
		return;

	if (currTick < this->_nextAttackTime)
		return;

	CPlayer *pTargetPlayer = this->_targetPtr;
	if (NULL == pTargetPlayer)
		CCrashDump::Crash();

	if (pTargetPlayer->_clientID == this->_targetID)
	{
		if (pTargetPlayer->_byDie)
		{
			MoveStop(true);
			this->_targetID = -1;
			this->_targetPtr = NULL;
			return;
		}

		float distance = Distance(this->_serverX_curr, this->_serverY_curr, pTargetPlayer->_serverX_curr, pTargetPlayer->_serverY_curr);

		if (g_Pattern_AttackRange[attackType][characterType] >= distance)
		{
			// ���� ��Ŷ.
			MoveStop(true);

			// ���� �ȿ� �����Ƿ� ����
			if (1 == this->_attackType)
			{
				// �з��� ��ǥ���ϰ�
				if (3 != this->_characterType)
				{
					AttackPushPos(this->_clientPosX, this->_clientPosY, pTargetPlayer->_clientPosX, pTargetPlayer->_clientPosY, &fPushX, &fPushY);
					tileX = POS_to_TILE_X(fPushX);
					tileY = POS_to_TILE_Y(fPushY);

					// �з��� ��ǥ�� �̵��� �� �ִ� ������ �˻�
					if (!pField->CheckTileMove(tileX, tileY))
					{
						// ������ ��Ŷ�� �з��� ��ǥ 0�� ������ ���ڸ�
						fPushX = 0;
						fPushY = 0;
					}
					else
					{
						pTargetPlayer->Move(tileX, tileY);
					}
				}
				else
				{
					// ���� ���� ������ �ȹз����� �غ�
					fPushX = 0;
					fPushY = 0;
				}

				// ���� ���� �ð�
				this->_nextAttackTime = currTick + g_Pattern_AttackTime[1][characterType];
				this->_nextTileTime = currTick + dfATTACK_STOP_TIME;

				// ���� ��� ���� �ð� ���ϱ�
				pTargetPlayer->_nextAttackTime = currTick + dfDAMAGE_STUN_TIME;
				pTargetPlayer->_nextTileTime = currTick + dfDAMAGE_STUN_TIME;

				

				// ��Ŷ ������ // ���� �׼�, ������(���� �� ��������)
				pSendPacket = CPacket::Alloc();
				MakePacket_Attack(pSendPacket, 1, this->_clientID, pTargetPlayer->_clientID, this->_nextAttackTime, this->_clientPosX, this->_clientPosY);
				pServer->SendPacket_SectorAround(pSendPacket, this->_sectorX_curr, this->_sectorY_curr, NULL);
				pSendPacket->Free();

				damageValue = g_Pattern_AttackPower[1][characterType];

				pSendPacket = CPacket::Alloc();
				MakePacket_Damaged(pSendPacket, this->_targetID, damageValue, fPushX, fPushY);
				pServer->SendPacket_SectorAround(pSendPacket, pTargetPlayer->_sectorX_curr, pTargetPlayer->_sectorY_curr, NULL);
				pSendPacket->Free();

				if (0 > (pTargetPlayer->_iHP -= damageValue))
				{
					pTargetPlayer->_iHP = 0;
					pTargetPlayer->_byDie = 1;
					pField->MoveTileObject(pTargetPlayer->_serverX_curr, pTargetPlayer->_serverY_curr, (-1), (-1), pTargetPlayer->_clientID);
					bDie = true;
				}

				if (bDie)
				{
					this->_targetID = -1;
					this->_targetPtr = NULL;

					pSendPacket = CPacket::Alloc();
					MakePacket_Die(pSendPacket, pTargetPlayer->_clientID);
					pServer->SendPacket_SectorAround(pSendPacket, pTargetPlayer->_sectorX_curr, pTargetPlayer->_sectorY_curr, NULL);
					pSendPacket->Free();

					st_DBWRITER_MSG *pMsg = pServer->_databaseMsgPool.Alloc();
					pMsg->Type_DB = dfDBWRITER_TYPE_GAME;
					pMsg->Type_Message = enDB_GAME_WRITE_PLAYER_DIE;

					stDB_GAME_WRITE_PLAYER_DIE_in dieMessage;
					dieMessage.AccountNo = pTargetPlayer->_accountNo;
					dieMessage.AttackerAccountNo = this->_accountNo;
					dieMessage.DiePosX = pTargetPlayer->_serverX_curr;
					dieMessage.DiePosY = pTargetPlayer->_serverY_curr;

					memcpy_s(pMsg->Message, dfDBWRITER_MSG_MAX, &dieMessage, sizeof(stDB_GAME_WRITE_PLAYER_DIE_in));
					pServer->_databaseMsgQueue.Enqueue(pMsg);

					pMsg = pServer->_databaseMsgPool.Alloc();
					pMsg->Type_DB = dfDBWRITER_TYPE_GAME;
					pMsg->Type_Message = enDB_GAME_WRITE_PLAYER_KILL;

					stDB_GAME_WRITE_PLAYER_KILL_in killMessage;
					killMessage.AccountNo = this->_accountNo;
					killMessage.TargetAccountNo = pTargetPlayer->_accountNo;
					killMessage.KillPosX = this->_serverX_curr;
					killMessage.KillPosY = this->_serverY_curr;

					memcpy_s(pMsg->Message, dfDBWRITER_MSG_MAX, &killMessage, sizeof(stDB_GAME_WRITE_PLAYER_KILL_in));
					pServer->_databaseMsgQueue.Enqueue(pMsg);
				}

				st_CLIENT_POS *pPos = pServer->_battleAreaPosPool.Alloc();
				pPos->_PosX = this->_clientPosX;
				pPos->_PosY = this->_clientPosY;

				pServer->_battleAreaPosList.push_back(pPos);
			}
			else
			{
				// ���� ���� �ð�
				this->_nextAttackTime = currTick + g_Pattern_AttackTime[2][characterType];
				this->_nextTileTime = currTick + dfATTACK_STOP_TIME;

				// ��Ŷ ������ // ���� �׼�, ������(���� �� ��������)
				pSendPacket = CPacket::Alloc();
				MakePacket_Attack(pSendPacket, 2, this->_clientID, pTargetPlayer->_clientID, this->_nextAttackTime, this->_clientPosX, this->_clientPosY);
				pServer->SendPacket_SectorAround(pSendPacket, this->_sectorX_curr, this->_sectorY_curr, NULL);
				pSendPacket->Free();

				clientPosX = this->_clientPosX;
				clientPosY = this->_clientPosY;

				damageValue = g_Pattern_AttackPower[2][characterType];

				// ���� ���� üũ
				for (int iCnt = 0; iCnt < dfPATTERN_ATTACK_AREA_MAX; ++iCnt)
				{
					if (255 == g_Pattern_AttackArea[this->_characterType][iCnt][0])
						break;

					tileX = this->_serverX_curr + g_Pattern_AttackArea[this->_characterType][iCnt][0];
					tileY = this->_serverY_curr + g_Pattern_AttackArea[this->_characterType][iCnt][1];

					if (!pField->GetTileObject(tileX, tileY, list))
						continue;

					auto iter = list->begin();
					for (iter; iter != list->end(); ++iter)
					{
						pTargetPlayer = &pServer->_pPlayerArray[EXTRACTCLIENTINDEX((*iter))];
						if (MODE_GAME != pTargetPlayer->_iSessionMode)
							continue;

						AttackPushPos(clientPosX, clientPosY, pTargetPlayer->_clientPosX, pTargetPlayer->_clientPosY, &fPushX, &fPushY);
						tileX = POS_to_TILE_X(fPushX);
						tileY = POS_to_TILE_Y(fPushY);

						if (!pField->CheckTileMove(tileX, tileY))
						{
							// ������ ��Ŷ�� �з��� ��ǥ 0�� ������ ���ڸ�
							fPushX = 0;
							fPushY = 0;
						}
						else
							pTargetPlayer->Move(tileX, tileY);

						// ���� ��� ���� �ð� ���ϱ�
						pTargetPlayer->_nextAttackTime = currTick + dfDAMAGE_STUN_TIME;
						pTargetPlayer->_nextTileTime = currTick + dfDAMAGE_STUN_TIME;

						pSendPacket = CPacket::Alloc();
						MakePacket_Damaged(pSendPacket, pTargetPlayer->_clientID, damageValue, fPushX, fPushY);
						pServer->SendPacket_SectorAround(pSendPacket, pTargetPlayer->_sectorX_curr, pTargetPlayer->_sectorY_curr, NULL);
						pSendPacket->Free();

						if (0 > (pTargetPlayer->_iHP -= damageValue))
						{
							pTargetPlayer->_iHP = 0;
							pTargetPlayer->_byDie = 1;
							pField->MoveTileObject(pTargetPlayer->_serverX_curr, pTargetPlayer->_serverY_curr, (-1), (-1), pTargetPlayer->_clientID);
							bDie = true;
						}

						if (bDie)
						{
							if (this->_targetID == pTargetPlayer->_clientID)
							{
								this->_targetID = -1;
								this->_targetPtr = NULL;
							}

							pSendPacket = CPacket::Alloc();
							MakePacket_Die(pSendPacket, pTargetPlayer->_clientID);
							pServer->SendPacket_SectorAround(pSendPacket, pTargetPlayer->_sectorX_curr, pTargetPlayer->_sectorY_curr, NULL);
							pSendPacket->Free();

							st_DBWRITER_MSG *pMsg = pServer->_databaseMsgPool.Alloc();
							pMsg->Type_DB = dfDBWRITER_TYPE_GAME;
							pMsg->Type_Message = enDB_GAME_WRITE_PLAYER_DIE;

							stDB_GAME_WRITE_PLAYER_DIE_in dieMessage;
							dieMessage.AccountNo = pTargetPlayer->_accountNo;
							dieMessage.AttackerAccountNo = this->_accountNo;
							dieMessage.DiePosX = pTargetPlayer->_serverX_curr;
							dieMessage.DiePosY = pTargetPlayer->_serverY_curr;

							memcpy_s(pMsg->Message, dfDBWRITER_MSG_MAX, &dieMessage, sizeof(stDB_GAME_WRITE_PLAYER_DIE_in));
							pServer->_databaseMsgQueue.Enqueue(pMsg);

							pMsg = pServer->_databaseMsgPool.Alloc();
							pMsg->Type_DB = dfDBWRITER_TYPE_GAME;
							pMsg->Type_Message = enDB_GAME_WRITE_PLAYER_KILL;

							stDB_GAME_WRITE_PLAYER_KILL_in killMessage;
							killMessage.AccountNo = this->_accountNo;
							killMessage.TargetAccountNo = pTargetPlayer->_accountNo;
							killMessage.KillPosX = this->_serverX_curr;
							killMessage.KillPosY = this->_serverY_curr;

							memcpy_s(pMsg->Message, dfDBWRITER_MSG_MAX, &killMessage, sizeof(stDB_GAME_WRITE_PLAYER_KILL_in));
							pServer->_databaseMsgQueue.Enqueue(pMsg);
						}
					}
				}
			}
		}
		else
		{
			// ���� �ۿ� �����Ƿ� �̵�. ������ ��밡 �������� �ʾҴٸ� �� ������ �н�
			if (this->_goal_X != pTargetPlayer->_serverX_curr || this->_goal_Y != pTargetPlayer->_serverY_curr)
			{
				pServer->_jps->FindPath(this->_serverX_curr, this->_serverY_curr, pTargetPlayer->_serverX_curr, pTargetPlayer->_serverY_curr, this->_path, &this->_pathCount);

				pSendPacket = CPacket::Alloc();
				MakePacket_ResMoveCharacter(pSendPacket, this->_clientID, this->_pathCount, this->_path, this->_clientPosX, this->_clientPosY);
				pServer->SendPacket_SectorAround(pSendPacket, this->_sectorX_curr, this->_sectorY_curr, NULL);
				pSendPacket->Free();

				this->_pPath = this->_path;
				this->_goal_X = pTargetPlayer->_serverX_curr;
				this->_goal_Y = pTargetPlayer->_serverY_curr;
			}
		}
	}
	else
	{
		// �߰��� Ÿ���� ������ ���� ��� �߻���
		MoveStop(true);
		this->_targetID = -1;
		this->_targetPtr = NULL;
	}


	return;
}

bool CGameServer::CPlayer::OnAuth_ClientJoin(void)
{
	st_DBWRITER_MSG *pMsg = this->_pGameServer->_databaseMsgPool.Alloc();
	pMsg->Type_DB = dfDBWRITER_TYPE_GAME;
	pMsg->Type_Message = enDB_GAME_READ_PLAYER_CHECK;
	stDB_GAME_READ_PLAYER_CHECK_in message;
	message.AccountNo = this->_accountNo;
	message.ConnectInfo = *this->_connectInfo;
	memcpy_s(pMsg->Message, dfDBWRITER_MSG_MAX, &message, sizeof(stDB_GAME_READ_PLAYER_CHECK_in));
	this->_pGameServer->_databaseMsgQueue.Enqueue(pMsg);

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
	int iTileX, iTileY;
	int iSectorX, iSectorY;

	// ���⼭ ĳ���� ������ �Ѵ�.
	if (dfDUMMY_ACCOUNTNO_LIMIT < this->_accountNo)
	{
		if (1 == this->_byParty)
		{
			iTileX = dfCREATE_PLAYER_X_PARTY1;
			iTileY = dfCREATE_PLAYER_Y_PARTY1;
			
		}
		else
		{
			iTileX = dfCREATE_PLAYER_X_PARTY2;
			iTileY = dfCREATE_PLAYER_Y_PARTY2;
		}
	}
	else
	{
		//iNextTileX = dfCREATE_PLAYER_X_DUMMY;
		//iNextTileY = dfCREATE_PLAYER_Y_DUMMY;
		while (true)
		{
			iTileX = ((rand() % 4) + 300);
			iTileY = ((rand() % 5) + 30);

			if (this->_pGameServer->_field->CheckTileMove(iTileX, iTileY))
				break;
		}
	}
	
	this->_serverX_prev = -1;
	this->_serverY_prev = -1;
	this->_serverX_curr = iTileX;
	this->_serverY_curr = iTileY;

	this->_pGameServer->_field->MoveTileObject((-1), (-1), iTileX, iTileY, this->_clientID);

	iSectorX = TILE_to_SECTOR_X(iTileX);
	iSectorY = TILE_to_SECTOR_Y(iTileY);

	this->_sectorX_prev = -1;
	this->_sectorY_prev = -1;
	this->_sectorX_curr = iSectorX;
	this->_sectorY_curr = iSectorY;
	
	this->_pGameServer->_sector->MoveSector((-1), (-1), iSectorX, iSectorY, this->_clientID, this);

	this->_clientPosX = TILE_to_POS_X(iTileX);
	this->_clientPosY = TILE_to_POS_Y(iTileY);

	this->_rotation = (en_DIRECTION)((rand() % eMOVE_MAX) + 1);
	
	this->_iHP = dfHP_MAX;
	this->_pPath = NULL;

	this->_byDie = 0;

	this->_targetID = -1;
	this->_targetPtr = NULL;

	this->_killCount = 0;
	this->_killCount_guest = 0;

	this->_byFirstAction = 0;

	SendPacket_NewCreateCharacter();
	
	st_DBWRITER_MSG *pMsg = this->_pGameServer->_databaseMsgPool.Alloc();
	pMsg->Type_DB = dfDBWRITER_TYPE_GAME;
	pMsg->Type_Message = enDB_GAME_WRITE_LOG_JOIN;
	stDB_GAME_WRITE_LOG_JOIN_in message;
	message.AccountNo = this->_accountNo;
	message.CharacterType = this->_characterType;
	message.TileX = this->_serverX_curr;
	message.TileY = this->_serverY_curr;
	memcpy_s(pMsg->Message, dfDBWRITER_MSG_MAX, &message, sizeof(stDB_GAME_WRITE_LOG_JOIN_in));
	this->_pGameServer->_databaseMsgQueue.Enqueue(pMsg);

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

				case en_PACKET_CS_GAME_REQ_ATTACK1_TARGET:
				{
					PacketProc_Attack1(pRecvPacket);
					break;
				}

				case en_PACKET_CS_GAME_REQ_ATTACK2_TARGET:
				{
					PacketProc_Attack2(pRecvPacket);
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
	// Ÿ�Ͽ��� ����
	if (0 ==this->_byDie)
		this->_pGameServer->_field->MoveTileObject(this->_serverX_curr, this->_serverY_curr, (-1), (-1), this->_clientID);

	// ���Ϳ��� ����
	this->_pGameServer->_sector->MoveSector(this->_sectorX_curr, this->_sectorY_curr, (-1), (-1), this->_clientID, this);

	// ������Ʈ ���� ��Ŷ ������
	SendPacket_RemoveObject_Disconnect();

	/////////////////////////////////////////////////////////////////
	// DB �α����
	st_DBWRITER_MSG *pMsg = this->_pGameServer->_databaseMsgPool.Alloc();
	pMsg->Type_DB = dfDBWRITER_TYPE_GAME;
	pMsg->Type_Message = enDB_GAME_WRITE_LOG_JOIN;

	stDB_GAME_WRITE_LOG_LEAVE_in leaveMessage;
	leaveMessage.AccountNo = this->_accountNo;
	leaveMessage.TileX = this->_serverX_curr;
	leaveMessage.TileY = this->_serverY_curr;
	leaveMessage.KillCount = this->_killCount;
	leaveMessage.GuestKillCount = this->_killCount_guest;

	memcpy_s(pMsg->Message, dfDBWRITER_MSG_MAX, &leaveMessage, sizeof(stDB_GAME_WRITE_LOG_JOIN_in));
	this->_pGameServer->_databaseMsgQueue.Enqueue(pMsg);

	/////////////////////////////////////////////////////////////////
	// DB�� ���ӻ��� �ʱ�ȭ
	pMsg = this->_pGameServer->_databaseMsgPool.Alloc();
	pMsg->Type_DB = dfDBWRITER_TYPE_ACCOUNT;
	pMsg->Type_Message = enDB_ACCOUNT_WRITE_STATUS_LOGOUT;

	stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in message;
	message.AccountNo = this->_accountNo;
	memcpy_s(pMsg->Message, dfDBWRITER_MSG_MAX, &message, sizeof(stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in));
	this->_pGameServer->_databaseMsgQueue.Enqueue(pMsg);

	return true;
}

bool CGameServer::CPlayer::OnGame_ClientRelease(void)
{
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

	this->_clientID = -1;
	this->_connectInfo = NULL;
	this->_recvQ.ClearBuffer();
	this->_iSessionMode = CSession::MODE_NONE;
	this->_bAuthToGame = false;
	this->_bLogout = false;
	this->_iSendCount = 0;

	return true;
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

void CGameServer::CPlayer::MoveStop(bool bSend)
{
//	int iNextTileX, iNextTileY;

	this->_pPath = NULL;
	this->_pathCount = 0;
	this->_path_curr = 0;

	//// Ŭ�� �� ĭ �� �տ� ������. �������� ��ĭ ����
	//MoveTile(this->_rotation, this->_serverX_curr, this->_serverY_curr, &iNextTileX, &iNextTileY);
	//if ((-1) != iNextTileX)
	//	Move(iNextTileX, iNextTileY);

	if (bSend)
		SendPacket_MoveStop();
	return;
}


/////////////////////////////////////////////////////////////////////
//
//	�Լ� : Move(int tileX, int tileY)
//	����
//		int tileX	: �̵��� Ÿ�� X��ǥ
//		int tileY	: �̵��� Ÿ�� Y��ǥ
//
//	�̵�ó���� ����Ѵ�. ��ǥ ������ �ϰ� Ÿ�� ������Ʈ �ű��, 
//	���� �ű��, ���͸� �Ű�ٸ� ��Ŷ ���۱��� ���� ���ش�
/////////////////////////////////////////////////////////////////////
void CGameServer::CPlayer::Move(int tileX, int tileY)
{
	if ((this->_serverX_prev != tileX) || (this->_serverY_prev != tileY))
	{
		this->_serverX_prev = this->_serverX_curr;
		this->_serverY_prev = this->_serverY_curr;
		this->_serverX_curr = tileX;
		this->_serverY_curr = tileY;

		this->_pGameServer->_field->MoveTileObject(this->_serverX_prev, this->_serverY_prev, tileX, tileY, this->_clientID);

		this->_sectorX_prev = this->_sectorX_curr;
		this->_sectorY_prev = this->_sectorY_curr;
		this->_sectorX_curr = TILE_to_SECTOR_X(tileX);
		this->_sectorY_curr = TILE_to_SECTOR_Y(tileY);

		this->_clientPosX = TILE_to_POS_X(tileX);
		this->_clientPosY = TILE_to_POS_Y(tileY);

		if ((this->_sectorX_prev != this->_sectorX_curr) || (this->_sectorY_prev != this->_sectorY_curr))
		{
			this->_pGameServer->_sector->MoveSector(this->_sectorX_prev, this->_sectorY_prev, this->_sectorX_curr, this->_sectorY_curr, this->_clientID, this);
			SendPacket_MoveSector();
		}
	}

	return;
}


void CGameServer::CPlayer::GetSectorPos(int *iSectorX, int *iSectorY)
{
	*iSectorX = this->_sectorX_curr;
	*iSectorY = this->_sectorY_curr;

	return;
}

/////////////////////////////////////////////////////////////////////////
// PacketProc
/////////////////////////////////////////////////////////////////////////
#pragma region packetproc_auth
void  CGameServer::CPlayer::PacketProc_Login(CPacket *pRecvPacket)
{
	__int64 iAccountNo;
	char SessionKey[64];

	*pRecvPacket >> iAccountNo;
	pRecvPacket->Dequeue(SessionKey, 64);

	// 1. ���� Ű �˻� �� ���ٸ� �α��� ���� �ٷ� ����
	// 2. DB���� accountNo�� �˻� �� �α��� ���� ���� ����
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

	// DB�˻�
	stDB_ACCOUNT_READ_USER_in in;
	stDB_ACCOUNT_READ_USER_out out;
	in.AccountNo = iAccountNo;

	this->_pGameServer->_database_Account->QueryDB(enDB_ACCOUNT_READ_USER, &in, &out);

	if (dfGAME_LOGIN_FAIL == out.Status || dfGAME_LOGIN_VERSION_MISS == out.Status)
	{
		MakePacket_ResLogin(pSendPacket, out.Status, iAccountNo);
		SendPacket(pSendPacket);
		pSendPacket->Free();

		// ��Ŷ �ϳ� ������ ���� ������ �ʿ��ϴ�.
	}
	else
	{
		MakePacket_ResLogin(pSendPacket, out.Status, iAccountNo);
		SendPacket(pSendPacket);
		pSendPacket->Free();

		// ���� ���� ����
		this->_accountNo = iAccountNo;
		wcscpy_s(this->_szNickname, dfNICK_MAX_LEN, out.szNick);
		this->_byParty = out.Party;
	}

	/*if (dfDUMMY_ACCOUNTNO_LIMIT >= iAccountNo)
	SetMode_Game();*/

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
#pragma endregion packetproc_auth

#pragma region packetproc_game
void CGameServer::CPlayer::PacketProc_MoveCharacter(CPacket *pRecvPacket)
{
	//
	//	ó�� ���ӽ� �� ��Ŷ�� ������ ������ �̵�ó���� �ϴµ� Ŭ�� �̵�ó���� ���Ѵ�.
	//	�ᱹ ��ũ ��Ŷ�� ������ �Ǵµ� ����� �����ΰ�?
	//
	
	__int64 clientID;
	float startX;
	float startY;
	float destX;
	float destY;
	int startTileX;
	int startTileY;
	int destTileX;
	int destTileY;

	if (1 == this->_byDie)
		return;

	*pRecvPacket >> clientID >> startX >> startY >> destX >> destY;

	startTileX = POS_to_TILE_X(startX);
	startTileY = POS_to_TILE_Y(startY);
	destTileX = POS_to_TILE_X(destX);
	destTileY = POS_to_TILE_Y(destY);

	if (this->CheckErrorRange(startX, startY))
		SendPacket_Sync();
	else
		Move(startTileX, startTileY);

	CPacket *pSendPacket = CPacket::Alloc();
	if (this->_pGameServer->_jps->FindPath(this->_serverX_curr, this->_serverY_curr, destTileX, destTileY, this->_path, &this->_pathCount))
	{
		MakePacket_ResMoveCharacter(pSendPacket, clientID, (BYTE)this->_pathCount, this->_path, this->_clientPosX, this->_clientPosY);
		this->_pPath = this->_path;
		this->_nextTileTime = GetTickCount64();
		this->_path_curr = 0;

		this->_goal_X = destTileX;
		this->_goal_Y = destTileY;
	}
	else
	{
		// ���� ã�� ���ϸ� ĳ���� ����
		MakePacket_StopCharacter(pSendPacket, clientID, this->_clientPosX, this->_clientPosY, 0xFFFF);
		MoveStop(false);
	}

	this->_pGameServer->SendPacket_SectorAround(pSendPacket, this->_sectorX_curr, this->_sectorY_curr, NULL);
	pSendPacket->Free();

	// �̵�, ���� ��Ŷ�� ���� ���� �ߴ�
	this->_targetID = -1;
	this->_targetPtr = NULL;

	if (0 == this->_byFirstAction)
		this->_byFirstAction = 1;

	return;
}

void CGameServer::CPlayer::PacketProc_StopCharacter(CPacket *pRecvPacket)
{
	__int64 clientID;
	float PosX;
	float PosY;
	WORD rotation;

	int iTileX, iTileY;

	*pRecvPacket >> clientID >> PosX >> PosY >> rotation;

	if (this->CheckErrorRange(PosX, PosY))
	{
		SendPacket_Sync();
	}
	else
	{
		iTileX = POS_to_TILE_X(PosX);
		iTileY = POS_to_TILE_Y(PosY);

		Move(iTileX, iTileY);
	}

	MoveStop(false);

	// �̵�, ���� ��Ŷ�� ���� ���� �ߴ�
	/*this->_targetID = -1;
	this->_targetPtr = NULL;*/
	return;
}

void CGameServer::CPlayer::PacketProc_Attack1(CPacket *pRecvPacket)
{
	CLIENT_ID AttackID;
	CLIENT_ID TargetID;
	//CPlayer *pAttackPlayer = NULL;
	CPlayer *pTargetPlayer = NULL;
	int index;

	*pRecvPacket >> AttackID >> TargetID;

	/*index = EXTRACTCLIENTINDEX(AttackID);
	pAttackPlayer = &this->_pGameServer->_pPlayerArray[index];
	if (AttackID != pAttackPlayer->_clientID)
	{
		SYSLOG(L"PACKET", LOG::LEVEL_ERROR, L"don`t match player");
		CCrashDump::Crash();
		return;
	}*/

	index = EXTRACTCLIENTINDEX(TargetID);
	pTargetPlayer = &this->_pGameServer->_pPlayerArray[index];
	if (TargetID != pTargetPlayer->_clientID)
	{
		SYSLOG(L"PACKET", LOG::LEVEL_ERROR, L"don`t match player");
		CCrashDump::Crash();
		return;
	}

	UINT64 tick = GetTickCount64();
	// pAttackPlayer�� this�� ������ ���� �� �ۿ� ���� �ʳ�?
	if ((this->_byParty != pTargetPlayer->_byParty) || (3 == this->_characterType))
	{
		this->_targetID = TargetID;
		this->_targetPtr = pTargetPlayer;
		this->_attackType = 1;

		if (tick >= this->_nextAttackTime)
			this->_nextAttackTime = tick;
	}

	if (0 == this->_byFirstAction)
		this->_byFirstAction = 1;

	return;
}

void CGameServer::CPlayer::PacketProc_Attack2(CPacket *pRecvPacket)
{
	CLIENT_ID AttackID;
	CLIENT_ID TargetID;
	//CPlayer *pAttackPlayer = NULL;
	CPlayer *pTargetPlayer = NULL;
	int index;

	*pRecvPacket >> AttackID >> TargetID;

	/*index = EXTRACTCLIENTINDEX(AttackID);
	pAttackPlayer = &this->_pGameServer->_pPlayerArray[index];
	if (AttackID != pAttackPlayer->_clientID)
	{
	SYSLOG(L"PACKET", LOG::LEVEL_ERROR, L"don`t match player");
	CCrashDump::Crash();
	return;
	}*/

	index = EXTRACTCLIENTINDEX(TargetID);
	pTargetPlayer = &this->_pGameServer->_pPlayerArray[index];
	if (TargetID != pTargetPlayer->_clientID)
	{
		SYSLOG(L"PACKET", LOG::LEVEL_ERROR, L"don`t match player");
		CCrashDump::Crash();
		return;
	}

	UINT64 tick = GetTickCount64();
	// pAttackPlayer�� this�� ������ ���� �� �ۿ� ���� �ʳ�?
	if ((this->_byParty != pTargetPlayer->_byParty) || (3 == this->_characterType))
	{
		this->_targetID = TargetID;
		this->_targetPtr = pTargetPlayer;
		this->_attackType = 2;

		if (tick >= this->_nextAttackTime)
			this->_nextAttackTime = tick;
	}

	if (0 == this->_byFirstAction)
		this->_byFirstAction = 1;

	return;
}
#pragma endregion packetproc_game

#pragma region packetproc_etc
void CGameServer::CPlayer::PacketProc_ClientHeartBeat(CPacket *pRecvPacket)
{
	this->_heartBeatTick = time(NULL);
	return;
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
#pragma endregion packetproc_etc

/////////////////////////////////////////////////////////////////////////
// MakePacket
/////////////////////////////////////////////////////////////////////////
#pragma region makepacket_auth
void CGameServer::CPlayer::MakePacket_ResLogin(CPacket *pSendPacket, BYTE byStatus, CLIENT_ID iAccountNo)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_LOGIN;
	*pSendPacket << byStatus;
	*pSendPacket << iAccountNo;

	return;
}

void CGameServer::CPlayer::MakePacket_ResCharacterSelect(CPacket *pSendPacket, BYTE byStatus)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_CHARACTER_SELECT;
	*pSendPacket << byStatus;
}
#pragma endregion makepacket_auth

#pragma region makepacket_game
void CGameServer::CPlayer::MakePacket_CreateCharacter(CPacket *pSendPacket, CLIENT_ID clientID, BYTE characterType, wchar_t *szNickname, float PosX, float PosY, WORD rotation, int hp, BYTE party)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_CREATE_MY_CHARACTER;
	*pSendPacket << clientID;
	*pSendPacket << characterType;
	pSendPacket->Enqueue((char *)szNickname, dfNICK_BYTE_LEN);
	*pSendPacket << PosX;
	*pSendPacket << PosY;
	*pSendPacket << rotation;
	*pSendPacket << (int)0;				// Cristal (��� ����)
	*pSendPacket << hp;
	*pSendPacket << (__int64)0;			// Exp (��� ����)
	*pSendPacket << (WORD)0;			// Level (��� ����)
	*pSendPacket << party;

	return;
}

void CGameServer::CPlayer::MakePacket_CreateOtherCharacter(CPacket *pSendPacket, CLIENT_ID clientID, BYTE characterType, wchar_t *szNickname, float PosX, float PosY, WORD rotation, BYTE respawn, BYTE die, int hp, BYTE party, BYTE firstAction)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_CREATE_OTHER_CHARACTER;
	*pSendPacket << clientID;
	*pSendPacket << characterType;
	pSendPacket->Enqueue((char *)szNickname, dfNICK_BYTE_LEN);
	*pSendPacket << PosX;
	*pSendPacket << PosY;
	*pSendPacket << rotation;
	*pSendPacket << (WORD)0;			// Level (��� ����)
	*pSendPacket << respawn;
	*pSendPacket << (BYTE)0;			// Sit (��� ����)
	*pSendPacket << die;
	*pSendPacket << hp;
	*pSendPacket << party;
	*pSendPacket << firstAction;

	return;
}

void CGameServer::CPlayer::MakePacket_RemoveObject(CPacket *pSendPacket, CLIENT_ID clientID)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_REMOVE_OBJECT;
	*pSendPacket << clientID;

	return;
}

void CGameServer::CPlayer::MakePacket_ResMoveCharacter(CPacket *pSendPacket, __int64 clientID, BYTE pathCount, PATH *pPath, float posX, float posY)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_MOVE_CHARACTER;
	*pSendPacket << clientID;
	*pSendPacket << pathCount;
	pSendPacket->Enqueue((char *)pPath, sizeof(PATH) * pathCount);
	*pSendPacket << posX;
	*pSendPacket << posY;

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

void CGameServer::CPlayer::MakePacket_Attack(CPacket *pSendPacket, BYTE attackType, CLIENT_ID attackerID, CLIENT_ID targetID, UINT64 coolTime, float attackPosX, float attackPosY)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_ATTACK;
	*pSendPacket << attackType;
	*pSendPacket << attackerID;
	*pSendPacket << targetID;
	*pSendPacket << coolTime;

	*pSendPacket << attackPosX;
	*pSendPacket << attackPosY;

	return;
}

void CGameServer::CPlayer::MakePacket_Damaged(CPacket *pSendPacket, CLIENT_ID damagedID, int value, float PushX, float PushY)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_DAMAGE;
	*pSendPacket << damagedID;
	*pSendPacket << value;
	*pSendPacket << PushX;
	*pSendPacket << PushY;

	return;
}

void CGameServer::CPlayer::MakePacket_Die(CPacket *pSendPacket, CLIENT_ID deadClientID)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_PLAYER_DIE;
	*pSendPacket << deadClientID;

	return;
}
#pragma endregion makepacket_game

#pragma region makepacket_etc
void CGameServer::CPlayer::MakePacket_ResEcho(CPacket *pSendPacket, __int64 iAccountNo, __int64 SendTick)
{
	*pSendPacket << (WORD)en_PACKET_CS_GAME_RES_ECHO;
	*pSendPacket << iAccountNo;
	*pSendPacket << SendTick;

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
#pragma endregion makepacket_etc

/////////////////////////////////////////////////////////////////////////
// SendPacket
/////////////////////////////////////////////////////////////////////////
#pragma region sendpacket
void CGameServer::CPlayer::SendPacket_NewCreateCharacter(void)
{
	// 1. �ش� �������� ĳ���� ������ ������ - �۵���
	CPacket *pSendPacket = CPacket::Alloc();
	MakePacket_CreateCharacter(pSendPacket, this->_clientID, this->_characterType, this->_szNickname, this->_clientPosX, this->_clientPosY, this->_rotation, this->_iHP, this->_byParty);
	SendPacket(pSendPacket);
	pSendPacket->Free();

	// 2. �ֺ� ����(3 X 3)����(���� �����ϰ�) �ش� ������ �������� �˸��� - �۵���
	int iSectorX = this->_sectorX_curr;
	int iSectorY = this->_sectorY_curr;

	pSendPacket = CPacket::Alloc();
	MakePacket_CreateOtherCharacter(pSendPacket, this->_clientID, this->_characterType, this->_szNickname, this->_clientPosX, this->_clientPosY, this->_rotation, 1, this->_byDie, this->_iHP, this->_byParty, this->_byFirstAction);
	this->_pGameServer->SendPacket_SectorAround(pSendPacket, iSectorX, iSectorY, this);
	pSendPacket->Free();

	// 3. �ش� �������� �ֺ� ���� ĳ������ ������ ������. - �۵���
	stAROUND_SECTOR around;
	this->_pGameServer->_sector->GetAroundSector(iSectorX, iSectorY, &around);
	sectorMap *map = NULL;

	for (int i = 0; i < around._iCount; ++i)
	{
		map = this->_pGameServer->_sector->GetList(around._around[i]._iSectorX, around._around[i]._iSectorY);

		if (NULL == map)
			CCrashDump::Crash();

		for (auto iter = map->begin(); iter != map->end(); ++iter)
		{
			CPlayer *pPlayer = iter->second;

			if (pPlayer == this)
				continue;

			pSendPacket = CPacket::Alloc();
			MakePacket_CreateOtherCharacter(pSendPacket, pPlayer->_clientID, pPlayer->_characterType, pPlayer->_szNickname, pPlayer->_clientPosX, pPlayer->_clientPosY, pPlayer->_rotation, 0, pPlayer->_byDie, pPlayer->_iHP, pPlayer->_byParty, pPlayer->_byFirstAction);
			SendPacket(pSendPacket);
			pSendPacket->Free();

			if (NULL != pPlayer->_pPath)
			{
				pSendPacket = CPacket::Alloc();
				MakePacket_ResMoveCharacter(pSendPacket, pPlayer->_clientID, (pPlayer->_pathCount - pPlayer->_path_curr), pPlayer->_pPath, pPlayer->_clientPosX, pPlayer->_clientPosY);
				SendPacket(pSendPacket);
				pSendPacket->Free();
			}
		}
	}
	return;
}

void CGameServer::CPlayer::SendPacket_RemoveObject_Disconnect(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	MakePacket_RemoveObject(pSendPacket, this->_clientID);
	this->_pGameServer->SendPacket_SectorAround(pSendPacket, this->_sectorX_curr, this->_sectorY_curr, this);	// ���Ϳ��� ���� ���� ������ �����ؾ��ϰ� �ݴ�� �ȳ־ �ǰ�
	pSendPacket->Free();

	return;
}

void CGameServer::CPlayer::SendPacket_MoveSector(void)
{
	stAROUND_SECTOR addSector, removeSector;
	CPacket *pSendPacket = NULL;
	sectorMap *map = NULL;
	CPlayer *pPlayer = NULL;
	int iCnt = 0;

	this->_pGameServer->_sector->GetAroundSector(this->_sectorX_prev, this->_sectorY_prev, &removeSector);
	this->_pGameServer->_sector->GetAroundSector(this->_sectorX_curr, this->_sectorY_curr, &addSector);
	this->_pGameServer->_sector->GetUpdateSector(&removeSector, &addSector);

	// 1. removeSector - �ش� ���� ĳ���� ������Ʈ ���� ��Ŷ ����. �ֺ��� ����
	pSendPacket = CPacket::Alloc();
	MakePacket_RemoveObject(pSendPacket, this->_clientID);
	for (iCnt = 0; iCnt < removeSector._iCount; ++iCnt)
		this->_pGameServer->SendPacket_SectorOne(pSendPacket, removeSector._around[iCnt]._iSectorX, removeSector._around[iCnt]._iSectorY, NULL);
	pSendPacket->Free();

	// 2. removeSector - ���� ������ ���� ĳ���� ������Ʈ ���� ��Ŷ ����. ������ ����
	for (iCnt = 0; iCnt < removeSector._iCount; ++iCnt)
	{
		map = this->_pGameServer->_sector->GetList(removeSector._around[iCnt]._iSectorX, removeSector._around[iCnt]._iSectorY);

		if (NULL == map)
			CCrashDump::Crash();

		for (auto iter = map->begin(); iter != map->end(); ++iter)
		{
			pPlayer = iter->second;

			if (pPlayer == this)
				continue;

			pSendPacket = CPacket::Alloc();
			MakePacket_RemoveObject(pSendPacket, pPlayer->_clientID);
			SendPacket(pSendPacket);
			pSendPacket->Free();
		}
	}

	// 3. addSector - �ش� ���� ĳ���� ������Ʈ ���� ��Ŷ ����. �ֺ��� ����
	pSendPacket = CPacket::Alloc();
	MakePacket_CreateOtherCharacter(pSendPacket, this->_clientID, this->_characterType, this->_szNickname, this->_clientPosX, this->_clientPosY, this->_rotation, 0, this->_byDie, this->_iHP, this->_byParty, this->_byFirstAction);
	for (iCnt = 0; iCnt < addSector._iCount; ++iCnt)
		this->_pGameServer->SendPacket_SectorOne(pSendPacket, addSector._around[iCnt]._iSectorX, addSector._around[iCnt]._iSectorY, NULL);
	pSendPacket->Free();

	// 4. addSector - �ش� ������ �̵� ���̿��ٸ� ĳ���� �̵� ��Ŷ ����. �ֺ��� ����
	if (NULL != this->_pPath)
	{
		pSendPacket = CPacket::Alloc();
		MakePacket_ResMoveCharacter(pSendPacket, this->_clientID, (this->_pathCount - this->_path_curr), this->_pPath, this->_clientPosX, this->_clientPosY);
		for (iCnt = 0; iCnt < addSector._iCount; ++iCnt)
			this->_pGameServer->SendPacket_SectorOne(pSendPacket, addSector._around[iCnt]._iSectorX, addSector._around[iCnt]._iSectorY, NULL);
		pSendPacket->Free();
	}
	
	// 5. addSector - �ű� ������ ĳ���� ������ �ش� �������� �� ��Ŷ ����. ������ ����
	for (iCnt = 0; iCnt < addSector._iCount; ++iCnt)
	{
		map = this->_pGameServer->_sector->GetList(addSector._around[iCnt]._iSectorX, addSector._around[iCnt]._iSectorY);

		if (NULL == map)
			CCrashDump::Crash();

		for (auto iter = map->begin(); iter != map->end(); ++iter)
		{
			CPlayer *pPlayer = iter->second;

			if (pPlayer == this)
				continue;

			pSendPacket = CPacket::Alloc();
			MakePacket_CreateOtherCharacter(pSendPacket, pPlayer->_clientID, pPlayer->_characterType, pPlayer->_szNickname, pPlayer->_clientPosX, pPlayer->_clientPosY, pPlayer->_rotation, 0, pPlayer->_byDie, pPlayer->_iHP, pPlayer->_byParty, pPlayer->_byFirstAction);
			SendPacket(pSendPacket);
			pSendPacket->Free();

			// 6. addSector - �ű� ������ ĳ���Ͱ� �̵����̶�� �̵� ��Ŷ ����. ������ ����
			if (NULL != pPlayer->_pPath)
			{
				pSendPacket = CPacket::Alloc();
				MakePacket_ResMoveCharacter(pSendPacket, pPlayer->_clientID, (pPlayer->_pathCount - pPlayer->_path_curr), pPlayer->_pPath, pPlayer->_clientPosX, pPlayer->_clientPosY);
				SendPacket(pSendPacket);
				pSendPacket->Free();
			}
		}
	}
}

void CGameServer::CPlayer::SendPacket_MoveStop()
{
	CPacket *pSendPacket = CPacket::Alloc();
	MakePacket_StopCharacter(pSendPacket, this->_clientID, this->_clientPosX, this->_clientPosY, 0xffff);
	this->_pGameServer->SendPacket_SectorAround(pSendPacket, this->_sectorX_curr, this->_sectorY_curr, NULL);
	//SendPacket(pSendPacket);
	pSendPacket->Free();

	return;
}

void CGameServer::CPlayer::SendPacket_Sync(void)
{
	CPacket *pSendPacket = CPacket::Alloc();
	MakePacket_Sync(pSendPacket, this->_clientID, this->_serverX_curr, this->_serverY_curr);
	
	//SendPacket(pSendPacket);
	this->_pGameServer->SendPacket_SectorAround(pSendPacket, this->_sectorX_curr, this->_sectorY_curr, NULL);
	pSendPacket->Free();

	return;
}


#pragma endregion sendpacket