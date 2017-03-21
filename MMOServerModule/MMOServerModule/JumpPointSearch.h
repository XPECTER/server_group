#pragma once

struct PATH
{
	float	X;		// 클라이언트 좌표
	float	Y;		// 클라이언트 좌표
};

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
	

public:
	typedef struct stJUMP_NODE
	{
		stJUMP_NODE *pParent;

		int _iGoal;							// Goal. 출발점부터의 이동 횟수
		int _iHeuristic;							// Heuristic. 목적지까지의 거리
		int _iFitness;							// Fitness. G와 H를 더한 값

		int _iPosX;							// 노드의 X좌표
		int _iPosY;							// 노드의 Y좌표

		BYTE _byDir;                       // 노드의 방향
	}NODE;

public:
	CJumpPointSearch(WORD width, WORD height);
	~CJumpPointSearch();

	
	void JumpPointSearch_Init(void);			// 객체를 사용하기 위한 세팅
	bool LoadTextMap(wchar_t *szFileName, char *obstacle);		// 맵 텍스트 파일 로드
	
	bool FindPath(int iStartX, int iStartY, int iEndX, int iEndY, PATH *pOut, int *iOutCount);	// 길 찾기

private:
	void SetTileProperty(int iX, int iY, en_TILE_PROPERTY type);

	NODE* CreateNode(NODE *pParents, int iPosX, int iPosY, BYTE byDir);

	void Jump(NODE *pNode, BYTE byDir);
	bool Jump_DirUU(int iInX, int iInY, int *iOutX, int *iOutY);
	bool Jump_DirRR(int iInX, int iInY, int *iOutX, int *iOutY);
	bool Jump_DirDD(int iInX, int iInY, int *iOutX, int *iOutY);
	bool Jump_DirLL(int iInX, int iInY, int *iOutX, int *iOutY);
	bool Jump_DirRU(NODE *pParents, int iPosX, int iPosY, bool bLoop);
	bool Jump_DirRD(NODE *pParents, int iPosX, int iPosY, bool bLoop);
	bool Jump_DirLD(NODE *pParents, int iPosX, int iPosY, bool bLoop);
	bool Jump_DirLU(NODE *pParents, int iPosX, int iPosY, bool bLoop);

	bool CheckRange(int iPosX, int iPosY);

	bool CheckDestination(int iPosX, int iPosY);

	void CompleteFind(NODE *pNode, PATH *pOut, int *iOutCount);

private:
	CMemoryPool<NODE> _nodePool;

	std::list<NODE*> _openList;
	std::list<NODE*> _closeList;

	int _width;
	int _height;

	int _iStartX;
	int _iStartY;
		 
	int _iEndX;
	int _iEndY;

	int **map;
};

bool Compare(CJumpPointSearch::NODE *a, CJumpPointSearch::NODE *b);