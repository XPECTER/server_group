#pragma once

struct PATH
{
	float	X;		// Ŭ���̾�Ʈ ��ǥ
	float	Y;		// Ŭ���̾�Ʈ ��ǥ
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
	

public:
	typedef struct stJUMP_NODE
	{
		stJUMP_NODE *pParent;

		int _iGoal;							// Goal. ����������� �̵� Ƚ��
		int _iHeuristic;							// Heuristic. ������������ �Ÿ�
		int _iFitness;							// Fitness. G�� H�� ���� ��

		int _iPosX;							// ����� X��ǥ
		int _iPosY;							// ����� Y��ǥ

		BYTE _byDir;                       // ����� ����
	}NODE;

public:
	CJumpPointSearch(WORD width, WORD height);
	~CJumpPointSearch();

	
	void JumpPointSearch_Init(void);			// ��ü�� ����ϱ� ���� ����
	bool LoadTextMap(wchar_t *szFileName, char *obstacle);		// �� �ؽ�Ʈ ���� �ε�
	
	bool FindPath(int iStartX, int iStartY, int iEndX, int iEndY, PATH *pOut, int *iOutCount);	// �� ã��

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