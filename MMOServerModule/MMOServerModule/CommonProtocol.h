//#ifndef __GODDAMNBUG_ONLINE_PROTOCOL__
//#define __GODDAMNBUG_ONLINE_PROTOCOL__

/*
	- 2017.02.25

	# en_PACKET_CS_GAME_RES_ATTACK

	추가 - int		CoolTime			// 다음 공격 가능 대기 시간. 

	# en_PACKET_CS_GAME_RES_CREATE_MY_CHARACTER,

	추가 -  BYTE	Party				// 1 또는 2  캐릭터의 파티정보

	# en_PACKET_CS_GAME_RES_CREATE_OTHER_CHARACTER,

	추가 -  BYTE	Party				// 1 또는 2  캐릭터의 파티정보

	# en_PACKET_CS_GAME_RES_HEAL

	미사용으로 전환. 별도의 힐링 패킷없이 데미지로 통일. (음수로 데미지를 먹이면 힐링됨)




	- 2017.02.24 

	# en_PACKET_CS_GAME_RES_CREATE_OTHER_CHARACTER,

	추가 - 	int	HP	

	//------------------------------------------------------------


*/




enum en_PACKET_TYPE
{
	////////////////////////////////////////////////////////
	//
	//	Client & Server Protocol
	//
	////////////////////////////////////////////////////////

	//------------------------------------------------------
	// Chatting Server
	//------------------------------------------------------
	en_PACKET_CS_CHAT_SERVER			= 0,

	//------------------------------------------------------------
	// 채팅서버 로그인 요청
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WCHAR	ID[20]				// null 포함
	//		WCHAR	Nickname[20]		// null 포함
	//		char	SessionKey[64];
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_CHAT_REQ_LOGIN,

	//------------------------------------------------------------
	// 채팅서버 로그인 응답
	//
	//	{
	//		WORD	Type
	//
	//		BYTE	Status				// 0:실패	1:성공
	//		INT64	AccountNo
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_CHAT_RES_LOGIN,

	//------------------------------------------------------------
	// 채팅서버 섹터 이동 요청
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WORD	SectorX
	//		WORD	SectorY
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_CHAT_REQ_SECTOR_MOVE,

	//------------------------------------------------------------
	// 채팅서버 섹터 이동 결과
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WORD	SectorX
	//		WORD	SectorY
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_CHAT_RES_SECTOR_MOVE,

	//------------------------------------------------------------
	// 채팅서버 채팅보내기 요청
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WORD	MessageLen
	//		WCHAR	Message[MessageLen / 2]		// null 미포함
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_CHAT_REQ_MESSAGE,

	//------------------------------------------------------------
	// 채팅서버 채팅보내기 응답  (다른 클라가 보낸 채팅도 이걸로 받음)
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		WCHAR	ID[20]						// null 포함
	//		WCHAR	Nickname[20]				// null 포함
	//		
	//		WORD	MessageLen
	//		WCHAR	Message[MessageLen / 2]		// null 미포함
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_CHAT_RES_MESSAGE,

	//------------------------------------------------------------
	// 하트비트
	//
	//	{
	//		WORD		Type
	//	}
	//
	//
	// 클라이언트는 이를 30초마다 보내줌.
	// 서버는 40초 이상동안 메시지 수신이 없는 클라이언트를 강제로 끊어줘야 함.
	//------------------------------------------------------------	
	en_PACKET_CS_CHAT_REQ_HEARTBEAT,






	//------------------------------------------------------
	// Login Server
	//------------------------------------------------------
	en_PACKET_CS_LOGIN_SERVER				= 100,

	//------------------------------------------------------------
	// 로그인 서버로 클라이언트 로그인 요청
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		char	SessionKey[64]
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_LOGIN_REQ_LOGIN,

	//------------------------------------------------------------
	// 로그인 서버에서 클라이언트로 로그인 응답
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		BYTE	Status				// 0 (세션오류) / 1 (성공) ...  하단 defines 사용
	//
	//		WCHAR	ID[20]				// 사용자 ID		. null 포함
	//		WCHAR	Nickname[20]		// 사용자 닉네임	. null 포함
	//
	//		WCHAR	GameServerIP[16]	// 접속대상 게임,채팅 서버 정보
	//		USHORT	GameServerPort
	//		WCHAR	ChatServerIP[16]
	//		USHORT	ChatServerPort
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_LOGIN_RES_LOGIN,








	//------------------------------------------------------
	// Game Server
	//------------------------------------------------------
	en_PACKET_CS_GAME_SERVER				= 1000,

	//------------------------------------------------------------
	// 로그인 요청
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		char	SessionKey[64]
	//
	//		int		Version			// Major 100 + Minor 10  = 1.10
	//								// 현재 최신 버전은		0.01 (1) - 2016.03.28
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_GAME_REQ_LOGIN,

	//------------------------------------------------------------
	// 로그인 응답
	//
	//	{
	//		WORD	Type
	//
	//		BYTE	Status (0: 실패 / 1: 성공 / 2: 성공 / 3:버전 다름.)
	//		INT64	AccountNo
	//	}
	//
	//  accountdb 에서 gamecodi_party 컬럼을 확인하여 이를 Status 로 보내줌.
	//
	//  Status 가 1 이면 파티 1번 유저
	//  Status 가 2 이면 파티 2번 유저	2017.02.01  용도 변경.
	//  Status 가 3 이면 실패
	//
	//  그러므로 항상 캐릭터 선택으로 진행됨.
	//
	//  Status 1 : 캐릭터 선택 화면으로 전환 /  Party 1 사용자.
	//  Status 2 : 캐릭터 선택 화면으로 전환 /  Party 2 사용자.
	//  Status 3 : 서버,클라의 버전 미스매치 
	//
	//  en_PACKET_CS_GAME_RES_LOGIN define 값 사용.
	//------------------------------------------------------------
	en_PACKET_CS_GAME_RES_LOGIN,



	//------------------------------------------------------------
	// 캐릭터 선택 요청!
	//
	//	{
	//		WORD	Type
	//
	//		BYTE	CharacterType ( 1 ~ 5 )
	//	}
	//
	// 로그인 성공 후 클라이언트는 캐릭터 선택 화면으로 전환이 되었으며,
	// 캐릭터 선택 후 본 패킷  REQ_CHARACTER_SELECT  을 서버로 보냄.
	//
	// 서버는 본 패킷을 받으면 신규 캐릭터 선택된 캐릭터를 플레이어에 저장하며, 기본 능력치로 셋팅된 정보를 저장 후 결과를 응답을 보냄.
	//
	// 캐릭터 번호는 en_PACKET_CS_GAME_CHARACTER_TYPE 값 사용.
	//------------------------------------------------------------
	en_PACKET_CS_GAME_REQ_CHARACTER_SELECT,

	//------------------------------------------------------------
	// 캐릭터 선택 응답.
	//
	//	{
	//		WORD	Type
	//
	//		BYTE	Status	( 0 : 실패 / 1 : 성공 )
	//	}
	//
	//  캐릭터 선택 결과 여부.  
	//
	//  캐릭터 선택에 성공 하였다면 Status 1 을 보냄. 
	//	실패는 Party 1 (1,2,3)  Party 2 (3,4,5) 의 범위가 아니라면 실패.
	// 
	//  파티 별 선택 캐릭터가 지정 되어 있으므로, 이 범위를 확인한다.
	//
	//  Status 0 은 캐릭터 생성 실패로 전송 후 접속을 끊으며,
	//  Status 1 은 신규 캐릭터 정보를 Player 에 저장하고 본 패킷 전송 후 GAME 모드로 전환, 게임 시작.
	//  
	//------------------------------------------------------------
	en_PACKET_CS_GAME_RES_CHARACTER_SELECT,




	// 죽으면 다시 캐릭터 선택을 해주어야 하는데... 이 부분은 고민이 필요함.















	//------------------------------------------------------------
	// 내 캐릭터 생성
	//
	//	{
	//		WORD	Type
	//		
	//		INT64	ClientID			// 이는 현 서버접속자 중 고유값 (ClientID or AccontNo)
	//									// 서버와 클라이언트가 각 플레이어를 구분 할 수 있는 수치이면 됨.
	//									// AccountNo 도 가능은 하나 다른 유저들의 AccountNo 를 공유하는건 좋지는 않음.
	//		BYTE	CharacterType
	//		WCHAR	Nickname[20]
	//		float	PosX
	//		float	PosY
	//		USHORT	Rotation
	//		int		Cristal				// 미사용
	//		int		HP
	//		INT64	Exp					// 미사용
	//		USHORT	Level				// 미사용
	//		BYTE	Party				// 1 또는 2  캐릭터의 파티정보
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_GAME_RES_CREATE_MY_CHARACTER,

	//------------------------------------------------------------
	// 다른 유저의  캐릭터 생성
	//
	//	{
	//		WORD	Type
	//
	//		INT64	ClientID
	//
	//		BYTE	CharacterType
	//		WCHAR	Nickname[20]
	//		float	PosX
	//		float	PosY
	//		USHORT	Rotation
	//		USHORT	Level
	//		BYTE	Respawn			// 신규 생성은 이펙트 터짐.
	//
	//		BYTE	Sit				// 미사용
	//		BYTE	Die				// 미사용
	//		int		HP				// 추가
	//		BYTE	Party			// 1 또는 2  캐릭터의 파티정보
	//
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_GAME_RES_CREATE_OTHER_CHARACTER,


	//------------------------------------------------------------
	// 캐릭터, 오브젝트의 삭제 (유저의 접속종료 또는 섹터에서 나감 등..)
	//
	//	{
	//		WORD	Type
	//
	//		INT64	ClientID
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_GAME_RES_REMOVE_OBJECT,







	//------------------------------------------------------------
	// 캐릭터 이동 요청 (유저가 마우스를 클릭하면 이를보냄.)
	// 
	// 클라이언트와 서버간의 좌표 전달은 모두 클라이언트의 소수점 좌표를 사용한다.
	// 클라좌표 - 서버타일좌표 / 2 + a  (a 는 자연스러움을 위해 서버가 랜덤하게 0.1 ~ 0.9 의 값을 지정함) 
	//
	// 클라이언트가 이동패킷을 보내면, 서버는 길을 찾아서 이동경로를 클라이언트에게 전달한다.
	// 그리고 서버역시 이에 맞도록 실제로 이동을 해야 함.
	// 
	// 이동경로는 목적지 까지의 꺽임 좌표 (JumpPointSearch 의 결과물) 로 보내주며
	// 클라이언트는 이동 경로를 받으면 경로대로 이동한다.
	//
	// 중간에라도 서버에서 경로가 바뀌는 경우는 바뀐 경로를 즉시 재전송, 또는 정지를 보내면 된다.
	// ( 1 요청에 1 응답일 필요가 없다는 뜻 )
	//
	// 최종적인 클라좌표 및 시선각도에 대한 동기화는
	// 클라이언트가 정지 했을 때, 정지 패킷으로 동기화를 맞추며 서버에서는 허용오차 범위를 확인하여
	// - 인정 (서버에 저장 및 타 유저 동기화)
	// - 거부 (클라 끊기)
	// 의 로직을 타면 됨.
	//
	//	{
	//		WORD	Type
	//
	//		INT64	ClientID
	//		float	StartX		// 클라 현 소수점 좌표
	//		float	StartY		// 클라 현 소수점 좌표
	//		float	DestX		// 클라 목적지 소수점 좌표
	//		float	DestY		// 클라 목적지 소수점 좌표
	//
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_GAME_REQ_MOVE_CHARACTER,


	//------------------------------------------------------------
	// 캐릭터 이동 결과 (클라가 이동요청을 보낸 후 길을 찾아서 결과 패킷을 당사자 및 주변에 뿌림)
	//
	// 서버는 이동요청시 찾아낸 JumpPointSearch 이동경로를 플레이어에 저장하고, 이 결과를 클라이언트들 에게 전달.
	// 서버도 시간에 따라 적절하게 이동해야함.
	//
	// 최종적인 동기화는 클라이언트가 목적지에 다달았을 때 서버로 정지 패킷을 보내줌.
	//
	//  PATH
	//  {
	//		float	X;		// 클라이언트 좌표
	//		float	Y;		// 클라이언트 좌표
	//  }
	//
	//
	//
	//	{
	//		WORD	Type
	//
	//		INT64	ClientID
	//
	//		char	Count			// 경로 개수
	//		PATH	Path[Count]		// 실제 경로
	//
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_GAME_RES_MOVE_CHARACTER,

	//------------------------------------------------------------
	// 캐릭터 정지 요청 
	//
	// 클라이언트 정지시, 서버로 정지를 보내며 허용범위를 확인하여 범위 내라면 서버가 좌표를 저장하고 주변에 뿌림.
	// 허용 범위를 초과한다면 연결을 끊음.
	//
	// 서버도 이동을 하고 있기때문에, 서버도 목적지에 도달했다면 알아서 정지만 해준다. (서버에서 목적지 도달시 패킷송신은 없음)
	// 이 정지 패킷은 클라가 정지하면서 동기화를 맞춰주기 위함.
	//
	//	{
	//		WORD	Type
	//
	//		INT64	ClientID
	//		float	X
	//		float	Y
	//		USHORT	Rotation
	//	}
	//
	// en_PACKET_CS_GAME_REQ_STOP_CHARACTER,
	// en_PACKET_CS_GAME_RES_STOP_CHARACTER,
	//
	// 2개의 패킷 모두 내용은 같음.
	//------------------------------------------------------------
	en_PACKET_CS_GAME_REQ_STOP_CHARACTER,
	en_PACKET_CS_GAME_RES_STOP_CHARACTER,

	







	//------------------------------------------------------------
	// 공격1  공격 대상서택 요청 / 응답  (왼클릭 공격 / 대상 1명 공격)
	// 공격2  공격 대상선택 요청 / 응답  (오른클릭 공격 / 대상 주변 광역공격 / 엘프는 광역 힐링)
	//
	// 선택받은 1명을 공격하는 패킷. 공격 데미지는 높지만 대상이 1명임.
	//
	// 클라이언트가 적 클릭시 REQ 패킷을 서버로 보냄.
	// 1. 서버는 대상 캐릭터가 공격가능한 대상인지 ( Party 별 캐릭터 타입 확인 / 엘프는 중립으로 모두 공격가능 )
	// 2. 공격가능한 대상이라면 플레이어에 공격대상 저장.
	//
	// 3. 공격가능 거리인지 확인.
	// 4. 공격가능 거리가 아니라면 대상 캐릭터를 향해 이동.  (일반 이동패킷과 동일하게 송신)
	// 5. 한 타일 한 타일 이동할 때 마다 공격가능 거리에 들어왔는지 확인.
	// 6. 공격가능 거리에 도착 하였다면 공격.  
	// 7. 공격텀 대기 후 3번으로 루프.
	// 8. 공격대상이 죽거나 대상을 변경할 때 까지 자동공격.
	// a. 이동,정지 패킷이 온다면 공격을 중단함 (공격대상 초기화)
	// 
	//
	// 공격데미지 - Attack1 : 100 내외 알아서 / Attack2 : 60 내외 알아서
	// 공격거리 - Attack1 : 타일거리 2 / Attack2 : 타일거리 2 ~ 10 범위
	// 공격대상 - Attack1 : 1명 / Attack2 : 대상 주변으로 반지름 5타일 정도 원형으로 지역공격
 	// 공격취소 - TargetID 가 0 으로 수신됨. (클라이언트 화면에서 대상이 없어지면 0 으로 공격취소를 서버로 보냄)
	//
	// 공격취소 및 변경
	// 
	// Attack 은 1종만 가능하므로 다른 Attack 이 들어오면 기존 AttackTarget 은 취소 처리 함
	// AttackTarget 이 설정된 뒤에 이동,정지,취소 명령이 온다면 공격을 취소시킴.
	//	{
	//		WORD	Type
	//
	//		INT64	AttackID		// 공격자 ID
	//		INT64	TargetID		// 피해자 ID
	//	}
	//
	// en_PACKET_CS_GAME_REQ_ATTACK1_TARGET,
	// en_PACKET_CS_GAME_RES_ATTACK1_TARGET,		// 미사용
	// en_PACKET_CS_GAME_REQ_ATTACK2_TARGET,
	// en_PACKET_CS_GAME_RES_ATTACK2_TARGET,		// 미사용
	//
	// 4개의 패킷 모두 내용은 같음.
	//------------------------------------------------------------
	en_PACKET_CS_GAME_REQ_ATTACK1_TARGET,
	en_PACKET_CS_GAME_RES_ATTACK1_TARGET,			// 미사용

	en_PACKET_CS_GAME_REQ_ATTACK2_TARGET,
	en_PACKET_CS_GAME_RES_ATTACK2_TARGET,			// 미사용



	//------------------------------------------------------------
	// 협동공격 요청.  - 추가예정....?
	//
	//------------------------------------------------------------
	en_PACKET_CS_GAME_REQ_ATTACK3_TARGET,
	en_PACKET_CS_GAME_RES_ATTACK3_TARGET,



	//------------------------------------------------------------
	// 공격액션!
	//
	// 공격대상이 공격 가능거리에 있을때 서버는 공격계산과 동시에 이 패킷을 주변에 뿌린다.
	// 사실 이 패킷은 공격행동 동기화를 위함이지  그 외의 다른 영향은 전혀 없음.
	// 또한 AttackID, TargetID 도 필요가 없으나, 클라이언트가 캐릭터의 방향을 제대로 잡기 위해서
	// 공격자와 피해자를 지정하도록 하고 있음. (클라가 알아서 피해자를 향하여 방향을 잡는다)
	//
	//	{
	//		WORD	Type
	//
	//		BYTE	AttackType		// 1:Attack1 / 2:Attack2 / 3:Attack3
	//		INT64	AttackID		// 공격자 ID
	//		INT64	TargetID		// 피해자 ID
	//		int		CoolTime		// 다음 공격 가능 대기 시간 ms  (ex 1000 - 1초 후 공격 가능)
	//	}
	//------------------------------------------------------------
	en_PACKET_CS_GAME_RES_ATTACK,


	//------------------------------------------------------------
	// 데미지
	//
	// 서버에서 공격대상에 데미지 또는 힐링를 먹인 후 이 패킷을 보냄.  해당 플레이어 주변에 모두 뿌림
	//
	//	{
	//		WORD	Type
	//
	//		INT64	DamageClientID
	//		int		DamageValue				// 데미지 먹은 수치 / 클라는 이 수치만큼 HP 를 차감함.
	//										// 힐링의 경우 마이너스 수치가 오므로 차감하면 더해짐.
	//										// 음수를 보내면 클라가 알아서 힐링으로 처리 함!
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_GAME_RES_DAMAGE,


	//------------------------------------------------------------
	// 엘프의 힐링 HP 회복   // 미사용
	//
	// 엘프의 힐링도 데미지로 통일 함.  데미지를 -로 넣어주면 힐링이 됨.
	//
	//	{
	//		WORD	Type
	//
	//		INT64	HealClientID
	//		int		HealValue				// 증가되는 HP 수치 / 클라는 이 수치만큼 HP 를 증가시킴.
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_GAME_RES_HEAL,




	//------------------------------------------------------------
	// 플레이어 HP 보정.  
	//
	// 서버측에서는 필요시 마다 본 패킷을 클라로 보내어 HP 수치를 맞춰줄 수 있음.
	// 수시로 보내는건 안좋으며, 필요시 사용가능
	//
	//
	//	{
	//		WORD	Type
	//
	//		INT		HP
	//	}
	//------------------------------------------------------------
	en_PACKET_CS_GAME_RES_PLAYER_HP,

	//------------------------------------------------------------
	// 플레이어 죽음
	//
	// 서버측에서 플레이어가 죽었을 시 클라이언트에 이 패킷을 보낸다.
	// 죽는 즉시 플레이어를 타일맵에서 제거하며, 섹터에서는 빼지 않는다.
	//
	// 왜냐면 죽어서 쓰러진 후에도 주변의 움직임이나 채팅 내용은 여전히 봐야하므로
	// 타일맵에서 제거하는 이유는 공격을 당하지 않기 위해서.
	//
	// 타일맵에서 제거 하지 않고 죽은 후에도 데미지를 먹게 해도 됨.
	// 
	// 죽으면 타이틀로 돌아가서 다시 시작을 유도한다.
	//
	//	{
	//		WORD	Type
	//		
	//		INT64	ClientID				// 죽은 캐릭터.
	//	}
	//------------------------------------------------------------
	en_PACKET_CS_GAME_RES_PLAYER_DIE,






	//------------------------------------------------------------
	// 테스트용 에코 요청
	//
	//	{
	//		WORD		Type
	//
	//		INT64		AccountoNo
	//		LONGLONG	SendTick
	//	}
	//
	//------------------------------------------------------------	
	en_PACKET_CS_GAME_REQ_ECHO				= 5000,

	//------------------------------------------------------------
	// 테스트용 에코 응답 (REQ 를 그대로 돌려줌)
	//
	//	{
	//		WORD		Type
	//
	//		INT64		AccountoNo
	//		LONGLONG	SendTick
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_GAME_RES_ECHO,

	//------------------------------------------------------------
	// 하트비트
	//
	//	{
	//		WORD		Type
	//	}
	//
	//
	// 클라이언트는 이를 30초마다 보내줌.
	// 서버는 40초 이상동안 메시지 수신이 없는 클라이언트를 강제로 끊어줘야 함.
	//------------------------------------------------------------	
	en_PACKET_CS_GAME_REQ_HEARTBEAT,


	//------------------------------------------------------------	
	// 좌표가 허용범위 이상 틀어질 경우 서버좌표로 클라를 맞춤.
	//
	// MoveStart, MoveStop 시 클라좌표가 허용범위 이상 틀어져 있다면 이 패킷을 주변에 뿌림.
	//
	//	{
	//		WORD	Type
	//
	//		INT64	ClientID
	//		float	X
	//		float	Y
	//	}
	//------------------------------------------------------------	
	en_PACKET_CS_GAME_RES_CHARACTER_SYNC,










	////////////////////////////////////////////////////////
	//
	//   Server & Server Protocol  / LAN 통신은 기본으로 응답을 받지 않음.
	//
	////////////////////////////////////////////////////////
	en_PACKET_SS_LAN						= 10000,
	//------------------------------------------------------
	// GameServer & LoginServer & ChatServer Protocol
	//------------------------------------------------------

	//------------------------------------------------------------
	// 다른 서버가 로그인 서버로 로그인.
	// 이는 응답이 없으며, 그냥 로그인 됨.  
	//
	//	{
	//		WORD	Type
	//
	//		BYTE	ServerType			// dfSERVER_TYPE_GAME / dfSERVER_TYPE_CHAT
	//
	//		WCHAR	ServerName[32]		// 해당 서버의 이름.  
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_SS_LOGINSERVER_LOGIN,

	
	
	//------------------------------------------------------------
	// 로그인서버에서 게임.채팅 서버로 새로운 클라이언트 접속을 알림.
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		CHAR	SessionKey[64]
	//	}
	//
	//------------------------------------------------------------
	// en_PACKET_SS_NEW_CLIENT_LOGIN,	// 신규 접속자의 세션키 전달패킷을 요청,응답구조로 변경 2017.01.05


	//------------------------------------------------------------
	// 로그인서버에서 게임.채팅 서버로 새로운 클라이언트 접속을 알림.
	//
	// 마지막의 Parameter 는 세션키 공유에 대한 고유값 확인을 위한 어떤 값. 이는 응답 결과에서 다시 받게 됨.
	// 채팅서버와 게임서버는 Parameter 에 대한 처리는 필요 없으며 그대로 Res 로 돌려줘야 합니다.
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		CHAR	SessionKey[64]
	//		INT64	Parameter
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_SS_REQ_NEW_CLIENT_LOGIN,

	//------------------------------------------------------------
	// 게임.채팅 서버가 새로운 클라이언트 접속패킷 수신결과를 돌려줌.
	// 게임서버용, 채팅서버용 패킷의 구분은 없으며, 로그인서버에 타 서버가 접속 시 CHAT,GAME 서버를 구분하므로 
	// 이를 사용해서 알아서 구분 하도록 함.
	//
	// 플레이어의 실제 로그인 완료는 이 패킷을 Chat,Game 양쪽에서 다 받았을 시점임.
	//
	// 마지막 값 Parameter 는 이번 세션키 공유에 대해 구분할 수 있는 특정 값
	// ClientID 를 쓰던, 고유 카운팅을 쓰던 상관 없음.
	//
	// 로그인서버에 접속과 재접속을 반복하는 경우 이전에 공유응답이 새로 접속한 뒤의 응답으로
	// 오해하여 다른 세션키를 들고 가는 문제가 생김.
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		INT64	Parameter
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_SS_RES_NEW_CLIENT_LOGIN,




	//------------------------------------------------------------
	// 게임서버와 채팅서버에서  로그인서버로 하트비트를 날림.
	//
	//	{
	//		WORD	Type
	//
	//		BYTE	ThreadType		// dfTHREAD_TYPE_WORKER / dfTHREAD_TYPE_DB / dfTHREAD_TYPE_GAME
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_SS_HEARTBEAT					= 11000,








	//------------------------------------------------------
	// Monitor Server Protocol
	//------------------------------------------------------


	////////////////////////////////////////////////////////
	//
	//   MonitorServer & MoniterTool Protocol / 응답을 받지 않음.
	//
	////////////////////////////////////////////////////////

	//------------------------------------------------------
	// Monitor Server  Protocol
	//------------------------------------------------------
	en_PACKET_SS_MONITOR					= 20000,
	//------------------------------------------------------
	// Server -> Monitor Protocol
	//------------------------------------------------------
	//------------------------------------------------------------
	// LoginServer, GameServer , ChatServer , Agent 가 모니터링 서버에 로그인 함
	//
	// 
	//	{
	//		WORD	Type
	//
	//		BYTE	ServerType				// Login / Game / Chat / Agent	하단 Define 됨.
	//		WCHAR	ServerName[32]			// Game,Chat,Agent 서버는 이름으로 서버 ID 를 맵핑함.
	//										// ServerLink.cnf 파일의 이름 사용
	//
	//										// Login 서버는 'LOGIN' 
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_SS_MONITOR_LOGIN,

	//------------------------------------------------------------
	// 서버가 모니터링서버로 데이터 전송
	// 각 서버는 자신이 모니터링중인 수치를 1초마다 모니터링 서버로 전송.
	//
	// 서버의 다운 및 기타 이유로 모니터링 데이터가 전달되지 못할떄를 대비하여 TimeStamp 를 전달한다.
	// 이는 모니터링 클라이언트에서 계산,비교 사용한다.
	// 
	//	{
	//		WORD	Type
	//
	//		BYTE	DataType				// 모니터링 데이터 Type 하단 Define 됨.
	//		int		DataValue				// 해당 데이터 수치.
	//		int		TimeStamp				// 해당 데이터를 얻은 시간 TIMESTAMP  (time() 함수)
	//										// 본래 time 함수는 time_t 타입변수이나 64bit 로 낭비스러우니
	//										// int 로 캐스팅하여 전송. 그래서 2038년 까지만 사용가능
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_SS_MONITOR_DATA_UPDATE,


	en_PACKET_CS_MONITOR					= 25000,
	//------------------------------------------------------
	// Monitor -> Monitor Tool Protocol  (Client <-> Server 프로토콜)
	//------------------------------------------------------
	//------------------------------------------------------------
	// 모니터링 클라이언트(툴) 이 모니터링 서버로 로그인 요청
	//
	//	{
	//		WORD	Type
	//
	//		WCHAR	ServerName[32]			// 모니터링 대상 서버 이름 
	//										// ServerLink.cnf 파일의 이름 사용
	//
	//										// 통합모니터링은 'COMMON' 사용
	//
	//		char	LoginSessionKey[32]		// 로그인 인증 키. (이는 모니터링 서버에 고정값으로 보유)
	//										// 각 모니터링 툴은 같은 키를 가지고 들어와야 함
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_MONITOR_TOOL_REQ_LOGIN,

	//------------------------------------------------------------
	// 모니터링 클라이언트(툴) 모니터링 서버로 로그인 응답
	//
	//	{
	//		WORD	Type
	//
	//		BYTE	ServerNo				// 지정 서버 No
	//		BYTE	Status					// 로그인 결과 0 / 1 / 2 ... 하단 Define
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_MONITOR_TOOL_RES_LOGIN,

	//------------------------------------------------------------
	// 모니터링 서버가 모니터링 클라이언트(툴) 에게 모니터링 데이터 전송
	// 모니터링 툴이 모니터링 서버로 로그인 시 서버를 지정 하였다면 해당 서버의 모니터링 데이터만 전송.
	//
	// 모니터링 툴이 모니터링 서버로 'COMMON' 통합 모니터링으로 로그인 하였다면 
	// 모든 서버에 대한 모니터링 데이터를 보내준다.
	//
	// 이 모니터링 데이터는 각 서버가 모니터링 서버에게 보내준 데이터를 그대로 릴레이 전달하는 데이터임.
	//
	// COMMON 통합 모니터링 클라의경우 모니터링 데이터가 생각보다 많음.
	// 이 데이터를 절약하기 위해서는 초단위로 모든 데이터를 묶어서 30~40개의 모니터링 데이터를 하나의 패킷으로 만드는게
	// 좋으나  여러가지 생각할 문제가 많으므로 그냥 각각의 모니터링 데이터를 개별적으로 전송처리 한다.
	//
	//	{
	//		WORD	Type
	//		
	//		BYTE	ServerNo				// 서버 No
	//		BYTE	DataType				// 모니터링 데이터 Type 하단 Define 됨.
	//		int		DataValue				// 해당 데이터 수치.
	//		int		TimeStamp				// 해당 데이터를 얻은 시간 TIMESTAMP  (time() 함수)
	//										// 본래 time 함수는 time_t 타입변수이나 64bit 로 낭비스러우니
	//										// int 로 캐스팅하여 전송. 그래서 2038년 까지만 사용가능
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE,


	//------------------------------------------------------------
	// 모니터링 클라이언트(툴) 가 모니터링 서버에게 서버 컨트롤
	//
	// 이는 모니터링 서버에서 각 클라(모니터링 툴) 에게 지정된 서버의 에이전트에게 재전달 됨.
	// * 채팅서버는 Shutdown 기능이 없음.  
	//
	//	{
	//		WORD	Type
	//
	//		BYTE	ServerType				// 컨트롤 대상 서버  하단 Define 사용
	//		BYTE	Control					// 컨트롤 명령, Run / Terminate / Shutdown   하단 Define 사용
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_CS_MONITOR_TOOL_SERVER_CONTROL,




	////////////////////////////////////////////////////////
	//
	//   GameServer & Agent Protocol / 응답을 받지 않음.
	//
	////////////////////////////////////////////////////////
	en_PACKET_SS_AGENT							= 30000,
	//------------------------------------------------------
	// Agent Protocol
	//------------------------------------------------------
	//------------------------------------------------------------
	// Agent 가 GameServer 에게 서버종료 명령을 날림
	// 게임서버는 Agent 에게 이 메시지를 받으면 즉시 서버를 중단한다.
	//
	//	{
	//		WORD	Type
	//
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_SS_AGENT_GAMESERVER_SHUTDOWN,

};



enum en_PACKET_CS_LOGIN_RES_LOGIN 
{
	dfLOGIN_STATUS_NONE				= -1,		// 미인증상태
	dfLOGIN_STATUS_FAIL				= 0,		// 세션오류
	dfLOGIN_STATUS_OK				= 1,		// 성공
	dfLOGIN_STATUS_GAME				= 2,		// 게임중
	dfLOGIN_STATUS_ACCOUNT_MISS		= 3,		// account 테이블에 AccountNo 없음
	dfLOGIN_STATUS_SESSION_MISS		= 4,		// Session 테이블에 AccountNo 없음
	dfLOGIN_STATUS_STATUS_MISS		= 5,		// Status 테이블에 AccountNo 없음
	dfLOGIN_STATUS_NOSERVER			= 6,		// 서비스중인 서버가 없음.
};


enum en_PACKET_CS_GAME_RES_LOGIN 
{
	dfGAME_LOGIN_FAIL				= 0,		// 세션키 오류 또는 Account 데이블상의 오류
	dfGAME_LOGIN_PARTY1				= 1,		// 성공 > 파티 1번
	dfGAME_LOGIN_PARTY2				= 2,		// 성공 > 파티 2번
	dfGAME_LOGIN_VERSION_MISS		= 3,		// 서버,클라 버전 다름
};


enum en_PACKET_CS_GAME_CHARACTER_TYPE
{
	dfGAME_CHARACTER_GOLEM			= 1,		// 골렘
	dfGAME_CHARACTER_KNIGHT			= 2,		// 기사
	dfGAME_CHARACTER_ELF			= 3,		// 엘프
	dfGAME_CHARACTER_ORC			= 4,		// 오크
	dfGAME_CHARACTER_ARCHER			= 5,		// 궁수
	dfGAME_CHARACTER_MONSTER		= 6,		// 몬스터 // 미사용
};


enum en_PACKET_SS_LOGINSERVER_LOGIN
{
	dfSERVER_TYPE_GAME		= 1,
	dfSERVER_TYPE_CHAT		= 2,
	dfSERVER_TYPE_MONITOR	= 3,
};

enum en_PACKET_SS_HEARTBEAT
{
	dfTHREAD_TYPE_WORKER	= 1,
	dfTHREAD_TYPE_DB		= 2,
	dfTHREAD_TYPE_GAME		= 3,
};

// en_PACKET_SS_MONITOR_LOGIN
enum en_PACKET_CS_MONITOR_TOOL_SERVER_CONTROL
{
	dfMONITOR_SERVER_TYPE_LOGIN		= 1,
	dfMONITOR_SERVER_TYPE_GAME		= 2,
	dfMONITOR_SERVER_TYPE_CHAT		= 3,
	dfMONITOR_SERVER_TYPE_AGENT		= 4,

	dfMONITOR_SERVER_CONTROL_SHUTDOWN			= 1,		// 서버 정상종료 (게임서버 전용)
	dfMONITOR_SERVER_CONTROL_TERMINATE			= 2,		// 서버 프로세스 강제종료
	dfMONITOR_SERVER_CONTROL_RUN				= 3,		// 서버 프로세스 생성 & 실행
};


enum en_PACKET_SS_MONITOR_DATA_UPDATE
{
	dfMONITOR_DATA_TYPE_LOGIN_SESSION				= 1,		// 로그인서버 세션 수 (컨넥션 수)
	dfMONITOR_DATA_TYPE_LOGIN_AUTH_TPS				= 2,		// 로그인서버 인증 처리 초당 횟수
	dfMONITOR_DATA_TYPE_LOGIN_PACKET_POOL			= 3,		// 로그인서버 패킷풀 사용량
	dfMONITOR_DATA_TYPE_LOGIN_SERVER_ON				= 4,		// 켜진서버 서버 개수
	dfMONITOR_DATA_TYPE_LOGIN_LIVE_SERVER			= 5,		// 현재 라이브 지정 서버 번호

	dfMONITOR_DATA_TYPE_GAME_SESSION				= 6,		// 게임서버 세션 수 (컨넥션 수)
	dfMONITOR_DATA_TYPE_GAME_AUTH_PLAYER			= 7,		// 게임서버 AUTH MODE 플레이어 수
	dfMONITOR_DATA_TYPE_GAME_GAME_PLAYER			= 8,		// 게임서버 GAME MODE 플레이어 수
	dfMONITOR_DATA_TYPE_GAME_ACCEPT_TPS				= 9,		// 게임서버 Accept 처리 초당 횟수
	dfMONITOR_DATA_TYPE_GAME_PACKET_PROC_TPS		= 10,		// 게임서버 패킷처리 초당 횟수
	dfMONITOR_DATA_TYPE_GAME_PACKET_SEND_TPS		= 11,		// 게임서버 패킷 보내기 초당 완료 횟수
	dfMONITOR_DATA_TYPE_GAME_DB_WRITE_TPS			= 12,		// 게임서버 DB 저장 메시지 초당 처리 횟수
	dfMONITOR_DATA_TYPE_GAME_DB_WRITE_MSG			= 13,		// 게임서버 DB 저장 메시지 버퍼 개수 (남은 수)
	dfMONITOR_DATA_TYPE_GAME_AUTH_THREAD_FPS		= 14,		// 게임서버 AUTH 스레드 초당 프레임 수 (루프 수)
	dfMONITOR_DATA_TYPE_GAME_GAME_THREAD_FPS		= 15,		// 게임서버 GAME 스레드 초당 프레임 수 (루프 수)
	dfMONITOR_DATA_TYPE_GAME_PACKET_POOL			= 16,		// 게임서버 패킷풀 사용량
	
	dfMONITOR_DATA_TYPE_CHAT_SESSION				= 17,		// 채팅서버 세션 수 (컨넥션 수)
	dfMONITOR_DATA_TYPE_CHAT_PLAYER					= 18,		// 채팅서버 인증성공 사용자 수 (실제 접속자)
	dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS				= 19,		// 채팅서버 UPDATE 스레드 초당 초리 횟수
	dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL			= 20,		// 채팅서버 패킷풀 사용량
	dfMONITOR_DATA_TYPE_CHAT_UPDATEMSG_POOL			= 21,		// 채팅서버 UPDATE MSG 풀 사용량
	
	dfMONITOR_DATA_TYPE_AGENT_GAME_SERVER_RUN		= 22,		// 에이전트 GameServer 실행 여부 ON / OFF
	dfMONITOR_DATA_TYPE_AGENT_CHAT_SERVER_RUN		= 23,		// 에이전트 ChatServer 실행 여부 ON / OFF
	dfMONITOR_DATA_TYPE_AGENT_GAME_SERVER_CPU		= 24,		// 에이전트 GameServer CPU 사용률
	dfMONITOR_DATA_TYPE_AGENT_CHAT_SERVER_CPU		= 25,		// 에이전트 ChatServer CPU 사용률
	dfMONITOR_DATA_TYPE_AGENT_GAME_SERVER_MEM		= 26,		// 에이전트 GameServer 메모리 사용 MByte
	dfMONITOR_DATA_TYPE_AGENT_CHAT_SERVER_MEM		= 27,		// 에이전트 ChatServer 메모리 사용 MByte
	dfMONITOR_DATA_TYPE_AGENT_CPU_TOTAL				= 28,		// 에이전트 서버컴퓨터 CPU 전체 사용률
	dfMONITOR_DATA_TYPE_AGENT_NONPAGED_MEMORY		= 29,		// 에이전트 서버컴퓨터 논페이지 메모리 MByte	// 최근 수정
	dfMONITOR_DATA_TYPE_AGENT_NETWORK_RECV			= 30,		// 에이전트 서버컴퓨터 네트워크 수신량 KByte	// 최근 수정
	dfMONITOR_DATA_TYPE_AGENT_NETWORK_SEND			= 31,		// 에이전트 서버컴퓨터 네트워크 송신량 KByte	// 최근 수정
	dfMONITOR_DATA_TYPE_AGENT_						= 32,		// 에이전트 서버컴퓨터 
	dfMONITOR_DATA_TYPE_AGENT_AVAILABLE_MEMORY		= 33,		// 에이전트 서버컴퓨터 사용가능 메모리
};


enum en_PACKET_CS_MONITOR_TOOL_RES_LOGIN
{
	dfMONITOR_TOOL_LOGIN_OK						= 1,		// 로그인 성공
	dfMONITOR_TOOL_LOGIN_ERR_NOSERVER			= 2,		// 서버이름 오류 (매칭미스)
	dfMONITOR_TOOL_LOGIN_ERR_SESSIONKEY			= 3,		// 로그인 세션키 오류
};


//#endif