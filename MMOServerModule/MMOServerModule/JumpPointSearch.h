#pragma once

class CJumpPointSearch
{
private:
	enum en_TILE_PROPERTY
	{
		en_TILE_NONE = 0,
		en_TILE_FIX_OBSTACLE,
		en_TILE_NON_FIX_OBSTACLE,
	};

	// 방향 값
	enum eDIRECTION
	{
		en_DIR_NN = 0,
		en_DIR_UU,
		en_DIR_RU,
		en_DIR_RR,
		en_DIR_RD,
		en_DIR_DD,
		en_DIR_LD,
		en_DIR_LL,
		en_DIR_LU
	};

public:
	struct PATH
	{
		float	X;		// 클라이언트 좌표
		float	Y;		// 클라이언트 좌표
	};

private:
	typedef struct stJUMP_NODE
	{
		stJUMP_NODE *pParent;

		int _iGoal;							// Goal. 출발점부터의 이동 횟수
		int _iHeuristic;							// Heuristic. 목적지까지의 거리
		int _iFitness;							// Fitness. G와 H를 더한 값

		WORD _wPosX;							// 노드의 X좌표
		WORD _wPosY;							// 노드의 Y좌표

		BYTE _byDir;                       // 노드의 방향
	}NODE;

public:
	CJumpPointSearch(WORD width, WORD height);
	~CJumpPointSearch();

	
	void JumpPointSearch_Init(void);			// 객체를 사용하기 위한 세팅
	bool LoadTextMap(wchar_t *szFileName);		// 맵 텍스트 파일 로드
	
	bool FindPath(WORD wStartX, WORD wStartY, WORD wEndX, WORD wEndY, PATH *pOut, int *iOutCount);	// 길 찾기


private:
	void SetFixedObstacle(WORD wX, WORD wY);

	NODE* CreateNode(NODE *pParents, WORD wPosX, WORD wPosY, BYTE byDir);

	bool Jump(NODE *pNode, BYTE byDir);
	bool Jump_DirUU(short wInX, short wInY, short *wOutX, short *wOutY);
	bool Jump_DirRR(short wInX, short wInY, short *wOutX, short *wOutY);
	bool Jump_DirDD(short wInX, short wInY, short *wOutX, short *wOutY);
	bool Jump_DirLL(short wInX, short wInY, short *wOutX, short *wOutY);
	bool Jump_DirRU(short wInX, short wInY, short *wMiddleOutX, short *wMiddleOutY, short *wOutX, short *wOutY, BYTE *byOutDir);
	bool Jump_DirRD(short wInX, short wInY, short *wMiddleOutX, short *wMiddleOutY, short *wOutX, short *wOutY, BYTE *byOutDir);
	bool Jump_DirLD(short wInX, short wInY, short *wMiddleOutX, short *wMiddleOutY, short *wOutX, short *wOutY, BYTE *byOutDir);
	bool Jump_DirLU(short wInX, short wInY, short *wMiddleOutX, short *wMiddleOutY, short *wOutX, short *wOutY, BYTE *byOutDir);

	bool CheckDestination(WORD wPosX, WORD wPosY);

	void CompleteFind(NODE *pNode, PATH *pOut, int *iOutCount);

private:
	CMemoryPool<NODE> _nodePool;

	std::list<NODE*> _openList;
	std::list<NODE*> _closeList;

	WORD _width;
	WORD _height;

	WORD _wStartX;
	WORD _wStartY;

	WORD _wEndX;
	WORD _wEndY;

	int **map;
};