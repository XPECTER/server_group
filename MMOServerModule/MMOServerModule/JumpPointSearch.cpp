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
	bool bResult = false;

	this->_wStartX = wStartX;
	this->_wStartY = wStartY;
	this->_wEndX = wEndX;
	this->_wEndY = wEndY;

	// ������ ���
	this->CreateNode(NULL, wStartX, wStartY, en_DIR_NN);

	while (this->_openList.size())
	{
		auto iter = this->_openList.begin();

		if (this->CheckDestination((*iter)->_wPosX, (*iter)->_wPosY))
		{
			// Path ����ü ��Ƽ� �Ѱ����
			this->CompleteFind((*iter), pOut, iOutCount);
		}

		switch ((*iter)->_byDir)
		{
			case en_DIR_NN:
			{
				// ���� �����ϴ� ������ ��ȹ�� ���� �޶��� �� ����
				// ������ ��������(�� -> ���� -> �Ʒ� -> ��) �����ϰ� 
				// �밢�� ����(�� �� -> ���� �� -> ���� �Ʒ� -> �� �Ʒ�)

				if (this->Jump_DirUU(*iter))
					break;

				if (this->Jump_DirRR(*iter))
					break;

				if (this->Jump_DirDD(*iter))
					break;

				if (this->Jump_DirLL(*iter))
					break;


				break;
			}

			case en_DIR_UU:
			{
				if (this->Jump_DirUU(*iter))
					break;
				break;
			}

			case en_DIR_RR:
			{
				break;
			}

			case en_DIR_DD:
			{
				break;
			}

			case en_DIR_LL:
			{
				break;
			}

			case en_DIR_RU:
			{
				break;
			}

			case en_DIR_RD:
			{
				break;
			}

			case en_DIR_LD:
			{
				break;
			}

			case en_DIR_LU:
			{
				break;
			}
		}

		// ������ �Ǿ��ٴ� ����
		this->_closeList.push_back((*iter));
		this->_openList.pop_front();
	}

	return false;
}

void CJumpPointSearch::CreateNode(NODE *pParents, WORD wPosX, WORD wPosY, BYTE byDir)
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
	return;
}

bool CJumpPointSearch::CheckDestination(WORD wX, WORD wY)
{
	if (this->_wEndX == wX && this->_wEndY == wY)
		return true;
	else
		return false;
}

bool CJumpPointSearch::Jump(NODE *pNode, BYTE byDir)
{
	WORD wX = pNode->_wPosX;
	WORD wY = pNode->_wPosY;

	while (true)
	{

	}
}


bool CJumpPointSearch::Jump_DirUU(NODE *pNode)
{
	short wX = (short)pNode->_wPosX;
	short wY = (short)pNode->_wPosY;

	while (true)
	{
		wY--;

		if ((0 > (wY - 1)) || (en_TILE_FIX_OBSTACLE == map[wY][wX]))
			break;

		if (this->CheckDestination(wX, wY))
		{
			this->CreateNode(pNode, wX, wY, en_DIR_NN);
			return true;
		}

		// �� �ʿ� ���� ����
		if (0 <= wX - 1 && this->_width >= wX + 2)
		{
			// ���� üũ
			if (en_TILE_FIX_OBSTACLE == map[wY][wX - 1] && en_TILE_NONE == map[wY - 1][wX - 1])
			{
				this->CreateNode(pNode, wX, wY, en_DIR_UU);
				return true;
			}
			// ������ üũ
			else if (en_TILE_FIX_OBSTACLE == map[wY][wX + 1] && en_TILE_NONE == map[wY - 1][wX + 1])
			{
				this->CreateNode(pNode, wX, wY, en_DIR_UU);
				return true;
			}
		}
		// ������ ��
		else if (0 > wX - 1)
		{
			if (en_TILE_FIX_OBSTACLE == map[wY][wX + 1] && en_TILE_NONE == map[wY - 1][wX + 1])
			{
				this->CreateNode(pNode, wX, wY, en_DIR_UU);
				return true;
			}
		}
		// �������� ��
		else
		{
			if (en_TILE_FIX_OBSTACLE == map[wY][wX - 1] && en_TILE_NONE == map[wY - 1][wX - 1])
			{
				this->CreateNode(pNode, wX, wY, en_DIR_UU);
				return true;
			}
		}
	}

	return false;
}

bool CJumpPointSearch::Jump_DirRR(NODE *pNode)
{
	short wX = (short)pNode->_wPosX;
	short wY = (short)pNode->_wPosY;

	while (true)
	{
		wX++;

		if (((this->_width - 1) < (wX + 1)) || en_TILE_FIX_OBSTACLE == map[wY][wX])
			break;

		if (this->CheckDestination(wX, wY))
		{
			this->CreateNode(pNode, wX, wY, en_DIR_NN);
			return true;
		}

		// �� �ʿ� ���� ����
		if (0 <= wY - 1 && this->_height >= wY + 2)
		{
			// �� �� üũ
			if (en_TILE_FIX_OBSTACLE == map[wY - 1][wX] && en_TILE_NONE == map[wY - 1][wX + 1])
			{
				this->CreateNode(pNode, wX, wY, en_DIR_RR);
				return true;
			}
			// �Ʒ� �� üũ
			else if (en_TILE_FIX_OBSTACLE == map[wY + 1][wX] && en_TILE_NONE == map[wY + 1][wX + 1])
			{
				this->CreateNode(pNode, wX, wY, en_DIR_RR);
				return true;
			}
		}
		// �� ���� ��
		else if (0 > wY - 1)
		{
			if (en_TILE_FIX_OBSTACLE == map[wY + 1][wX] && en_TILE_NONE == map[wY + 1][wX + 1])
			{
				this->CreateNode(pNode, wX, wY, en_DIR_RR);
				return true;
			}
		}
		// �Ʒ� ���� ��
		else
		{
			if (en_TILE_FIX_OBSTACLE == map[wY - 1][wX] && en_TILE_NONE == map[wY - 1][wX + 1])
			{
				this->CreateNode(pNode, wX, wY, en_DIR_RR);
				return true;
			}
		}
	}

	return false;
}

bool CJumpPointSearch::Jump_DirDD(NODE *pNode)
{
	short wX = (short)pNode->_wPosX;
	short wY = (short)pNode->_wPosY;

	while (true)
	{
		wY++;

		if (((this->_height - 1) < (wY + 1)) || (en_TILE_FIX_OBSTACLE == map[wY][wX]))
			break;

		if (this->CheckDestination(wX, wY))
		{
			this->CreateNode(pNode, wX, wY, en_DIR_NN);
			return true;
		}

		// �� �ʿ� ���� ����
		if (0 <= wX - 1 && this->_width >= wX + 2)
		{
			// ���� üũ
			if (en_TILE_FIX_OBSTACLE == map[wY][wX - 1] && en_TILE_NONE == map[wY + 1][wX - 1])
			{
				this->CreateNode(pNode, wX, wY, en_DIR_DD);
				return true;
			}
			// ������ üũ
			else if (en_TILE_FIX_OBSTACLE == map[wY][wX + 1] && en_TILE_NONE == map[wY + 1][wX + 1])
			{
				this->CreateNode(pNode, wX, wY, en_DIR_DD);
				return true;
			}
		}
		// ������ ��
		else if (0 > wX - 1)
		{
			if (en_TILE_FIX_OBSTACLE == map[wY][wX + 1] && en_TILE_NONE == map[wY + 1][wX + 1])
			{
				this->CreateNode(pNode, wX, wY, en_DIR_DD);
				return true;
			}
		}
		// �������� ��
		else
		{
			if (en_TILE_FIX_OBSTACLE == map[wY][wX - 1] && en_TILE_NONE == map[wY + 1][wX - 1])
			{
				this->CreateNode(pNode, wX, wY, en_DIR_DD);
				return true;
			}
		}
	}

	return false;
}

bool CJumpPointSearch::Jump_DirLL(NODE *pNode)
{
	short wX = (short)pNode->_wPosX;
	short wY = (short)pNode->_wPosY;

	while (true)
	{
		wX--;

		if ((0 > (wX - 1)) || (en_TILE_FIX_OBSTACLE == map[wY][wX]))
			break;

		if (this->CheckDestination(wX, wY))
		{
			this->CreateNode(pNode, wX, wY, en_DIR_NN);
			return true;
		}

		// �� �ʿ� ���� ����
		if (0 <= wY - 1 && this->_height >= wY + 2)
		{
			// �� �� üũ
			if (en_TILE_FIX_OBSTACLE == map[wY - 1][wX] && en_TILE_NONE == map[wY - 1][wX - 1])
			{
				this->CreateNode(pNode, wX, wY, en_DIR_LL);
				return true;
			}
			// �Ʒ� �� üũ
			else if (en_TILE_FIX_OBSTACLE == map[wY + 1][wX] && en_TILE_NONE == map[wY + 1][wX - 1])
			{
				this->CreateNode(pNode, wX, wY, en_DIR_LL);
				return true;
			}
		}
		// �� ���� ��
		else if (0 > wY - 1)
		{
			if (en_TILE_FIX_OBSTACLE == map[wY + 1][wX] && en_TILE_NONE == map[wY + 1][wX - 1])
			{
				this->CreateNode(pNode, wX, wY, en_DIR_LL);
				return true;
			}
		}
		// �Ʒ� ���� ��
		else
		{
			if (en_TILE_FIX_OBSTACLE == map[wY - 1][wX] && en_TILE_NONE == map[wY - 1][wX - 1])
			{
				this->CreateNode(pNode, wX, wY, en_DIR_LL);
				return true;
			}
		}
	}

	return false;
}

void CJumpPointSearch::Jump_DirRU(NODE *pNode)
{
	short wX = (short)pNode->_wPosX;
	short wY = (short)pNode->_wPosY;

	while (true)
	{
		wX++;
		wY--;

		if (wX )
	}
}
void CJumpPointSearch::Jump_DirRD(NODE *pNode, WORD wX, WORD wY)
{

}
void CJumpPointSearch::Jump_DirLD(NODE *pNode, WORD wX, WORD wY)
{

}
void CJumpPointSearch::Jump_DirLU(NODE *pNode, WORD wX, WORD wY)
{
	
}

void CJumpPointSearch::CompleteFind(NODE *pNode, PATH *pOut, int *iOutCount)
{
	NODE *pCurr = pNode;
	int iCnt = 0;

	for (int iCnt = 0; iCnt < dfPATH_POINT_MAX; ++iCnt)
	{
		if (NULL == pCurr->pParent)
			break;

		pOut[iCnt].X = TILE_to_POS_X(pCurr->_wPosX);		// �̰� Ŭ���̾�Ʈ ��ǥ�� ��ȯ�ؾ���
		pOut[iCnt].Y = TILE_to_POS_Y(pCurr->_wPosY);		// �̰� Ŭ���̾�Ʈ ��ǥ�� ��ȯ�ؾ���

		iCnt++;
	}

	*iOutCount = iCnt;
}