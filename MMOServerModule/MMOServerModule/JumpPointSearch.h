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

	// ���� ��
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
		float	X;		// Ŭ���̾�Ʈ ��ǥ
		float	Y;		// Ŭ���̾�Ʈ ��ǥ
	};

private:
	typedef struct stJUMP_NODE
	{
		stJUMP_NODE *pParent;

		int _iGoal;							// Goal. ����������� �̵� Ƚ��
		int _iHeuristic;							// Heuristic. ������������ �Ÿ�
		int _iFitness;							// Fitness. G�� H�� ���� ��

		WORD _wPosX;							// ����� X��ǥ
		WORD _wPosY;							// ����� Y��ǥ

		BYTE _byDir;                       // ����� ����
	}NODE;

public:
	CJumpPointSearch(WORD width, WORD height);
	~CJumpPointSearch();

	
	void JumpPointSearch_Init(void);			// ��ü�� ����ϱ� ���� ����
	bool LoadTextMap(wchar_t *szFileName);		// �� �ؽ�Ʈ ���� �ε�
	
	bool FindPath(WORD wStartX, WORD wStartY, WORD wEndX, WORD wEndY, PATH *pOut, int *iOutCount);	// �� ã��


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