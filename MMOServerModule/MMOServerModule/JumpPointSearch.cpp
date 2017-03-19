#include "stdafx.h"
#include "JumpPointSearch.h"

// 서버가 사용할 맵 크기로 받는다.
// 클라이언트는 200 * 100을 사용하고 서버는 400 * 200을 사용한다.
CJumpPointSearch::CJumpPointSearch(WORD width, WORD height)
{
	this->_width = width;
	this->_height = height;
	this->map = NULL;
}

CJumpPointSearch::~CJumpPointSearch()
{

}

bool CJumpPointSearch::LoadTextMap(wchar_t *szFileName)
{
	FILE *pFile = NULL;

	if (0 != _wfopen_s(&pFile, szFileName, L"r"))
	{
		SYSLOG(L"", LOG::LEVEL_ERROR, L"Text Map Load Failed");
		return false;
	}

	fseek(pFile, 3, SEEK_SET);	// BOM 코드 제외
	char row[1024] = { 0, };
	char *token = NULL;
	char *next = NULL;

	int iClientMapWidth = this->_width / 2;
	int iClientMapHeight = this->_height / 2;

	for (int iHeightCnt = 0; iHeightCnt < iClientMapHeight; ++iHeightCnt)
	{
		fgets(row, 1024, pFile);
		token = strtok_s(row, " ,\n", &next);

		for (int iWidthCnt = 0; iWidthCnt < iClientMapWidth; ++iWidthCnt)
		{
			if (NULL != strstr(token, "X"))
			{
				SetFixedObstacle(iWidthCnt, iHeightCnt);
			}

			token = strtok_s(NULL, ",\n", &next);

			if (NULL == token)
				break;
		}
	}

	return true;
}

void CJumpPointSearch::JumpPointSearch_Init(void)
{
	this->map = new int*[this->_height];
	for (int iCnt = 0; iCnt < this->_height; ++iCnt)
		this->map[iCnt] = new int[this->_width];

	for (int iHeightCnt = 0; iHeightCnt < this->_height; ++iHeightCnt)
		for (int iWidthCnt = 0; iWidthCnt < this->_width; ++iWidthCnt)
			this->map[iHeightCnt][iWidthCnt] = en_TILE_NONE;
}

void CJumpPointSearch::SetFixedObstacle(unsigned short shX, unsigned short shY)
{
	if (NULL == this->map)
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Map don`t create yet");
		return;
	}
	
	WORD shServerX = shX * 2;
	WORD shServerY = shY * 2;

	// 서버에선 클라이언트의 맵 크기의 X2 만큼 사용하기 때문에 네 곳을 지정해야한다.
	this->map[shServerY][shServerX]			= en_TILE_FIX_OBSTACLE;
	this->map[shServerY][shServerX + 1]		= en_TILE_FIX_OBSTACLE;
	this->map[shServerY + 1][shServerX]		= en_TILE_FIX_OBSTACLE;
	this->map[shServerY + 1][shServerX + 1] = en_TILE_FIX_OBSTACLE;

	return;
}

bool CJumpPointSearch::FindPath(WORD wStartX, WORD wStartY, WORD wEndX, WORD wEndY, PATH *pOut, int *iOutCount)
{
	if (NULL == this->map)
	{
		return false;
	}

	if (en_TILE_FIX_OBSTACLE == this->map[wStartY][wStartX])
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_DEBUG, L" [JPS] wrong start position");
		return false;
	}

	if (en_TILE_FIX_OBSTACLE == this->map[wEndY][wEndX])
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_DEBUG, L" [JPS] wrong end position");
		return false;
	}

	bool bResult = false;

	this->_wStartX = wStartX;
	this->_wStartY = wStartY;
	this->_wEndX = wEndX;
	this->_wEndY = wEndY;

	// 시작점 노드
	this->CreateNode(NULL, wStartX, wStartY, en_DIR_NN);

	for (int i = 0; i < dfPATH_OPEN_MAX; ++i)
	{
		// 정렬이 되어있다는 전제
		auto iter = this->_openList.begin();
		NODE *pNode = (*iter);

		if (this->CheckDestination((*iter)->_wPosX, (*iter)->_wPosY))
		{
			// Path 구조체 담아서 넘겨줘라
			this->CompleteFind((*iter), pOut, iOutCount);
			return true;
		}

		this->Jump(pNode, pNode->_byDir);

		// closeList에 넣는건 그냥 넣어도 됨
		this->_closeList.push_back((*iter));
		this->_openList.erase(iter);
	}

	return false;
}

CJumpPointSearch::NODE* CJumpPointSearch::CreateNode(NODE *pParents, WORD wPosX, WORD wPosY, BYTE byDir)
{
	NODE *pNewNode = this->_nodePool.Alloc();
	pNewNode->pParent = pParents;
	pNewNode->_byDir = byDir;
	pNewNode->_wPosX = wPosX;
	pNewNode->_wPosY = wPosY;

	if (NULL != pParents)
	{
		if (pParents->_wPosX == pNewNode->_wPosX || pParents->_wPosY == pNewNode->_wPosY)
			pNewNode->_iGoal = (abs(pParents->_wPosY - pNewNode->_wPosY) + abs(pParents->_wPosX - pNewNode->_wPosX)) * 10;
		else
			pNewNode->_iGoal = (abs(pParents->_wPosY - pNewNode->_wPosY) + abs(pParents->_wPosX - pNewNode->_wPosX)) * 14;	// 대각선 가중치 1.4
	}
	else
	{
		pNewNode->_iGoal = 0;
	}

	pNewNode->_iHeuristic = (abs(this->_wEndX - pNewNode->_wPosX) + abs(this->_wEndY - pNewNode->_wPosY)) * 10;
	pNewNode->_iFitness = pNewNode->_iGoal + pNewNode->_iHeuristic;

	this->_openList.push_back(pNewNode);

	// Fitness 값에 의한 정렬을 해줘야 한다.
	this->_openList.sort(Compare);
	return pNewNode;
}

bool CJumpPointSearch::CheckDestination(WORD wX, WORD wY)
{
	if (this->_wEndX == wX && this->_wEndY == wY)
		return true;
	else
		return false;
}

void CJumpPointSearch::Jump(NODE *pNode, BYTE byDir)
{
	short shOutX = -1;
	short shOutY = -1;
	short shMiddleX = -1;
	short shMiddleY = -1;
	NODE *pMiddleNode = NULL;
	BYTE byOutDir;

	switch (byDir)
	{
		case en_DIR_NN:
		{
			if (this->Jump_DirUU(pNode->_wPosX, pNode->_wPosY, &shOutX, &shOutY))
			{
				this->CreateNode(pNode, shOutX, shOutY, en_DIR_UU);
			}

			if (this->Jump_DirRR(pNode->_wPosX, pNode->_wPosY, &shOutX, &shOutY))
			{
				this->CreateNode(pNode, shOutX, shOutY, en_DIR_RR);
			}

			if (this->Jump_DirDD(pNode->_wPosX, pNode->_wPosY, &shOutX, &shOutY))
			{
				this->CreateNode(pNode, shOutX, shOutY, en_DIR_DD);
			}

			if (this->Jump_DirLL(pNode->_wPosX, pNode->_wPosY, &shOutX, &shOutY))
			{
				this->CreateNode(pNode, shOutX, shOutY, en_DIR_LL);
			}

			if (this->Jump_DirRU(pNode->_wPosX, pNode->_wPosY, &shMiddleX, &shMiddleY, &shOutX, &shOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, shMiddleX, shMiddleY, en_DIR_RU);
				this->CreateNode(pMiddleNode, shOutX, shOutY, byOutDir);
			}

			if (this->Jump_DirRD(pNode->_wPosX, pNode->_wPosY, &shMiddleX, &shMiddleY, &shOutX, &shOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, shMiddleX, shMiddleY, en_DIR_RD);
				this->CreateNode(pMiddleNode, shOutX, shOutY, byOutDir);
			}

			if (this->Jump_DirLD(pNode->_wPosX, pNode->_wPosY, &shMiddleX, &shMiddleY, &shOutX, &shOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, shMiddleX, shMiddleY, en_DIR_LD);
				this->CreateNode(pMiddleNode, shOutX, shOutY, byOutDir);
			}

			if (this->Jump_DirLU(pNode->_wPosX, pNode->_wPosY, &shMiddleX, &shMiddleY, &shOutX, &shOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, shMiddleX, shMiddleY, en_DIR_LU);
				this->CreateNode(pMiddleNode, shOutX, shOutY, byOutDir);
			}

			break;
		}

		case en_DIR_UU:
		{
			if (this->Jump_DirUU(pNode->_wPosX, pNode->_wPosY, &shOutX, &shOutY))
			{
				this->CreateNode(pNode, shOutX, shOutY, en_DIR_UU);
			}

			if (this->Jump_DirLU(pNode->_wPosX, pNode->_wPosY, &shMiddleX, &shMiddleY, &shOutX, &shOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, shMiddleX, shMiddleY, en_DIR_LU);
				this->CreateNode(pMiddleNode, shOutX, shOutY, byOutDir);
			}

			if (this->Jump_DirRU(pNode->_wPosX, pNode->_wPosY, &shMiddleX, &shMiddleY, &shOutX, &shOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, shMiddleX, shMiddleY, en_DIR_RU);
				this->CreateNode(pMiddleNode, shOutX, shOutY, byOutDir);
			}

			break;
		}

		case en_DIR_RR:
		{
			if (this->Jump_DirRR(pNode->_wPosX, pNode->_wPosY, &shOutX, &shOutY))
			{
				this->CreateNode(pNode, shOutX, shOutY, en_DIR_RR);
			}

			if (this->Jump_DirRU(pNode->_wPosX, pNode->_wPosY, &shMiddleX, &shMiddleY, &shOutX, &shOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, shMiddleX, shMiddleY, en_DIR_RU);
				this->CreateNode(pMiddleNode, shOutX, shOutY, byOutDir);
			}

			if (this->Jump_DirRD(pNode->_wPosX, pNode->_wPosY, &shMiddleX, &shMiddleY, &shOutX, &shOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, shMiddleX, shMiddleY, en_DIR_RD);
				this->CreateNode(pMiddleNode, shOutX, shOutY, byOutDir);
			}

			break;
		}

		case en_DIR_DD:
		{
			if (this->Jump_DirDD(pNode->_wPosX, pNode->_wPosY, &shOutX, &shOutY))
			{
				this->CreateNode(pNode, shOutX, shOutY, en_DIR_DD);
			}

			if (this->Jump_DirRD(pNode->_wPosX, pNode->_wPosY, &shMiddleX, &shMiddleY, &shOutX, &shOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, shMiddleX, shMiddleY, en_DIR_RD);
				this->CreateNode(pMiddleNode, shOutX, shOutY, byOutDir);
			}

			if (this->Jump_DirLD(pNode->_wPosX, pNode->_wPosY, &shMiddleX, &shMiddleY, &shOutX, &shOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, shMiddleX, shMiddleY, en_DIR_LD);
				this->CreateNode(pMiddleNode, shOutX, shOutY, byOutDir);
			}

			break;
		}

		case en_DIR_LL:
		{
			if (this->Jump_DirLL(pNode->_wPosX, pNode->_wPosY, &shOutX, &shOutY))
			{
				this->CreateNode(pNode, shOutX, shOutY, en_DIR_LL);
			}

			if (this->Jump_DirLD(pNode->_wPosX, pNode->_wPosY, &shMiddleX, &shMiddleY, &shOutX, &shOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, shMiddleX, shMiddleY, en_DIR_LD);
				this->CreateNode(pMiddleNode, shOutX, shOutY, byOutDir);
			}

			if (this->Jump_DirLU(pNode->_wPosX, pNode->_wPosY, &shMiddleX, &shMiddleY, &shOutX, &shOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, shMiddleX, shMiddleY, en_DIR_LU);
				this->CreateNode(pMiddleNode, shOutX, shOutY, byOutDir);
			}

			break;
		}

		case en_DIR_RU:
		{
			if (this->Jump_DirRU(pNode->_wPosX, pNode->_wPosY, &shMiddleX, &shMiddleY, &shOutX, &shOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, shMiddleX, shMiddleY, en_DIR_RU);
				this->CreateNode(pMiddleNode, shOutX, shOutY, byOutDir);
			}

			break;
		}

		case en_DIR_RD:
		{
			if (this->Jump_DirRD(pNode->_wPosX, pNode->_wPosY, &shMiddleX, &shMiddleY, &shOutX, &shOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, shMiddleX, shMiddleY, en_DIR_RD);
				this->CreateNode(pMiddleNode, shOutX, shOutY, byOutDir);
			}

			break;
		}

		case en_DIR_LD:
		{
			if (this->Jump_DirLD(pNode->_wPosX, pNode->_wPosY, &shMiddleX, &shMiddleY, &shOutX, &shOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, shMiddleX, shMiddleY, en_DIR_LD);
				this->CreateNode(pMiddleNode, shOutX, shOutY, byOutDir);
			}

			break;
		}

		case en_DIR_LU:
		{
			if (this->Jump_DirLU(pNode->_wPosX, pNode->_wPosY, &shMiddleX, &shMiddleY, &shOutX, &shOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, shMiddleX, shMiddleY, en_DIR_LU);
				this->CreateNode(pMiddleNode, shOutX, shOutY, byOutDir);
			}

			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////
//
//	함수 : Jump_DirUU(short wInX, short wInY, short *wOutX, short *wOutY)
//	인자 : 
//		short wInX		: 로직을 시작할 X좌표
//		short wInY		: 로직을 시작할 Y좌표
//		short *wOutX	: 목적지를 찾았거나 중간 노드 생성 위치를 찾으면 세팅한다.
//		short *wOutY	: 목적지를 찾았거나 중간 노드 생성 위치를 찾으면 세팅한다.
//
//	로직 실행할 노드의 위치에서 윗 방향으로 검색하는 함수
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirUU(short wInX, short wInY, short *wOutX, short *wOutY)
{
	short wX = (short)wInX;
	short wY = (short)wInY;

	while (true)
	{
		wY--;

		if (!this->CheckRange(wX, wY))
			break;

		if ((0 > (wY - 1)) || (en_TILE_FIX_OBSTACLE == map[wY][wX]))
			break;

		if (this->CheckDestination(wX, wY))
		{
			*wOutX = wX;
			*wOutY = wY;
			return true;
		}

		// 양 쪽에 벽이 없음
		if (0 <= wX - 1 && this->_width >= wX + 2)
		{
			// 왼쪽 체크
			if (en_TILE_FIX_OBSTACLE == map[wY][wX - 1] && en_TILE_NONE == map[wY - 1][wX - 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
			// 오른쪽 체크
			else if (en_TILE_FIX_OBSTACLE == map[wY][wX + 1] && en_TILE_NONE == map[wY - 1][wX + 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
		// 왼쪽이 벽
		else if (0 > wX - 1)
		{
			if (en_TILE_FIX_OBSTACLE == map[wY][wX + 1] && en_TILE_NONE == map[wY - 1][wX + 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
		// 오른쪽이 벽
		else
		{
			if (en_TILE_FIX_OBSTACLE == map[wY][wX - 1] && en_TILE_NONE == map[wY - 1][wX - 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	함수 : Jump_DirRR(short wInX, short wInY, short *wOutX, short *wOutY)
//	인자 : 
//		short wInX		: 로직을 시작할 X좌표
//		short wInY		: 로직을 시작할 Y좌표
//		short *wOutX	: 목적지를 찾았거나 중간 노드 생성 위치를 찾으면 세팅한다.
//		short *wOutY	: 목적지를 찾았거나 중간 노드 생성 위치를 찾으면 세팅한다.
//
//	로직 실행할 노드의 위치에서 오른 방향으로 검색하는 함수
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirRR(short wInX, short wInY, short *wOutX, short *wOutY)
{
	short wX = (short)wInX;
	short wY = (short)wInY;

	while (true)
	{
		wX++;

		if (!this->CheckRange(wX, wY))
			break;

		if (((this->_width - 1) < (wX + 1)) || en_TILE_FIX_OBSTACLE == map[wY][wX])
			break;

		if (this->CheckDestination(wX, wY))
		{
			*wOutX = wX;
			*wOutY = wY;
			return true;
		}

		// 양 쪽에 벽이 없음
		if (0 <= wY - 1 && this->_height >= wY + 2)
		{
			// 위 쪽 체크
			if (en_TILE_FIX_OBSTACLE == map[wY - 1][wX] && en_TILE_NONE == map[wY - 1][wX + 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
			// 아래 쪽 체크
			else if (en_TILE_FIX_OBSTACLE == map[wY + 1][wX] && en_TILE_NONE == map[wY + 1][wX + 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
		// 위 쪽이 벽
		else if (0 > wY - 1)
		{
			if (en_TILE_FIX_OBSTACLE == map[wY + 1][wX] && en_TILE_NONE == map[wY + 1][wX + 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
		// 아래 쪽이 벽
		else
		{
			if (en_TILE_FIX_OBSTACLE == map[wY - 1][wX] && en_TILE_NONE == map[wY - 1][wX + 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	함수 : Jump_DirDD(short wInX, short wInY, short *wOutX, short *wOutY)
//	인자 : 
//		short wInX		: 로직을 시작할 X좌표
//		short wInY		: 로직을 시작할 Y좌표
//		short *wOutX	: 목적지를 찾았거나 중간 노드 생성 위치를 찾으면 세팅한다.
//		short *wOutY	: 목적지를 찾았거나 중간 노드 생성 위치를 찾으면 세팅한다.
//
//	로직 실행할 노드의 위치에서 아랫 방향으로 검색하는 함수
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirDD(short wInX, short wInY, short *wOutX, short *wOutY)
{
	short wX = (short)wInX;
	short wY = (short)wInY;

	while (true)
	{
		wY++;

		if (!this->CheckRange(wX, wY))
			break;

		if (((this->_height - 1) < (wY + 1)) || (en_TILE_FIX_OBSTACLE == map[wY][wX]))
			break;

		if (this->CheckDestination(wX, wY))
		{
			*wOutX = wX;
			*wOutY = wY;
			return true;
		}

		// 양 쪽에 벽이 없음
		if (0 <= wX - 1 && this->_width >= wX + 2)
		{
			// 왼쪽 체크
			if (en_TILE_FIX_OBSTACLE == map[wY][wX - 1] && en_TILE_NONE == map[wY + 1][wX - 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
			// 오른쪽 체크
			else if (en_TILE_FIX_OBSTACLE == map[wY][wX + 1] && en_TILE_NONE == map[wY + 1][wX + 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
		// 왼쪽이 벽
		else if (0 > wX - 1)
		{
			if (en_TILE_FIX_OBSTACLE == map[wY][wX + 1] && en_TILE_NONE == map[wY + 1][wX + 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
		// 오른쪽이 벽
		else
		{
			if (en_TILE_FIX_OBSTACLE == map[wY][wX - 1] && en_TILE_NONE == map[wY + 1][wX - 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	함수 : Jump_DirLL(short wInX, short wInY, short *wOutX, short *wOutY)
//	인자 : 
//		short wInX		: 로직을 시작할 X좌표
//		short wInY		: 로직을 시작할 Y좌표
//		short *wOutX	: 목적지를 찾았거나 중간 노드 생성 위치를 찾으면 세팅한다.
//		short *wOutY	: 목적지를 찾았거나 중간 노드 생성 위치를 찾으면 세팅한다.
//
//	로직 실행할 노드의 위치에서 왼 방향으로 검색하는 함수
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirLL(short wInX, short wInY, short *wOutX, short *wOutY)
{
	short wX = (short)wInX;
	short wY = (short)wInY;

	while (true)
	{
		wX--;

		if (!this->CheckRange(wX, wY))
			break;

		if ((0 > (wX - 1)) || (en_TILE_FIX_OBSTACLE == map[wY][wX]))
			break;

		if (this->CheckDestination(wX, wY))
		{
			//this->CreateNode(pNode, wX, wY, en_DIR_NN);

			*wOutX = wX;
			*wOutY = wY;
			return true;
		}

		// 양 쪽에 벽이 없음
		if (0 <= wY - 1 && this->_height >= wY + 2)
		{
			// 위 쪽 체크
			if (en_TILE_FIX_OBSTACLE == map[wY - 1][wX] && en_TILE_NONE == map[wY - 1][wX - 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
			// 아래 쪽 체크
			else if (en_TILE_FIX_OBSTACLE == map[wY + 1][wX] && en_TILE_NONE == map[wY + 1][wX - 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
		// 위 쪽이 벽
		else if (0 > wY - 1)
		{
			if (en_TILE_FIX_OBSTACLE == map[wY + 1][wX] && en_TILE_NONE == map[wY + 1][wX - 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
		// 아래 쪽이 벽
		else
		{
			if (en_TILE_FIX_OBSTACLE == map[wY - 1][wX] && en_TILE_NONE == map[wY - 1][wX - 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	함수 : Jump_DirRU(short wInX, short wInY, short *wMiddleOutX, short *wMiddleOutY, short *wOutX, short *wOutY, BYTE *byOutDir)
//	인자 : 
//		short wInX			: 로직을 시작할 X좌표
//		short wInY			: 로직을 시작할 Y좌표
//		short wMiddleOutX	: 중간 노드 생성하기 전 경로 노드의 X좌표
//		short wMiddleOutY	: 중간 노드 생성하기 전 경로 노드의 Y좌표
//		WORD *wOutX			: 목적지를 찾았거나 중간 노드 생성 위치를 찾으면 세팅한다.
//		WORD *wOutY			: 목적지를 찾았거나 중간 노드 생성 위치를 찾으면 세팅한다.
//		BYTE *byOutDir		: 어느 방향 로직에서 찾았는지 세팅
//
//	로직 실행할 노드의 위치에서 오른쪽 윗 방향으로 검색하는 함수
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirRU(short wInX, short wInY, short *wMiddleOutX, short *wMiddleOutY, short *wOutX, short *wOutY, BYTE *byOutDir)
{
	short wX = (short)wInX;
	short wY = (short)wInY;

	while (true)
	{
		wX++;
		wY--;

		if (this->Jump_DirUU(wX, wY, wOutX, wOutY))
		{
			*wMiddleOutX = wX;
			*wMiddleOutY = wY;
			*byOutDir = en_DIR_UU;
			return true;
		}

		if (this->Jump_DirRR(wX, wY, wOutX, wOutY))
		{
			*wMiddleOutX = wX;
			*wMiddleOutY = wY;
			*byOutDir = en_DIR_RR;
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	함수 : Jump_DirRD(short wInX, short wInY, short *wMiddleOutX, short *wMiddleOutY, short *wOutX, short *wOutY, BYTE *byOutDir)
//	인자 : 
//		short wInX			: 로직을 시작할 X좌표
//		short wInY			: 로직을 시작할 Y좌표
//		short wMiddleOutX	: 중간 노드 생성하기 전 경로 노드의 X좌표
//		short wMiddleOutY	: 중간 노드 생성하기 전 경로 노드의 Y좌표
//		WORD *wOutX			: 목적지를 찾았거나 중간 노드 생성 위치를 찾으면 세팅한다.
//		WORD *wOutY			: 목적지를 찾았거나 중간 노드 생성 위치를 찾으면 세팅한다.
//		BYTE *byOutDir		: 어느 방향 로직에서 찾았는지 세팅
//
//	로직 실행할 노드의 위치에서 오른쪽 아랫 방향으로 검색하는 함수
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirRD(short wInX, short wInY, short *wMiddleOutX, short *wMiddleOutY, short *wOutX, short *wOutY, BYTE *byOutDir)
{
	short wX = (short)wInX;
	short wY = (short)wInY;

	while (true)
	{
		wX++;
		wY++;

		if (this->Jump_DirRR(wX, wY, wOutX, wOutY))
		{
			*wMiddleOutX = wX;
			*wMiddleOutY = wY;
			*byOutDir = en_DIR_RR;
			return true;
		}

		if (this->Jump_DirDD(wX, wY, wOutX, wOutY))
		{
			*wMiddleOutX = wX;
			*wMiddleOutY = wY;
			*byOutDir = en_DIR_DD;
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	함수 : Jump_DirLD(short wInX, short wInY, short *wMiddleOutX, short *wMiddleOutY, short *wOutX, short *wOutY, BYTE *byOutDir)
//	인자 : 
//		short wInX			: 로직을 시작할 X좌표
//		short wInY			: 로직을 시작할 Y좌표
//		short wMiddleOutX	: 중간 노드 생성하기 전 경로 노드의 X좌표
//		short wMiddleOutY	: 중간 노드 생성하기 전 경로 노드의 Y좌표
//		WORD *wOutX			: 목적지를 찾았거나 중간 노드 생성 위치를 찾으면 세팅한다.
//		WORD *wOutY			: 목적지를 찾았거나 중간 노드 생성 위치를 찾으면 세팅한다.
//		BYTE *byOutDir		: 어느 방향 로직에서 찾았는지 세팅
//
//	로직 실행할 노드의 위치에서 왼쪽 아랫 방향으로 검색하는 함수
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirLD(short wInX, short wInY, short *wMiddleOutX, short *wMiddleOutY, short *wOutX, short *wOutY, BYTE *byOutDir)
{
	short wX = (short)wInX;
	short wY = (short)wInY;

	while (true)
	{
		wX--;
		wY++;

		if (this->Jump_DirDD(wX, wY, wOutX, wOutY))
		{
			*wMiddleOutX = wX;
			*wMiddleOutY = wY;
			*byOutDir = en_DIR_DD;
			return true;
		}

		if (this->Jump_DirLL(wX, wY, wOutX, wOutY))
		{
			*wMiddleOutX = wX;
			*wMiddleOutY = wY;
			*byOutDir = en_DIR_LL;
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	함수 : Jump_DirLU(short wInX, short wInY, short *wMiddleOutX, short *wMiddleOutY, short *wOutX, short *wOutY, BYTE *byOutDir)
//	인자 : 
//		short wInX			: 로직을 시작할 X좌표
//		short wInY			: 로직을 시작할 Y좌표
//		short wMiddleOutX	: 중간 노드 생성하기 전 경로 노드의 X좌표
//		short wMiddleOutY	: 중간 노드 생성하기 전 경로 노드의 Y좌표
//		WORD *wOutX			: 목적지를 찾았거나 중간 노드 생성 위치를 찾으면 세팅한다.
//		WORD *wOutY			: 목적지를 찾았거나 중간 노드 생성 위치를 찾으면 세팅한다.
//		BYTE *byOutDir		: 어느 방향 로직에서 찾았는지 세팅
//
//	로직 실행할 노드의 위치에서 왼쪽 윗 방향으로 검색하는 함수
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirLU(short wInX, short wInY, short *wMiddleOutX, short *wMiddleOutY, short *wOutX, short *wOutY, BYTE *byOutDir)
{
	short wX = (short)wInX;
	short wY = (short)wInY;

	while (true)
	{
		wX--;
		wY--;

		if (this->Jump_DirLL(wX, wY, wOutX, wOutY))
		{
			*wMiddleOutX = wX;
			*wMiddleOutY = wY;
			*byOutDir = en_DIR_LL;
			return true;
		}

		if (this->Jump_DirUU(wX, wY, wOutX, wOutY))
		{
			*wMiddleOutX = wX;
			*wMiddleOutY = wY;
			*byOutDir = en_DIR_UU;
			return true;
		}
	}

	return false;
}

bool CJumpPointSearch::CheckRange(short shX, short shY)
{
	if ((0 > shX) || ((this->_width - 1) < shX) || (0 > shY) || ((this->_height - 1) < shY))
		return false;
	else
		return true;
}

//bool CJumpPointSearch::Compare(const NODE &a, const NODE &b)
//{
//	return a._iFitness < b._iFitness;
//}

void CJumpPointSearch::CompleteFind(NODE *pNode, PATH *pOut, int *iOutCount)
{
	NODE *pCurr = pNode;
	int iCnt = 0;

	std::stack<NODE *> stack;
	while (NULL != pCurr->pParent)
	{
		stack.push(pCurr);
		pCurr = pCurr->pParent;
	}

	for (iCnt; iCnt < dfPATH_POINT_MAX; ++iCnt)
	{
		if (stack.empty())
			break;

		pCurr = stack.top();

		pOut[iCnt].X = TILE_to_POS_X(pCurr->_wPosX);		// 이거 클라이언트 좌표로 변환해야함
		pOut[iCnt].Y = TILE_to_POS_Y(pCurr->_wPosY);		// 이거 클라이언트 좌표로 변환해야함

		stack.pop();
	}

	// TEST할 땐 이상 없었는데??????
	*iOutCount = iCnt;

	auto iter = _openList.begin();
	for (iter; iter != _openList.end(); ++iter)
		this->_nodePool.Free((*iter));

	for (iter = _closeList.begin(); iter != _closeList.end(); ++iter)
		this->_nodePool.Free((*iter));

	this->_openList.clear();
	this->_closeList.clear();
	return;
}

bool Compare(CJumpPointSearch::NODE *a, CJumpPointSearch::NODE *b)
{
	return a->_iFitness < b->_iFitness;
}