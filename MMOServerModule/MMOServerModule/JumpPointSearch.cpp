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

bool CJumpPointSearch::LoadTextMap(wchar_t *szFileName, char *obstacle)
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
			if (NULL != strstr(token, obstacle))
			{
				SetTileProperty(iWidthCnt, iHeightCnt, en_TILE_FIX_OBSTACLE);
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

	int iHeightLoop = this->_height / 2;
	int iWidthLoog = this->_width / 2;

	for (int iHeightCnt = 0; iHeightCnt < iHeightLoop; ++iHeightCnt)
		for (int iWidthCnt = 0; iWidthCnt < iWidthLoog; ++iWidthCnt)
			this->SetTileProperty(iWidthCnt, iHeightCnt, en_TILE_NONE);
}

void CJumpPointSearch::SetTileProperty(int iX, int iY, en_TILE_PROPERTY type)
{
	if (NULL == this->map)
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Map don`t create yet");
		return;
	}
	
	int iServerX = iX * 2;
	int iServerY = iY * 2;

	// �������� Ŭ���̾�Ʈ�� �� ũ���� X2 ��ŭ ����ϱ� ������ �� ���� �����ؾ��Ѵ�.
	this->map[iServerY][iServerX] = type;
	this->map[iServerY][iServerX + 1] = type;
	this->map[iServerY + 1][iServerX] = type;
	this->map[iServerY + 1][iServerX + 1] = type;

	return;
}

bool CJumpPointSearch::FindPath(int iStartX, int iStartY, int iEndX, int iEndY, PATH *pOut, int *iOutCount)
{
	if (NULL == this->map)
	{
		return false;
	}

	if (en_TILE_FIX_OBSTACLE == this->map[iStartY][iStartX])
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_DEBUG, L" [JPS] wrong start position");
		return false;
	}

	if (en_TILE_FIX_OBSTACLE == this->map[iEndY][iEndX])
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_DEBUG, L" [JPS] wrong end position");
		return false;
	}

	bool bResult = false;

	this->_iStartX = iStartX;
	this->_iStartY = iStartY;
	this->_iEndX = iEndX;
	this->_iEndY = iEndY;

	// ������ ���
	this->CreateNode(NULL, iStartX, iStartY, en_DIR_NN);

	for (int i = 0; i < dfPATH_OPEN_MAX; ++i)
	{
		// ������ �Ǿ��ִٴ� ����
		auto iter = this->_openList.begin();
		NODE *pNode = (*iter);

		if (this->CheckDestination((*iter)->_iPosX, (*iter)->_iPosY))
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

	*iOutCount = 0;
	return false;
}

CJumpPointSearch::NODE* CJumpPointSearch::CreateNode(NODE *pParents, int iPosX, int iPosY, BYTE byDir)
{
	NODE *pNewNode = this->_nodePool.Alloc();
	pNewNode->pParent = pParents;
	pNewNode->_byDir = byDir;
	pNewNode->_iPosX = iPosX;
	pNewNode->_iPosY = iPosY;

	if (NULL != pParents)
	{
		if (pParents->_iPosX == pNewNode->_iPosX || pParents->_iPosY == pNewNode->_iPosY)
			pNewNode->_iGoal = (abs(pParents->_iPosY - pNewNode->_iPosY) + abs(pParents->_iPosX - pNewNode->_iPosX)) * 10;
		else
			pNewNode->_iGoal = (abs(pParents->_iPosY - pNewNode->_iPosY) + abs(pParents->_iPosX - pNewNode->_iPosX)) * 14;	// �밢�� ����ġ 1.4
	}
	else
	{
		pNewNode->_iGoal = 0;
	}

	pNewNode->_iHeuristic = (abs(this->_iEndX - pNewNode->_iPosX) + abs(this->_iEndY - pNewNode->_iPosY)) * 10;
	pNewNode->_iFitness = pNewNode->_iGoal + pNewNode->_iHeuristic;

	this->_openList.push_back(pNewNode);

	// Fitness ���� ���� ������ ����� �Ѵ�.
	this->_openList.sort(Compare);
	return pNewNode;
}

bool CJumpPointSearch::CheckDestination(int iX, int iY)
{
	if (this->_iEndX == iX && this->_iEndY == iY)
		return true;
	else
		return false;
}

void CJumpPointSearch::Jump(NODE *pNode, BYTE byDir)
{
	int iOutX = -1;
	int iOutY = -1;
	int iMiddleX = -1;
	int iMiddleY = -1;
	NODE *pMiddleNode = NULL;
	BYTE byOutDir;

	switch (byDir)
	{
		case en_DIR_NN:
		{
			if (this->Jump_DirUU(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
			{
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_UU);
			}

			if (this->Jump_DirRR(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
			{
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_RR);
			}

			if (this->Jump_DirDD(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
			{
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_DD);
			}

			if (this->Jump_DirLL(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
			{
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_LL);
			}

			if (this->Jump_DirRU(pNode->_iPosX, pNode->_iPosY, &iMiddleX, &iMiddleY, &iOutX, &iOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, iMiddleX, iMiddleY, en_DIR_RU);

				if ((-1) != iOutX)
					this->CreateNode(pMiddleNode, iOutX, iOutY, byOutDir);
			}

			if (this->Jump_DirRD(pNode->_iPosX, pNode->_iPosY, &iMiddleX, &iMiddleY, &iOutX, &iOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, iMiddleX, iMiddleY, en_DIR_RD);

				if ((-1) != iOutX)
					this->CreateNode(pMiddleNode, iOutX, iOutY, byOutDir);
			}

			if (this->Jump_DirLD(pNode->_iPosX, pNode->_iPosY, &iMiddleX, &iMiddleY, &iOutX, &iOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, iMiddleX, iMiddleY, en_DIR_LD);

				if ((-1) != iOutX)
					this->CreateNode(pMiddleNode, iOutX, iOutY, byOutDir);
			}

			if (this->Jump_DirLU(pNode->_iPosX, pNode->_iPosY, &iMiddleX, &iMiddleY, &iOutX, &iOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, iMiddleX, iMiddleY, en_DIR_LU);

				if ((-1) != iOutX)
					this->CreateNode(pMiddleNode, iOutX, iOutY, byOutDir);
			}

			break;
		}

		case en_DIR_UU:
		{
			if (this->Jump_DirUU(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
			{
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_UU);
			}

			if (this->Jump_DirLU(pNode->_iPosX, pNode->_iPosY, &iMiddleX, &iMiddleY, &iOutX, &iOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, iMiddleX, iMiddleY, en_DIR_LU);

				if ((-1) != iOutX)
					this->CreateNode(pMiddleNode, iOutX, iOutY, byOutDir);
			}

			if (this->Jump_DirRU(pNode->_iPosX, pNode->_iPosY, &iMiddleX, &iMiddleY, &iOutX, &iOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, iMiddleX, iMiddleY, en_DIR_RU);

				if ((-1) != iOutX)
					this->CreateNode(pMiddleNode, iOutX, iOutY, byOutDir);
			}

			break;
		}

		case en_DIR_RR:
		{
			if (this->Jump_DirRR(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
			{
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_RR);
			}

			if (this->Jump_DirRU(pNode->_iPosX, pNode->_iPosY, &iMiddleX, &iMiddleY, &iOutX, &iOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, iMiddleX, iMiddleY, en_DIR_RU);

				if ((-1) != iOutX)
					this->CreateNode(pMiddleNode, iOutX, iOutY, byOutDir);
			}

			if (this->Jump_DirRD(pNode->_iPosX, pNode->_iPosY, &iMiddleX, &iMiddleY, &iOutX, &iOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, iMiddleX, iMiddleY, en_DIR_RD);

				if ((-1) != iOutX)
					this->CreateNode(pMiddleNode, iOutX, iOutY, byOutDir);
			}

			break;
		}

		case en_DIR_DD:
		{
			if (this->Jump_DirDD(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
			{
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_DD);
			}

			if (this->Jump_DirRD(pNode->_iPosX, pNode->_iPosY, &iMiddleX, &iMiddleY, &iOutX, &iOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, iMiddleX, iMiddleY, en_DIR_RD);

				if ((-1) != iOutX)
					this->CreateNode(pMiddleNode, iOutX, iOutY, byOutDir);
			}

			if (this->Jump_DirLD(pNode->_iPosX, pNode->_iPosY, &iMiddleX, &iMiddleY, &iOutX, &iOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, iMiddleX, iMiddleY, en_DIR_LD);
				
				if ((-1) != iOutX)
					this->CreateNode(pMiddleNode, iOutX, iOutY, byOutDir);
			}

			break;
		}

		case en_DIR_LL:
		{
			if (this->Jump_DirLL(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
			{
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_LL);
			}

			if (this->Jump_DirLD(pNode->_iPosX, pNode->_iPosY, &iMiddleX, &iMiddleY, &iOutX, &iOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, iMiddleX, iMiddleY, en_DIR_LD);

				if ((-1) != iOutX)
					this->CreateNode(pMiddleNode, iOutX, iOutY, byOutDir);
			}

			if (this->Jump_DirLU(pNode->_iPosX, pNode->_iPosY, &iMiddleX, &iMiddleY, &iOutX, &iOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, iMiddleX, iMiddleY, en_DIR_LU);

				if ((-1) != iOutX)
					this->CreateNode(pMiddleNode, iOutX, iOutY, byOutDir);
			}

			break;
		}

		case en_DIR_RU:
		{
			if (this->Jump_DirRU(pNode->_iPosX, pNode->_iPosY, &iMiddleX, &iMiddleY, &iOutX, &iOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, iMiddleX, iMiddleY, en_DIR_RU);

				if ((-1) != iOutX)
					this->CreateNode(pMiddleNode, iOutX, iOutY, byOutDir);
			}

			break;
		}

		case en_DIR_RD:
		{
			if (this->Jump_DirRD(pNode->_iPosX, pNode->_iPosY, &iMiddleX, &iMiddleY, &iOutX, &iOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, iMiddleX, iMiddleY, en_DIR_RD);

				if ((-1) != iOutX)
					this->CreateNode(pMiddleNode, iOutX, iOutY, byOutDir);
			}

			break;
		}

		case en_DIR_LD:
		{
			if (this->Jump_DirLD(pNode->_iPosX, pNode->_iPosY, &iMiddleX, &iMiddleY, &iOutX, &iOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, iMiddleX, iMiddleY, en_DIR_LD);

				if ((-1) != iOutX)
					this->CreateNode(pMiddleNode, iOutX, iOutY, byOutDir);
			}

			break;
		}

		case en_DIR_LU:
		{
			if (this->Jump_DirLU(pNode->_iPosX, pNode->_iPosY, &iMiddleX, &iMiddleY, &iOutX, &iOutY, &byOutDir))
			{
				pMiddleNode = this->CreateNode(pNode, iMiddleX, iMiddleY, en_DIR_LU);

				if ((-1) != iOutX)
					this->CreateNode(pMiddleNode, iOutX, iOutY, byOutDir);
			}

			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////
//
//	�Լ� : Jump_DirUU(short iInX, short iInY, short *iOutX, short *iOutY)
//	���� : 
//		short iInX		: ������ ������ X��ǥ
//		short iInY		: ������ ������ Y��ǥ
//		short *iOutX	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		short *iOutY	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//
//	���� ������ ����� ��ġ���� �� �������� �˻��ϴ� �Լ�
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirUU(int iInX, int iInY, int *iOutX, int *iOutY)
{
	int iX = iInX;
	int iY = iInY;

	while (true)
	{
		iY--;

		if (!this->CheckRange(iX, iY))
			break;

		if ((0 > (iY - 1)) || (en_TILE_FIX_OBSTACLE == map[iY][iX]))
			break;

		if (this->CheckDestination(iX, iY))
		{
			*iOutX = iX;
			*iOutY = iY;
			return true;
		}

		// �� �ʿ� ���� ����
		if (0 <= iX - 1 && this->_width >= iX + 2)
		{
			// ���� üũ
			if (en_TILE_FIX_OBSTACLE == map[iY][iX - 1] && en_TILE_NONE == map[iY - 1][iX - 1])
			{
				*iOutX = iX;
				*iOutY = iY;
				return true;
			}
			// ������ üũ
			else if (en_TILE_FIX_OBSTACLE == map[iY][iX + 1] && en_TILE_NONE == map[iY - 1][iX + 1])
			{
				*iOutX = iX;
				*iOutY = iY;
				return true;
			}
		}
		// ������ ��
		else if (0 > iX - 1)
		{
			if (en_TILE_FIX_OBSTACLE == map[iY][iX + 1] && en_TILE_NONE == map[iY - 1][iX + 1])
			{
				*iOutX = iX;
				*iOutY = iY;
				return true;
			}
		}
		// �������� ��
		else
		{
			if (en_TILE_FIX_OBSTACLE == map[iY][iX - 1] && en_TILE_NONE == map[iY - 1][iX - 1])
			{
				*iOutX = iX;
				*iOutY = iY;
				return true;
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	�Լ� : Jump_DirRR(short iInX, short iInY, short *iOutX, short *iOutY)
//	���� : 
//		short iInX		: ������ ������ X��ǥ
//		short iInY		: ������ ������ Y��ǥ
//		short *iOutX	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		short *iOutY	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//
//	���� ������ ����� ��ġ���� ���� �������� �˻��ϴ� �Լ�
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirRR(int iInX, int iInY, int *iOutX, int *iOutY)
{
	int iX = iInX;
	int iY = iInY;

	while (true)
	{
		iX++;

		if (!this->CheckRange(iX, iY))
			break;

		if (((this->_width - 1) < (iX + 1)) || en_TILE_FIX_OBSTACLE == map[iY][iX])
			break;

		if (this->CheckDestination(iX, iY))
		{
			*iOutX = iX;
			*iOutY = iY;
			return true;
		}

		// �� �ʿ� ���� ����
		if (0 <= iY - 1 && this->_height >= iY + 2)
		{
			// �� �� üũ
			if (en_TILE_FIX_OBSTACLE == map[iY - 1][iX] && en_TILE_NONE == map[iY - 1][iX + 1])
			{
				*iOutX = iX;
				*iOutY = iY;
				return true;
			}
			// �Ʒ� �� üũ
			else if (en_TILE_FIX_OBSTACLE == map[iY + 1][iX] && en_TILE_NONE == map[iY + 1][iX + 1])
			{
				*iOutX = iX;
				*iOutY = iY;
				return true;
			}
		}
		// �� ���� ��
		else if (0 > iY - 1)
		{
			if (en_TILE_FIX_OBSTACLE == map[iY + 1][iX] && en_TILE_NONE == map[iY + 1][iX + 1])
			{
				*iOutX = iX;
				*iOutY = iY;
				return true;
			}
		}
		// �Ʒ� ���� ��
		else
		{
			if (en_TILE_FIX_OBSTACLE == map[iY - 1][iX] && en_TILE_NONE == map[iY - 1][iX + 1])
			{
				*iOutX = iX;
				*iOutY = iY;
				return true;
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	�Լ� : Jump_DirDD(short iInX, short iInY, short *iOutX, short *iOutY)
//	���� : 
//		short iInX		: ������ ������ X��ǥ
//		short iInY		: ������ ������ Y��ǥ
//		short *iOutX	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		short *iOutY	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//
//	���� ������ ����� ��ġ���� �Ʒ� �������� �˻��ϴ� �Լ�
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirDD(int iInX, int iInY, int *iOutX, int *iOutY)
{
	int iX = iInX;
	int iY = iInY;

	while (true)
	{
		iY++;

		if (!this->CheckRange(iX, iY))
			break;

		if (((this->_height - 1) < (iY + 1)) || (en_TILE_FIX_OBSTACLE == map[iY][iX]))
			break;

		if (this->CheckDestination(iX, iY))
		{
			*iOutX = iX;
			*iOutY = iY;
			return true;
		}

		// �� �ʿ� ���� ����
		if (0 <= iX - 1 && this->_width >= iX + 2)
		{
			// ���� üũ
			if (en_TILE_FIX_OBSTACLE == map[iY][iX - 1] && en_TILE_NONE == map[iY + 1][iX - 1])
			{
				*iOutX = iX;
				*iOutY = iY;
				return true;
			}
			// ������ üũ
			else if (en_TILE_FIX_OBSTACLE == map[iY][iX + 1] && en_TILE_NONE == map[iY + 1][iX + 1])
			{
				*iOutX = iX;
				*iOutY = iY;
				return true;
			}
		}
		// ������ ��
		else if (0 > iX - 1)
		{
			if (en_TILE_FIX_OBSTACLE == map[iY][iX + 1] && en_TILE_NONE == map[iY + 1][iX + 1])
			{
				*iOutX = iX;
				*iOutY = iY;
				return true;
			}
		}
		// �������� ��
		else
		{
			if (en_TILE_FIX_OBSTACLE == map[iY][iX - 1] && en_TILE_NONE == map[iY + 1][iX - 1])
			{
				*iOutX = iX;
				*iOutY = iY;
				return true;
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	�Լ� : Jump_DirLL(short iInX, short iInY, short *iOutX, short *iOutY)
//	���� : 
//		short iInX		: ������ ������ X��ǥ
//		short iInY		: ������ ������ Y��ǥ
//		short *iOutX	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		short *iOutY	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//
//	���� ������ ����� ��ġ���� �� �������� �˻��ϴ� �Լ�
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirLL(int iInX, int iInY, int *iOutX, int *iOutY)
{
	int iX = iInX;
	int iY = iInY;

	while (true)
	{
		iX--;

		if (!this->CheckRange(iX, iY))
			break;

		if ((0 > (iX - 1)) || (en_TILE_FIX_OBSTACLE == map[iY][iX]))
			break;

		if (this->CheckDestination(iX, iY))
		{
			//this->CreateNode(pNode, iX, iY, en_DIR_NN);

			*iOutX = iX;
			*iOutY = iY;
			return true;
		}

		// �� �ʿ� ���� ����
		if (0 <= iY - 1 && this->_height >= iY + 2)
		{
			// �� �� üũ
			if (en_TILE_FIX_OBSTACLE == map[iY - 1][iX] && en_TILE_NONE == map[iY - 1][iX - 1])
			{
				*iOutX = iX;
				*iOutY = iY;
				return true;
			}
			// �Ʒ� �� üũ
			else if (en_TILE_FIX_OBSTACLE == map[iY + 1][iX] && en_TILE_NONE == map[iY + 1][iX - 1])
			{
				*iOutX = iX;
				*iOutY = iY;
				return true;
			}
		}
		// �� ���� ��
		else if (0 > iY - 1)
		{
			if (en_TILE_FIX_OBSTACLE == map[iY + 1][iX] && en_TILE_NONE == map[iY + 1][iX - 1])
			{
				*iOutX = iX;
				*iOutY = iY;
				return true;
			}
		}
		// �Ʒ� ���� ��
		else
		{
			if (en_TILE_FIX_OBSTACLE == map[iY - 1][iX] && en_TILE_NONE == map[iY - 1][iX - 1])
			{
				*iOutX = iX;
				*iOutY = iY;
				return true;
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	�Լ� : Jump_DirRU(short iInX, short iInY, short *iMiddleOutX, short *iMiddleOutY, short *iOutX, short *iOutY, BYTE *byOutDir)
//	���� : 
//		short iInX			: ������ ������ X��ǥ
//		short iInY			: ������ ������ Y��ǥ
//		short iMiddleOutX	: �߰� ��� �����ϱ� �� ��� ����� X��ǥ
//		short iMiddleOutY	: �߰� ��� �����ϱ� �� ��� ����� Y��ǥ
//		WORD *iOutX			: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		WORD *iOutY			: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		BYTE *byOutDir		: ��� ���� �������� ã�Ҵ��� ����
//
//	���� ������ ����� ��ġ���� ������ �� �������� �˻��ϴ� �Լ�
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirRU(int iInX, int iInY, int *iMiddleOutX, int *iMiddleOutY, int *iOutX, int *iOutY, BYTE *byOutDir)
{
	int iX = iInX;
	int iY = iInY;

	while (true)
	{
		iX++;
		iY--;

		if (!this->CheckRange(iX, iY))
			break;

		if ((0 > (iX - 1)) || (en_TILE_FIX_OBSTACLE == map[iY][iX]))
			break;

		if (this->CheckDestination(iX, iY))
		{
			*iMiddleOutX = iX;
			*iMiddleOutY = iY;
			*iOutX = (-1);
			//*iOutY = (-1);
			return true;
		}

		if (this->Jump_DirUU(iX, iY, iOutX, iOutY))
		{
			*iMiddleOutX = iX;
			*iMiddleOutY = iY;
			*byOutDir = en_DIR_UU;
			return true;
		}

		if (this->Jump_DirRR(iX, iY, iOutX, iOutY))
		{
			*iMiddleOutX = iX;
			*iMiddleOutY = iY;
			*byOutDir = en_DIR_RR;
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	�Լ� : Jump_DirRD(short iInX, short iInY, short *iMiddleOutX, short *iMiddleOutY, short *iOutX, short *iOutY, BYTE *byOutDir)
//	���� : 
//		short iInX			: ������ ������ X��ǥ
//		short iInY			: ������ ������ Y��ǥ
//		short iMiddleOutX	: �߰� ��� �����ϱ� �� ��� ����� X��ǥ
//		short iMiddleOutY	: �߰� ��� �����ϱ� �� ��� ����� Y��ǥ
//		WORD *iOutX			: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		WORD *iOutY			: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		BYTE *byOutDir		: ��� ���� �������� ã�Ҵ��� ����
//
//	���� ������ ����� ��ġ���� ������ �Ʒ� �������� �˻��ϴ� �Լ�
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirRD(int iInX, int iInY, int *iMiddleOutX, int *iMiddleOutY, int *iOutX, int *iOutY, BYTE *byOutDir)
{
	int iX = iInX;
	int iY = iInY;

	while (true)
	{
		iX++;
		iY++;

		if (!this->CheckRange(iX, iY))
			break;

		if ((0 > (iX - 1)) || (en_TILE_FIX_OBSTACLE == map[iY][iX]))
			break;

		if (this->CheckDestination(iX, iY))
		{
			*iMiddleOutX = iX;
			*iMiddleOutY = iY;
			*iOutX = (-1);
			//*iOutY = (-1);
			return true;
		}

		if (this->Jump_DirRR(iX, iY, iOutX, iOutY))
		{
			*iMiddleOutX = iX;
			*iMiddleOutY = iY;
			*byOutDir = en_DIR_RR;
			return true;
		}

		if (this->Jump_DirDD(iX, iY, iOutX, iOutY))
		{
			*iMiddleOutX = iX;
			*iMiddleOutY = iY;
			*byOutDir = en_DIR_DD;
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	�Լ� : Jump_DirLD(short iInX, short iInY, short *iMiddleOutX, short *iMiddleOutY, short *iOutX, short *iOutY, BYTE *byOutDir)
//	���� : 
//		short iInX			: ������ ������ X��ǥ
//		short iInY			: ������ ������ Y��ǥ
//		short iMiddleOutX	: �߰� ��� �����ϱ� �� ��� ����� X��ǥ
//		short iMiddleOutY	: �߰� ��� �����ϱ� �� ��� ����� Y��ǥ
//		WORD *iOutX			: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		WORD *iOutY			: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		BYTE *byOutDir		: ��� ���� �������� ã�Ҵ��� ����
//
//	���� ������ ����� ��ġ���� ���� �Ʒ� �������� �˻��ϴ� �Լ�
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirLD(int iInX, int iInY, int *iMiddleOutX, int *iMiddleOutY, int *iOutX, int *iOutY, BYTE *byOutDir)
{
	int iX = iInX;
	int iY = iInY;

	while (true)
	{
		iX--;
		iY++;

		if (!this->CheckRange(iX, iY))
			break;

		if ((0 > (iX - 1)) || (en_TILE_FIX_OBSTACLE == map[iY][iX]))
			break;

		if (this->CheckDestination(iX, iY))
		{
			*iMiddleOutX = iX;
			*iMiddleOutY = iY;
			*iOutX = (-1);
			//*iOutY = (-1);
			return true;
		}

		if (this->Jump_DirDD(iX, iY, iOutX, iOutY))
		{
			*iMiddleOutX = iX;
			*iMiddleOutY = iY;
			*byOutDir = en_DIR_DD;
			return true;
		}

		if (this->Jump_DirLL(iX, iY, iOutX, iOutY))
		{
			*iMiddleOutX = iX;
			*iMiddleOutY = iY;
			*byOutDir = en_DIR_LL;
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	�Լ� : Jump_DirLU(short iInX, short iInY, short *iMiddleOutX, short *iMiddleOutY, short *iOutX, short *iOutY, BYTE *byOutDir)
//	���� : 
//		short iInX			: ������ ������ X��ǥ
//		short iInY			: ������ ������ Y��ǥ
//		short iMiddleOutX	: �߰� ��� �����ϱ� �� ��� ����� X��ǥ
//		short iMiddleOutY	: �߰� ��� �����ϱ� �� ��� ����� Y��ǥ
//		WORD *iOutX			: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		WORD *iOutY			: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� �����Ѵ�.
//		BYTE *byOutDir		: ��� ���� �������� ã�Ҵ��� ����
//
//	���� ������ ����� ��ġ���� ���� �� �������� �˻��ϴ� �Լ�
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirLU(int iInX, int iInY, int *iMiddleOutX, int *iMiddleOutY, int *iOutX, int *iOutY, BYTE *byOutDir)
{
	int iX = iInX;
	int iY = iInY;

	while (true)
	{
		iX--;
		iY--;

		if (!this->CheckRange(iX, iY))
			break;

		if ((0 > (iX - 1)) || (en_TILE_FIX_OBSTACLE == map[iY][iX]))
			break;

		if (this->CheckDestination(iX, iY))
		{
			*iMiddleOutX = iX;
			*iMiddleOutY = iY;
			*iOutX = (-1);
			//*iOutY = (-1);
			return true;
		}

		if (this->Jump_DirLL(iX, iY, iOutX, iOutY))
		{
			*iMiddleOutX = iX;
			*iMiddleOutY = iY;
			*byOutDir = en_DIR_LL;
			return true;
		}

		if (this->Jump_DirUU(iX, iY, iOutX, iOutY))
		{
			*iMiddleOutX = iX;
			*iMiddleOutY = iY;
			*byOutDir = en_DIR_UU;
			return true;
		}
	}

	return false;
}

bool CJumpPointSearch::CheckRange(int iPosX, int iPosY)
{
	if ((0 > iPosX) || ((this->_width - 1) < iPosX) || (0 > iPosY) || ((this->_height - 1) < iPosY))
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

		pOut[iCnt].X = TILE_to_POS_X(pCurr->_iPosX);
		pOut[iCnt].Y = TILE_to_POS_Y(pCurr->_iPosY);

		stack.pop();
	}

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