#include "stdafx.h"
#include "JumpPointSearch.h"

// ������ ����� �� ũ��� �޴´�.
// Ŭ���̾�Ʈ�� 200 * 100�� ����ϰ� ������ 400 * 200�� ����Ѵ�.
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

	fseek(pFile, 3, SEEK_SET);	// BOM �ڵ� ����
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

	// �������� Ŭ���̾�Ʈ�� �� ũ���� X2 ��ŭ ����ϱ� ������ �� ���� �����ؾ��Ѵ�.
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

	// ������ ���
	this->CreateNode(NULL, wStartX, wStartY, en_DIR_NN);

	for (int i = 0; i < dfPATH_OPEN_MAX; ++i)
	{
		// ������ �Ǿ��ִٴ� ����
		auto iter = this->_openList.begin();
		NODE *pNode = (*iter);

		if (this->CheckDestination((*iter)->_wPosX, (*iter)->_wPosY))
		{
			// Path ����ü ��Ƽ� �Ѱ����
			this->CompleteFind((*iter), pOut, iOutCount);
			return true;
		}

		this->Jump(pNode, pNode->_byDir);

		// closeList�� �ִ°� �׳� �־ ��
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
			pNewNode->_iGoal = (abs(pParents->_wPosY - pNewNode->_wPosY) + abs(pParents->_wPosX - pNewNode->_wPosX)) * 14;	// �밢�� ����ġ 1.4
	}
	else
	{
		pNewNode->_iGoal = 0;
	}

	pNewNode->_iHeuristic = (abs(this->_wEndX - pNewNode->_wPosX) + abs(this->_wEndY - pNewNode->_wPosY)) * 10;
	pNewNode->_iFitness = pNewNode->_iGoal + pNewNode->_iHeuristic;

	this->_openList.push_back(pNewNode);

	// Fitness ���� ���� ������ ����� �Ѵ�.
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
//	�Լ� : Jump_DirUU(short wInX, short wInY, short *wOutX, short *wOutY)
//	���� : 
//		short wInX		: ������ ������ X��ǥ
//		short wInY		: ������ ������ Y��ǥ
//		short *wOutX	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		short *wOutY	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//
//	���� ������ ����� ��ġ���� �� �������� �˻��ϴ� �Լ�
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

		// �� �ʿ� ���� ����
		if (0 <= wX - 1 && this->_width >= wX + 2)
		{
			// ���� üũ
			if (en_TILE_FIX_OBSTACLE == map[wY][wX - 1] && en_TILE_NONE == map[wY - 1][wX - 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
			// ������ üũ
			else if (en_TILE_FIX_OBSTACLE == map[wY][wX + 1] && en_TILE_NONE == map[wY - 1][wX + 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
		// ������ ��
		else if (0 > wX - 1)
		{
			if (en_TILE_FIX_OBSTACLE == map[wY][wX + 1] && en_TILE_NONE == map[wY - 1][wX + 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
		// �������� ��
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
//	�Լ� : Jump_DirRR(short wInX, short wInY, short *wOutX, short *wOutY)
//	���� : 
//		short wInX		: ������ ������ X��ǥ
//		short wInY		: ������ ������ Y��ǥ
//		short *wOutX	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		short *wOutY	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//
//	���� ������ ����� ��ġ���� ���� �������� �˻��ϴ� �Լ�
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

		// �� �ʿ� ���� ����
		if (0 <= wY - 1 && this->_height >= wY + 2)
		{
			// �� �� üũ
			if (en_TILE_FIX_OBSTACLE == map[wY - 1][wX] && en_TILE_NONE == map[wY - 1][wX + 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
			// �Ʒ� �� üũ
			else if (en_TILE_FIX_OBSTACLE == map[wY + 1][wX] && en_TILE_NONE == map[wY + 1][wX + 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
		// �� ���� ��
		else if (0 > wY - 1)
		{
			if (en_TILE_FIX_OBSTACLE == map[wY + 1][wX] && en_TILE_NONE == map[wY + 1][wX + 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
		// �Ʒ� ���� ��
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
//	�Լ� : Jump_DirDD(short wInX, short wInY, short *wOutX, short *wOutY)
//	���� : 
//		short wInX		: ������ ������ X��ǥ
//		short wInY		: ������ ������ Y��ǥ
//		short *wOutX	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		short *wOutY	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//
//	���� ������ ����� ��ġ���� �Ʒ� �������� �˻��ϴ� �Լ�
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

		// �� �ʿ� ���� ����
		if (0 <= wX - 1 && this->_width >= wX + 2)
		{
			// ���� üũ
			if (en_TILE_FIX_OBSTACLE == map[wY][wX - 1] && en_TILE_NONE == map[wY + 1][wX - 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
			// ������ üũ
			else if (en_TILE_FIX_OBSTACLE == map[wY][wX + 1] && en_TILE_NONE == map[wY + 1][wX + 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
		// ������ ��
		else if (0 > wX - 1)
		{
			if (en_TILE_FIX_OBSTACLE == map[wY][wX + 1] && en_TILE_NONE == map[wY + 1][wX + 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
		// �������� ��
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
//	�Լ� : Jump_DirLL(short wInX, short wInY, short *wOutX, short *wOutY)
//	���� : 
//		short wInX		: ������ ������ X��ǥ
//		short wInY		: ������ ������ Y��ǥ
//		short *wOutX	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		short *wOutY	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//
//	���� ������ ����� ��ġ���� �� �������� �˻��ϴ� �Լ�
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

		// �� �ʿ� ���� ����
		if (0 <= wY - 1 && this->_height >= wY + 2)
		{
			// �� �� üũ
			if (en_TILE_FIX_OBSTACLE == map[wY - 1][wX] && en_TILE_NONE == map[wY - 1][wX - 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
			// �Ʒ� �� üũ
			else if (en_TILE_FIX_OBSTACLE == map[wY + 1][wX] && en_TILE_NONE == map[wY + 1][wX - 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
		// �� ���� ��
		else if (0 > wY - 1)
		{
			if (en_TILE_FIX_OBSTACLE == map[wY + 1][wX] && en_TILE_NONE == map[wY + 1][wX - 1])
			{
				*wOutX = wX;
				*wOutY = wY;
				return true;
			}
		}
		// �Ʒ� ���� ��
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
//	�Լ� : Jump_DirRU(short wInX, short wInY, short *wMiddleOutX, short *wMiddleOutY, short *wOutX, short *wOutY, BYTE *byOutDir)
//	���� : 
//		short wInX			: ������ ������ X��ǥ
//		short wInY			: ������ ������ Y��ǥ
//		short wMiddleOutX	: �߰� ��� �����ϱ� �� ��� ����� X��ǥ
//		short wMiddleOutY	: �߰� ��� �����ϱ� �� ��� ����� Y��ǥ
//		WORD *wOutX			: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		WORD *wOutY			: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		BYTE *byOutDir		: ��� ���� �������� ã�Ҵ��� ����
//
//	���� ������ ����� ��ġ���� ������ �� �������� �˻��ϴ� �Լ�
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
//	�Լ� : Jump_DirRD(short wInX, short wInY, short *wMiddleOutX, short *wMiddleOutY, short *wOutX, short *wOutY, BYTE *byOutDir)
//	���� : 
//		short wInX			: ������ ������ X��ǥ
//		short wInY			: ������ ������ Y��ǥ
//		short wMiddleOutX	: �߰� ��� �����ϱ� �� ��� ����� X��ǥ
//		short wMiddleOutY	: �߰� ��� �����ϱ� �� ��� ����� Y��ǥ
//		WORD *wOutX			: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		WORD *wOutY			: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		BYTE *byOutDir		: ��� ���� �������� ã�Ҵ��� ����
//
//	���� ������ ����� ��ġ���� ������ �Ʒ� �������� �˻��ϴ� �Լ�
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
//	�Լ� : Jump_DirLD(short wInX, short wInY, short *wMiddleOutX, short *wMiddleOutY, short *wOutX, short *wOutY, BYTE *byOutDir)
//	���� : 
//		short wInX			: ������ ������ X��ǥ
//		short wInY			: ������ ������ Y��ǥ
//		short wMiddleOutX	: �߰� ��� �����ϱ� �� ��� ����� X��ǥ
//		short wMiddleOutY	: �߰� ��� �����ϱ� �� ��� ����� Y��ǥ
//		WORD *wOutX			: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		WORD *wOutY			: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		BYTE *byOutDir		: ��� ���� �������� ã�Ҵ��� ����
//
//	���� ������ ����� ��ġ���� ���� �Ʒ� �������� �˻��ϴ� �Լ�
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
//	�Լ� : Jump_DirLU(short wInX, short wInY, short *wMiddleOutX, short *wMiddleOutY, short *wOutX, short *wOutY, BYTE *byOutDir)
//	���� : 
//		short wInX			: ������ ������ X��ǥ
//		short wInY			: ������ ������ Y��ǥ
//		short wMiddleOutX	: �߰� ��� �����ϱ� �� ��� ����� X��ǥ
//		short wMiddleOutY	: �߰� ��� �����ϱ� �� ��� ����� Y��ǥ
//		WORD *wOutX			: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		WORD *wOutY			: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		BYTE *byOutDir		: ��� ���� �������� ã�Ҵ��� ����
//
//	���� ������ ����� ��ġ���� ���� �� �������� �˻��ϴ� �Լ�
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

		pOut[iCnt].X = TILE_to_POS_X(pCurr->_wPosX);		// �̰� Ŭ���̾�Ʈ ��ǥ�� ��ȯ�ؾ���
		pOut[iCnt].Y = TILE_to_POS_Y(pCurr->_wPosY);		// �̰� Ŭ���̾�Ʈ ��ǥ�� ��ȯ�ؾ���

		stack.pop();
	}

	// TEST�� �� �̻� �����µ�??????
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