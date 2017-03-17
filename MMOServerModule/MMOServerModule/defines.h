#ifndef __GAMESERVER__DEFINES__
#define __GAMESERVER__DEFINES__


//---------------------------------------------
// �ý��� �κ�
//---------------------------------------------
//---------------------------------------------
// IOCP ��Ŀ������ �ִ밳��
//---------------------------------------------
#define dfWORKER_THREAD_MAX		10

//---------------------------------------------
// DB ���彺���� Queue �ִ�
//---------------------------------------------
#define dfDBWRITER_QUEUE_MAX	500

//---------------------------------------------
// ��Ŷ �ڵ�, ��ȣȭ �ڵ�
//---------------------------------------------
#define dfPACKET_CODE			0x77
#define dfPACKET_KEY1			0x32
#define dfPACKET_KEY2			0x84

//---------------------------------------------
// �α��μ����� ���� ��Ʈ��Ʈ ����
//---------------------------------------------
#define dfLOGIN_SERVER_HEARTBEAT_TICK	1000

//---------------------------------------------
// ����Ű ���� Ÿ�Ӿƿ�
//---------------------------------------------
#define dfLOGIN_SESSION_KEY_TIMEOUT		20000




//---------------------------------------------
// ���̰��� accountno ����.
// 
// �Ʒ� ��ġ ������ accountno �� ���̰���
// Ŭ���̾�Ʈ�� ��� �� ������ ����Ű�� �׳� �������ְ���!
//---------------------------------------------
#define dfDUMMY_ACCOUNTNO_LIMIT	999999

//---------------------------------------------
// �Խ�Ʈ ���� �Ǵܿ�, �Ʒ� �̻��� AccountNo �� �Խ�Ʈ ����.
//
// �Խ�Ʈ ������ ���� ���� �Ұ�
//---------------------------------------------
#define dfGUEST_ACCOUNTNO_OVER	2000000




//--------------------------------------------
// ������ �κ�
//--------------------------------------------
//---------------------------------------------
// ID, Nick WCHAR �迭 ����.  (20����)
//---------------------------------------------
#define dfNICK_MAX_LEN			21
#define dfID_MAX_LEN			21

//---------------------------------------------
// ID, Nick, SessionKey  ��Ŷ�� byte ����
//---------------------------------------------
#define dfNICK_BYTE_LEN			40
#define dfID_BYTE_LEN			40
#define dfSESSION_KEY_BYTE_LEN	64


/*
//---------------------------------------------
// ���� ���� / �̻��
//
// ���� define  �����δ� 0 ~ 360 �� ������ ��������
// ������ (���ݵ�, ���� �̵�) ó���� �̸� 8�������� ����� ����Ѵ�.
//---------------------------------------------
#define dfDIRECTION_UU		0
#define dfDIRECTION_RU		1
#define dfDIRECTION_RR		2
#define dfDIRECTION_RD		3
#define dfDIRECTION_DD		4
#define dfDIRECTION_LD		5
#define dfDIRECTION_LL		6
#define dfDIRECTION_LU		7

#define dfROTATION_UU		0
#define dfROTATION_RU		45
#define dfROTATION_RR		90
#define dfROTATION_RD		135
#define dfROTATION_DD		180
#define dfROTATION_LD		225
#define dfROTATION_LL		270
#define dfROTATION_LU		315


//---------------------------------------------
// ���� ������ ���� �ִ�ġ.
//---------------------------------------------
#define dfMONSTER_RESPAWN_MAX	20
*/

//---------------------------------------------
// ������Ʈ Ÿ��
//---------------------------------------------
#define dfOBJECT_TYPE_PLAYER		1
/*
#define dfOBJECT_TYPE_MONSTER		2
#define dfOBJECT_TYPE_CRISTAL		3
#define dfOBJECT_TYPE_BIGMONSTER	4
*/




//---------------------------------------------
// Ÿ�� �� ������Ʈ ������ �ִ� ����.
//
// �� Ÿ�Ͽ� �� ��ġ �̻��� ��ü�� ���̸� ������ ó���� ���� �� �� ����.
//---------------------------------------------
#define dfMAP_TILE_OBJECT_MAX	20

//---------------------------------------------
// Ÿ�� �� �ִ� ũ��
//
//---------------------------------------------
#define dfMAP_TILE_X_MAX		400
#define dfMAP_TILE_Y_MAX		200

//---------------------------------------------
// Map.txt �� �ִ� �� �Ӽ� ������.
//
// ���� ���Ӽ����� �̿� X 2 �� ����� ���.  dfMAP_TILE_X_MAX / dfMAP_TILE_Y_MAX
//---------------------------------------------
#define dfMAP_OBSTACLE_FILE_X_MAX	200
#define dfMAP_OBSTACLE_FILE_Y_MAX	100

//---------------------------------------------
// ��ã�� �ִ� Point ����.
//---------------------------------------------
#define dfPATH_POINT_MAX			30


//---------------------------------------------
// JumpPointSearch ��ã�� Open Ž�� �ִ�Ƚ��.
//---------------------------------------------
#define dfPATH_OPEN_MAX				30


//---------------------------------------------
// Ŭ���̾�Ʈ��, ������ ��ǥ ����ȭ�� (�̵�����,�̵�����) ���� ��� ���� / Ŭ����ǥ ����.
//
// �̵�����,���� ��Ŷ ���Ž� Ŭ�� ������ ��ǥ��, ������ Ŭ����ǥ�� 
// �Ʒ� ���� ����� Ŭ�� ������ ��ǥ�� �ϰ� �����Ѵ�. �̸� �Ѿ�� Sync ��Ŷ�� ������ ����.
//---------------------------------------------
#define dfPOSITON_ERROR_RANGE_X	2.0f
#define dfPOSITON_ERROR_RANGE_Y	2.0f


//---------------------------------------------
// �� ���� �ִ� ����
//---------------------------------------------
#define dfSECTOR_X_MAX		60
#define dfSECTOR_Y_MAX		60

//---------------------------------------------
// ������ Ÿ��ũ��
//
//---------------------------------------------
#define dfSECTOR_TILE_WIDTH		25
#define dfSECTOR_TILE_HEIGHT	25


//---------------------------------------------
// ������ġ �˸� ��Ŷ ���� ���� ms (UnderAttack Packet)
//
//---------------------------------------------
#define dfUNDER_ATTACK_SEND_TIME		7000



//---------------------------------------------
// KillRank ����
//
// ���� ������ �������� ų �� ���� (���� ������ �ƴϰ� ���� �� ų ��)
//---------------------------------------------
#define dfRANK_KILL_MAX					5


//---------------------------------------------
// Ŭ���̾�Ʈ ��ǥ�� ���� Ÿ�� ��ǥ�� ����
//
// TILE to Pos �� Ŭ����ǥ�� ���� �� �����ϰ� �Ҽ����� �߰��Ѵ�.
//---------------------------------------------
#define INVERT_Y(TileV) (abs((TileV) - dfMAP_TILE_Y_MAX))

#define POS_to_TILE_X(Pos) ((int)((Pos) * 2.0f))
#define POS_to_TILE_Y(Pos) (INVERT_Y((int)((Pos) * 2.0f)))

//#define TILE_to_POS_X(Tile) ((float)((Tile) / 2.0f) + (((rand() % 4) + 1) / 10.0f))
//#define TILE_to_POS_Y(Tile) ((float)(INVERT_Y(Tile) / 2.0f) + (((rand() % 4) + 1) / 10.0f))
#define TILE_to_POS_X(Tile) ((float)((Tile) / 2.0f) + 0.25f)
#define TILE_to_POS_Y(Tile) ((float)(INVERT_Y(Tile) / 2.0f) + 0.25f)


//---------------------------------------------
// Ÿ�� ��ǥ�� ���� ��ǥ�� ����
//
//---------------------------------------------
#define TILE_to_SECTOR_X(Tile) (Tile / dfSECTOR_TILE_WIDTH)
#define TILE_to_SECTOR_Y(Tile) (Tile / dfSECTOR_TILE_HEIGHT)


//---------------------------------------------
// ���� ĳ���� ���� ��ġ
//
// Party 1		( X : 68 ~ 138 / Y : 156 ~ 184 )
//
//	left		34.0f		68
//	top			22.0f		44	-> 156
//	right		69.0f		138
//	bottom		8.0f		16	-> 184
//
// 
//
// Party 2		( X : 68 ~ 138 / Y : 12 ~ 26 )
//
//	left		34.0f		68
//	top			94.0f		188 -> 12
//	right		75.0f		150
//	bottom		87.0f		174 -> 26
//
//
// Dummy		( X : 221 ~ 380 / Y : 20 ~ 184 )
//
// AccountNo �� dfDUMMY_ACCOUNTNO_LIMIT ������ ����.
//
//	left		110.0f		220
//	top			90.0f		180 -> 20
//	right		190.0f		380
//	bottom		10.0f		20	-> 190
//
//---------------------------------------------
#define dfCREATE_PLAYER_X		(85 + (rand() % 21))
#define dfCREATE_PLAYER_Y		(93 + (rand() % 31))

#define dfCREATE_PLAYER_X_PARTY1		(68 + (rand() % 70))
#define dfCREATE_PLAYER_Y_PARTY1		(156 + (rand() % 29))

#define dfCREATE_PLAYER_X_PARTY2		(68 + (rand() % 70))
#define dfCREATE_PLAYER_Y_PARTY2		(12 + (rand() % 15))

#define dfCREATE_PLAYER_X_DUMMY			(221 + (rand() % 160))
#define dfCREATE_PLAYER_Y_DUMMY			(20 + (rand() % 165))




#define dfHP_MAX				5000

//---------------------------------------------
// ���ݿ� �̵���, ���� ���ɿ��� Ȯ�� ��.
//
// ���ݺҰ��� �̵��ÿ��� �Ʒ��� �ð��� ����ϸ�, ���� �Ŀ��� g_Pattern_AttackTime �� �� �������� ������ �Ѵ�.
//---------------------------------------------
#define dfATTACK_CHECK_MOVE	300


//---------------------------------------------
// �������� ���� ��� �Ʒ� �ð������� ������,���� ��� �ߴ�.
//---------------------------------------------
#define dfDAMAGE_STUN_TIME		500

//---------------------------------------------
// ���ݽ� �̵��ߴ� �ð�.  Ŭ���̾�Ʈ�� ���� �׼� ����..
//---------------------------------------------
#define dfATTACK_STOP_TIME		500





//---------------------------------------------
// ĳ���� �̵� ���� enum
//---------------------------------------------
enum en_DIRECTION
{
	eMOVE_NN = 0,
	eMOVE_UU,
	eMOVE_RU,
	eMOVE_RR,
	eMOVE_RD,
	eMOVE_DD,
	eMOVE_LD,
	eMOVE_LL,
	eMOVE_LU
};

//---------------------------------------------
// �� ��ġ���� �̵� ��ġ�� ������ �����ִ� ��ũ��
//---------------------------------------------
inline en_DIRECTION MoveDirection(int iSX, int iSY, int iDX, int iDY)
{
	if ( iSX == iDX )
	{
		if ( iSY == iDY )
			return eMOVE_NN;
		else if ( iSY > iDY )
			return eMOVE_UU;
		else if ( iSY < iDY )
			return eMOVE_DD;
	}
	else if ( iSX > iDX )
	{
		if ( iSY == iDY )
			return eMOVE_LL;
		else if ( iSY > iDY )
			return eMOVE_LU;
		else if ( iSY < iDY )
			return eMOVE_LD;
	}
	else if ( iSX < iDX )
	{
		if ( iSY == iDY )
			return eMOVE_RR;
		else if ( iSY > iDY )
			return eMOVE_RU;
		else if ( iSY < iDY )
			return eMOVE_RD;
	}
	return eMOVE_NN;
}


//---------------------------------------------
// Ÿ��Ÿ�� �̵� �ð�.
//---------------------------------------------
inline ULONGLONG NextTileTime(en_DIRECTION Dir)
{
	int iNextTime;
	switch ( Dir )
	{
	case eMOVE_LL:
	case eMOVE_RR:
	case eMOVE_UU:
	case eMOVE_DD:
		iNextTime = 130;	// 130
		break;
	case eMOVE_RU:
	case eMOVE_LU:
	case eMOVE_LD:
	case eMOVE_RD:
		iNextTime = 210;
		break;
	}

	return time(NULL) + iNextTime;
}


//---------------------------------------------
// ���⺰�� ���� ��ǥ �̵�.
//---------------------------------------------
inline void MoveTile(en_DIRECTION Dir, int iTileX, int iTileY, int *pMoveX, int *pMoveY)
{
	switch ( Dir )
	{
	case eMOVE_LL:
		*pMoveX = iTileX - 1;
		*pMoveY = iTileY;
		break;
	case eMOVE_RR:
		*pMoveX = iTileX + 1;
		*pMoveY = iTileY;
		break;
	case eMOVE_UU:
		*pMoveX = iTileX;
		*pMoveY = iTileY - 1;
		break;
	case eMOVE_DD:
		*pMoveX = iTileX;
		*pMoveY = iTileY + 1;
		break;
	case eMOVE_RU:
		*pMoveX = iTileX + 1;
		*pMoveY = iTileY - 1;
		break;
	case eMOVE_LU:
		*pMoveX = iTileX - 1;
		*pMoveY = iTileY - 1;
		break;
	case eMOVE_LD:
		*pMoveX = iTileX - 1;
		*pMoveY = iTileY + 1;
		break;
	case eMOVE_RD:
		*pMoveX = iTileX + 1;
		*pMoveY = iTileY + 1;
		break;
	}

	if ( *pMoveX < 0 || *pMoveY < 0 || *pMoveX >= dfMAP_TILE_X_MAX || *pMoveY >= dfMAP_TILE_Y_MAX )
	{
		*pMoveX = -1;
		*pMoveY = -1;
	}
}




#endif