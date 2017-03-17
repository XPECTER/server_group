#ifndef __GAMESERVER__DEFINES__
#define __GAMESERVER__DEFINES__


//---------------------------------------------
// 시스템 부분
//---------------------------------------------
//---------------------------------------------
// IOCP 워커스레드 최대개수
//---------------------------------------------
#define dfWORKER_THREAD_MAX		10

//---------------------------------------------
// DB 저장스레드 Queue 최대
//---------------------------------------------
#define dfDBWRITER_QUEUE_MAX	500

//---------------------------------------------
// 패킷 코드, 암호화 코드
//---------------------------------------------
#define dfPACKET_CODE			0x77
#define dfPACKET_KEY1			0x32
#define dfPACKET_KEY2			0x84

//---------------------------------------------
// 로그인서버로 보낼 하트비트 간격
//---------------------------------------------
#define dfLOGIN_SERVER_HEARTBEAT_TICK	1000

//---------------------------------------------
// 세션키 정보 타임아웃
//---------------------------------------------
#define dfLOGIN_SESSION_KEY_TIMEOUT		20000




//---------------------------------------------
// 더미계정 accountno 범위.
// 
// 아래 수치 이하의 accountno 는 더미계정
// 클라이언트가 들고 온 무작위 세션키를 그냥 인정해주겠음!
//---------------------------------------------
#define dfDUMMY_ACCOUNTNO_LIMIT	999999

//---------------------------------------------
// 게스트 계정 판단용, 아래 이상의 AccountNo 는 게스트 계정.
//
// 게스트 계정은 엘프 선택 불가
//---------------------------------------------
#define dfGUEST_ACCOUNTNO_OVER	2000000




//--------------------------------------------
// 컨텐츠 부분
//--------------------------------------------
//---------------------------------------------
// ID, Nick WCHAR 배열 길이.  (20글자)
//---------------------------------------------
#define dfNICK_MAX_LEN			21
#define dfID_MAX_LEN			21

//---------------------------------------------
// ID, Nick, SessionKey  패킷상 byte 길이
//---------------------------------------------
#define dfNICK_BYTE_LEN			40
#define dfID_BYTE_LEN			40
#define dfSESSION_KEY_BYTE_LEN	64


/*
//---------------------------------------------
// 몬스터 전용 / 미사용
//
// 방향 define  실제로는 0 ~ 360 의 각도를 가지지만
// 컨텐츠 (공격등, 몬스터 이동) 처리시 이를 8방향으로 나누어서 사용한다.
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
// 몬스터 리스폰 정보 최대치.
//---------------------------------------------
#define dfMONSTER_RESPAWN_MAX	20
*/

//---------------------------------------------
// 오브젝트 타입
//---------------------------------------------
#define dfOBJECT_TYPE_PLAYER		1
/*
#define dfOBJECT_TYPE_MONSTER		2
#define dfOBJECT_TYPE_CRISTAL		3
#define dfOBJECT_TYPE_BIGMONSTER	4
*/




//---------------------------------------------
// 타일 맵 오브젝트 박히는 최대 개수.
//
// 한 타일에 이 수치 이상의 객체가 쌓이면 컨텐츠 처리에 무시 될 수 있음.
//---------------------------------------------
#define dfMAP_TILE_OBJECT_MAX	20

//---------------------------------------------
// 타일 맵 최대 크기
//
//---------------------------------------------
#define dfMAP_TILE_X_MAX		600
#define dfMAP_TILE_Y_MAX		200

//---------------------------------------------
// Map.txt 에 있는 맵 속성 사이즈.
//
// 실제 게임서버는 이에 X 2 한 사이즈를 사용.  dfMAP_TILE_X_MAX / dfMAP_TILE_Y_MAX
//---------------------------------------------
#define dfMAP_OBSTACLE_FILE_X_MAX	200
#define dfMAP_OBSTACLE_FILE_Y_MAX	100

//---------------------------------------------
// 길찾기 최대 Point 개수.
//---------------------------------------------
#define dfPATH_POINT_MAX			30


//---------------------------------------------
// JumpPointSearch 길찾기 Open 탐색 최대횟수.
//---------------------------------------------
#define dfPATH_OPEN_MAX				30


//---------------------------------------------
// 클라이언트와, 서버의 좌표 동기화시 (이동시작,이동정지) 오차 허용 범위 / 클라좌표 기준.
//
// 이동시작,정지 패킷 수신시 클라가 보내준 좌표와, 서버의 클라좌표가 
// 아래 범위 내라면 클라가 보내준 좌표를 믿고 저장한다. 이를 넘어서면 Sync 패킷을 보내서 맞춤.
//---------------------------------------------
#define dfPOSITON_ERROR_RANGE_X	2.0f
#define dfPOSITON_ERROR_RANGE_Y	2.0f


//---------------------------------------------
// 맵 섹터 최대 개수
//---------------------------------------------
#define dfSECTOR_X_MAX		60
#define dfSECTOR_Y_MAX		60

//---------------------------------------------
// 섹터의 타일크기
//
//---------------------------------------------
#define dfSECTOR_TILE_WIDTH		25
#define dfSECTOR_TILE_HEIGHT	25


//---------------------------------------------
// 공격위치 알림 패킷 전송 간격 ms (UnderAttack Packet)
//
//---------------------------------------------
#define dfUNDER_ATTACK_SEND_TIME		7000



//---------------------------------------------
// KillRank 개수
//
// 현재 접속한 유저들의 킬 수 순위 (유저 누적이 아니고 접속 후 킬 수)
//---------------------------------------------
#define dfRANK_KILL_MAX					5


//---------------------------------------------
// 클라이언트 좌표를 서버 타일 좌표로 변경
//
// TILE to Pos 는 클라좌표로 변경 후 랜덤하게 소수점을 추가한다.
//---------------------------------------------
#define INVERT_Y(TileV) (abs((TileV) - dfMAP_TILE_Y_MAX))

#define POS_to_TILE_X(Pos) ((int)((Pos) * 2.0f))
#define POS_to_TILE_Y(Pos) (INVERT_Y((int)((Pos) * 2.0f)))

//#define TILE_to_POS_X(Tile) ((float)((Tile) / 2.0f) + (((rand() % 4) + 1) / 10.0f))
//#define TILE_to_POS_Y(Tile) ((float)(INVERT_Y(Tile) / 2.0f) + (((rand() % 4) + 1) / 10.0f))
#define TILE_to_POS_X(Tile) ((float)((Tile) / 2.0f) + 0.25f)
#define TILE_to_POS_Y(Tile) ((float)(INVERT_Y(Tile) / 2.0f) + 0.25f)


//---------------------------------------------
// 타일 좌표를 섹터 좌표로 변경
//
//---------------------------------------------
#define TILE_to_SECTOR_X(Tile) (Tile / dfSECTOR_TILE_WIDTH)
#define TILE_to_SECTOR_Y(Tile) (Tile / dfSECTOR_TILE_HEIGHT)


//---------------------------------------------
// 최초 캐릭터 생성 위치
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
// AccountNo 가 dfDUMMY_ACCOUNTNO_LIMIT 이하인 유저.
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
// 공격용 이동시, 공격 가능여부 확인 텀.
//
// 공격불가로 이동시에만 아래의 시간을 사용하며, 공격 후에는 g_Pattern_AttackTime 의 값 간격으로 공격을 한다.
//---------------------------------------------
#define dfATTACK_CHECK_MOVE	300


//---------------------------------------------
// 데미지를 먹은 경우 아래 시간동안은 움직임,공격 모두 중단.
//---------------------------------------------
#define dfDAMAGE_STUN_TIME		500

//---------------------------------------------
// 공격시 이동중단 시간.  클라이언트의 공격 액션 대기용..
//---------------------------------------------
#define dfATTACK_STOP_TIME		500





//---------------------------------------------
// 캐릭터 이동 방향 enum
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
// 현 위치에서 이동 위치로 방향을 구해주는 매크로
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
// 타음타일 이동 시간.
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
// 방향별로 실제 좌표 이동.
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