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

	this->_openList.push_front(pNewNode);

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

	switch (byDir)
	{
		case en_DIR_NN:
		{
			if (this->Jump_DirUU(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_UU);

			if (this->Jump_DirRR(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_RR);

			if (this->Jump_DirDD(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_DD);

			if (this->Jump_DirLL(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_LL);

			this->Jump_DirRU(pNode, pNode->_iPosX, pNode->_iPosY, false);
			
			this->Jump_DirRD(pNode, pNode->_iPosX, pNode->_iPosY, false);
			
			this->Jump_DirLD(pNode, pNode->_iPosX, pNode->_iPosY, false);
			
			this->Jump_DirLU(pNode, pNode->_iPosX, pNode->_iPosY, false);

			break;
		}

		case en_DIR_UU:
		{
			if (this->Jump_DirUU(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_RR);

			this->Jump_DirLU(pNode, pNode->_iPosX, pNode->_iPosY, true);

			this->Jump_DirRU(pNode, pNode->_iPosX, pNode->_iPosY, true);

			break;
		}

		case en_DIR_RR:
		{
			if (this->Jump_DirRR(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_RR);

			this->Jump_DirRU(pNode, pNode->_iPosX, pNode->_iPosY, true);

			this->Jump_DirRD(pNode, pNode->_iPosX, pNode->_iPosY, true);

			break;
		}

		case en_DIR_DD:
		{
			if (this->Jump_DirDD(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_DD);

			this->Jump_DirRD(pNode, pNode->_iPosX, pNode->_iPosY, true);

			this->Jump_DirLD(pNode, pNode->_iPosX, pNode->_iPosY, true);

			break;
		}

		case en_DIR_LL:
		{
			if (this->Jump_DirLL(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_LL);

			this->Jump_DirLD(pNode, pNode->_iPosX, pNode->_iPosY, true);

			this->Jump_DirLU(pNode, pNode->_iPosX, pNode->_iPosY, true);
			
			break;
		}

		case en_DIR_RU:
		{
			if (this->Jump_DirUU(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_UU);

			if (this->Jump_DirRR(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_RR);

			this->Jump_DirRU(pNode, pNode->_iPosX, pNode->_iPosY, true);

			break;
		}

		case en_DIR_RD:
		{
			if (this->Jump_DirRR(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_RR);

			if (this->Jump_DirDD(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_DD);

			this->Jump_DirRD(pNode, pNode->_iPosX, pNode->_iPosY, true);

			break;
		}

		case en_DIR_LD:
		{
			if (this->Jump_DirDD(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_DD);

			if (this->Jump_DirLL(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_LL);

			this->Jump_DirLD(pNode, pNode->_iPosX, pNode->_iPosY, true);

			break;
		}

		case en_DIR_LU:
		{
			if (this->Jump_DirLL(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_LL);

			if (this->Jump_DirUU(pNode->_iPosX, pNode->_iPosY, &iOutX, &iOutY))
				this->CreateNode(pNode, iOutX, iOutY, en_DIR_UU);

			this->Jump_DirLU(pNode, pNode->_iPosX, pNode->_iPosY, true);

			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////
//
//	�Լ� : Jump_DirUU(int iInX, int iInY, int *iOutX, int *iOutY, bool bLoop)
//	���� : 
//		int 	: ������ ������ X��ǥ
//		int 	: ������ ������ Y��ǥ
//		int* 	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� X��ǥ�� �����Ѵ�.
//		int* 	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� Y��ǥ�� �����Ѵ�.
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
//	�Լ� : Jump_DirRR(int iInX, int iInY, int *iOutX, int *iOutY, bool bLoop)
//	���� : 
//		int 	: ������ ������ X��ǥ
//		int 	: ������ ������ Y��ǥ
//		int* 	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� X��ǥ�� �����Ѵ�.
//		int* 	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� Y��ǥ�� �����Ѵ�.
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
//	�Լ� : Jump_DirDD(int iInX, int iInY, int *iOutX, int *iOutY, bool bLoop)
//	���� : 
//		int 	: ������ ������ X��ǥ
//		int 	: ������ ������ Y��ǥ
//		int* 	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� X��ǥ�� �����Ѵ�.
//		int* 	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� Y��ǥ�� �����Ѵ�.
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
//	�Լ� : Jump_DirLL(int iInX, int iInY, int *iOutX, int *iOutY, bool bLoop)
//	���� : 
//		int 	: ������ ������ X��ǥ
//		int 	: ������ ������ Y��ǥ
//		int* 	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� X��ǥ�� �����Ѵ�.
//		int* 	: �������� ã�Ұų� �߰� ��� ���� ��ġ�� ã���� Y��ǥ�� �����Ѵ�.
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
//	�Լ� : Jump_DirRU(NODE *pParents, int iPosX, int iPosY, bool bLoop)
//	���� : 
//		NODE *	: ��� ���� �� �θ� ���� ������ ��� ������
//		int		: ������ ������ X ��ǥ
//		int		: ������ ������ Y ��ǥ
//		bool	: �߰� ��带 ã���� �ش� �������� ��� Ž������ ����
//
//	���� ������ ����� ��ġ���� ���� �� �� �������� �˻��ϴ� �Լ�
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirRU(NODE *pParents, int iPosX, int iPosY, bool bLoop)
{
	int iX = iPosX;
	int iY = iPosY;
	int iOutX;
	int iOutY;
	NODE *pDiagonalNode = NULL;

	while (true)
	{
		// ���� �� �� ĭ �հ� �� �� �� ĭ ���� �� �� ��ֹ��̸� ����
		if ((en_TILE_FIX_OBSTACLE == map[iY - 1][iX]) &&
			(en_TILE_FIX_OBSTACLE == map[iY][iX + 1]))
			return false;

		// ������ ���� �� ĭ �̵�
		iX++;
		iY--;

		// ���� ����ų� ���� ĭ�� ��ֹ��̸� ����
		if (!this->CheckRange(iX, iY) || (en_TILE_FIX_OBSTACLE == map[iY][iX]))
			return false;

		// ���� ��ġ�� �������̸� ��� ����� ����
		if (this->CheckDestination(iX, iY))
		{
			if (NULL == pDiagonalNode)
				this->CreateNode(pParents, iX, iY, en_DIR_NN);
			else
				this->CreateNode(pDiagonalNode, iX, iY, en_DIR_NN);

			return true;
		}

		// ���� �� �� ĭ ���� ��
		if ((iX + 1) <= (this->_width - 1))
		{
			if (en_TILE_FIX_OBSTACLE == this->map[iY + 1][iX] &&
				en_TILE_NONE == this->map[iY][iX + 1] &&
				en_TILE_NONE == this->map[iY + 1][iX + 1])
			{
				if (NULL == pDiagonalNode)
					pDiagonalNode = this->CreateNode(pParents, iX, iY, en_DIR_RU);
				else
				{
					if (iX != pDiagonalNode->_iPosX || iY != pDiagonalNode->_iPosY)
					{
						pDiagonalNode = this->CreateNode(pDiagonalNode, iX, iY, en_DIR_RU);
					}
				}
				
				this->CreateNode(pDiagonalNode, (iX + 1), (iY + 1), en_DIR_RD);
			}
		}
		// �� �� �� ĭ ���� ��
		else if (0 <= (iY - 1))
		{
			if (en_TILE_FIX_OBSTACLE == this->map[iY][iX - 1] &&
				en_TILE_NONE == this->map[iY - 1][iX] &&
				en_TILE_NONE == this->map[iY - 1][iX - 1])
			{
				if (NULL == pDiagonalNode)
					pDiagonalNode = this->CreateNode(pParents, iX, iY, en_DIR_RU);
				else
				{
					if (iX != pDiagonalNode->_iPosX || iY != pDiagonalNode->_iPosY)
					{
						pDiagonalNode = this->CreateNode(pDiagonalNode, iX, iY, en_DIR_RU);
					}
				}
				
				this->CreateNode(pDiagonalNode, (iX - 1), (iY - 1), en_DIR_LU);
			}
		}

		// �� �� ��ȸ �� �߰� ��带 ������ ���� ���� ��� ����� ����.
		// ���� ã�� ��ġ�� �������� ���������� ����� ����
		if (this->Jump_DirUU(iX, iY, &iOutX, &iOutY))
		{
			if (NULL == pDiagonalNode)
				pDiagonalNode = this->CreateNode(pParents, iX, iY, en_DIR_RU);
			else
			{
				if (iX != pDiagonalNode->_iPosX || iY != pDiagonalNode->_iPosY)
				{
					pDiagonalNode = this->CreateNode(pDiagonalNode, iX, iY, en_DIR_RU);
				}
			}

			if (this->_iEndX == iOutX && this->_iEndY == iOutY)
			{
				this->CreateNode(pDiagonalNode, iOutX, iOutY, en_DIR_NN);
				return true;
			}
			
			if (false == bLoop)
				return true;
		}

		// ���� �� ��ȸ �� �߰� ��带 ������ ���� ���� ��� ����� ����.
		// ���� ã�� ��ġ�� �������� ���������� ����� ����
		if (this->Jump_DirRR(iX, iY, &iOutX, &iOutY))
		{
			if (NULL == pDiagonalNode)
				pDiagonalNode = this->CreateNode(pParents, iX, iY, en_DIR_RU);
			else
			{
				if (iX != pDiagonalNode->_iPosX || iY != pDiagonalNode->_iPosY)
				{
					pDiagonalNode = this->CreateNode(pDiagonalNode, iX, iY, en_DIR_RU);
				}
			}

			if (this->_iEndX == iOutX && this->_iEndY == iOutY)
			{
				this->CreateNode(pDiagonalNode, iOutX, iOutY, en_DIR_NN);
				return true;
			}

			if (false == bLoop)
				return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	�Լ� : Jump_DirRD(NODE *pParents, int iPosX, int iPosY, bool bLoop)
//	���� : 
//		NODE *	: ��� ���� �� �θ� ���� ������ ��� ������
//		int		: ������ ������ X ��ǥ
//		int		: ������ ������ Y ��ǥ
//		bool	: �߰� ��带 ã���� �ش� �������� ��� Ž������ ����
//
//	���� ������ ����� ��ġ���� ���� �� �Ʒ� �������� �˻��ϴ� �Լ�
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirRD(NODE *pParents, int iPosX, int iPosY, bool bLoop)
{
	int iX = iPosX;
	int iY = iPosY;
	int iOutX;
	int iOutY;
	NODE *pDiagonalNode = NULL;

	while (true)
	{
		// ���� �� �� ĭ �հ� �� �� �� ĭ ���� �� �� ��ֹ��̸� ����
		if ((en_TILE_FIX_OBSTACLE == map[iY + 1][iX]) &&
			(en_TILE_FIX_OBSTACLE == map[iY][iX + 1]))
			return false;

		// ������ �Ʒ��� �� ĭ �̵�
		iX++;
		iY++;

		// ���� ����ų� ���� ĭ�� ��ֹ��̸� ����
		if (!this->CheckRange(iX, iY) || (en_TILE_FIX_OBSTACLE == map[iY][iX]))
			return false;

		// ���� ��ġ�� �������̸� ��� ����� ����
		if (this->CheckDestination(iX, iY))
		{
			if (NULL == pDiagonalNode)
				this->CreateNode(pParents, iX, iY, en_DIR_NN);
			else
				this->CreateNode(pDiagonalNode, iX, iY, en_DIR_NN);

			return true;
		}

		// ���� �� �� ĭ ���� ��
		if ((iX + 1) <= (this->_width - 1))
		{
			if (en_TILE_FIX_OBSTACLE == this->map[iY - 1][iX] &&
				en_TILE_NONE == this->map[iY][iX + 1] &&
				en_TILE_NONE == this->map[iY - 1][iX + 1])
			{
				if (NULL == pDiagonalNode)
					pDiagonalNode = this->CreateNode(pParents, iX, iY, en_DIR_RD);
				else
				{
					if (iX != pDiagonalNode->_iPosX || iY != pDiagonalNode->_iPosY)
					{
						pDiagonalNode = this->CreateNode(pDiagonalNode, iX, iY, en_DIR_RD);
					}
				}
			
				this->CreateNode(pDiagonalNode, (iX + 1), (iY + 1), en_DIR_RU);
			}
		}
		// �Ʒ� �� �� ĭ ���� ��
		else if ((iY + 1) <= (this->_height - 1))
		{
			if (en_TILE_FIX_OBSTACLE == this->map[iY][iX - 1] &&
				en_TILE_NONE == this->map[iY + 1][iX] &&
				en_TILE_NONE == this->map[iY + 1][iX - 1])
			{
				if (NULL == pDiagonalNode)
					pDiagonalNode = this->CreateNode(pParents, iX, iY, en_DIR_RU);
				else
				{
					if (iX != pDiagonalNode->_iPosX || iY != pDiagonalNode->_iPosY)
					{
						pDiagonalNode = this->CreateNode(pDiagonalNode, iX, iY, en_DIR_RU);
					}
				}
			
				this->CreateNode(pDiagonalNode, (iX - 1), (iY - 1), en_DIR_LD);
			}
		}

		// ���� �� ��ȸ �� �߰� ��带 ������ ���� ���� ��� ����� ����.
		// ���� ã�� ��ġ�� �������� ���������� ����� ����
		if (this->Jump_DirRR(iX, iY, &iOutX, &iOutY))
		{
			if (NULL == pDiagonalNode)
				pDiagonalNode = this->CreateNode(pParents, iX, iY, en_DIR_RD);
			else
			{
				if (iX != pDiagonalNode->_iPosX || iY != pDiagonalNode->_iPosY)
				{
					pDiagonalNode = this->CreateNode(pDiagonalNode, iX, iY, en_DIR_RD);
				}
			}

			if (this->_iEndX == iOutX && this->_iEndY == iOutY)
			{
				this->CreateNode(pDiagonalNode, iOutX, iOutY, en_DIR_NN);
				return true;
			}
				
			if (false == bLoop)
				return true;
		}

		// �Ʒ� �� ��ȸ �� �߰� ��带 ������ ���� ���� ��� ����� ����.
		// ���� ã�� ��ġ�� �������� ���������� ����� ����
		if (this->Jump_DirDD(iX, iY, &iOutX, &iOutY))
		{
			if (NULL == pDiagonalNode)
				pDiagonalNode = this->CreateNode(pParents, iX, iY, en_DIR_RD);
			else
			{
				if (iX != pDiagonalNode->_iPosX || iY != pDiagonalNode->_iPosY)
				{
					pDiagonalNode = this->CreateNode(pDiagonalNode, iX, iY, en_DIR_RD);
				}
			}

			if (this->_iEndX == iOutX && this->_iEndY == iOutY)
			{
				this->CreateNode(pDiagonalNode, iOutX, iOutY, en_DIR_NN);
				return true;
			}

			if (false == bLoop)
				return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	�Լ� : Jump_DirLD(NODE *pParents, int iPosX, int iPosY, bool bLoop)
//	���� : 
//		NODE *	: ��� ���� �� �θ� ���� ������ ��� ������
//		int		: ������ ������ X ��ǥ
//		int		: ������ ������ Y ��ǥ
//		bool	: �߰� ��带 ã���� �ش� �������� ��� Ž������ ����
//
//	���� ������ ����� ��ġ���� �� �� �Ʒ� �������� �˻��ϴ� �Լ�
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirLD(NODE *pParents, int iPosX, int iPosY, bool bLoop)
{
	int iX = iPosX;
	int iY = iPosY;
	int iOutX;
	int iOutY;
	NODE *pDiagonalNode = NULL;

	while (true)
	{
		// ���� �� ĭ �հ� �Ʒ��� �� ĭ ���� �� �� ��ֹ��̸� ����
		if ((en_TILE_FIX_OBSTACLE == map[iY + 1][iX]) &&
			(en_TILE_FIX_OBSTACLE == map[iY][iX - 1]))
			return false;

		// ���� �Ʒ��� �� ĭ �̵�
		iX--;
		iY++;

		// ���� ����ų� ���� ĭ�� ��ֹ��̸� ����
		if (!this->CheckRange(iX, iY) || (en_TILE_FIX_OBSTACLE == map[iY][iX]))
			return false;

		// ���� ��ġ�� �������̸� ��� ����� ����
		if (this->CheckDestination(iX, iY))
		{
			if (NULL == pDiagonalNode)
				this->CreateNode(pParents, iX, iY, en_DIR_NN);
			else
				this->CreateNode(pDiagonalNode, iX, iY, en_DIR_NN);

			return true;
		}

		// �Ʒ� �� �� ĭ ���� ���� �ƴϸ�
		if ((iY + 1) <= (this->_height - 1))
		{
			if (en_TILE_FIX_OBSTACLE == this->map[iY][iX + 1] &&
				en_TILE_NONE == this->map[iY+ 1][iX + 1] &&
				en_TILE_NONE == this->map[iY + 1][iX])
			{
				if (NULL == pDiagonalNode)
					pDiagonalNode = this->CreateNode(pParents, iX, iY, en_DIR_LD);
				else
				{
					if (iX != pDiagonalNode->_iPosX || iY != pDiagonalNode->_iPosY)
					{
						pDiagonalNode = this->CreateNode(pDiagonalNode, iX, iY, en_DIR_LD);
					}
				}

				this->CreateNode(pDiagonalNode, (iX - 1), (iY - 1), en_DIR_RD);
			}
		}
		// �� �� �� ĭ ���� ���� �ƴϸ�
		else if (0 <= (iX - 1))
		{
			if (en_TILE_FIX_OBSTACLE == this->map[iY - 1][iX] &&
				en_TILE_NONE == this->map[iY - 1][iX - 1] &&
				en_TILE_NONE == this->map[iY][iX - 1])
			{
				if (NULL == pDiagonalNode)
					pDiagonalNode = this->CreateNode(pParents, iX, iY, en_DIR_LD);
				else
				{
					if (iX != pDiagonalNode->_iPosX || iY != pDiagonalNode->_iPosY)
					{
						pDiagonalNode = this->CreateNode(pDiagonalNode, iX, iY, en_DIR_LD);
					}
				}

				this->CreateNode(pDiagonalNode, (iX - 1), (iY - 1), en_DIR_LU);
			}
		}
		
		// �Ʒ� �� ��ȸ �� �߰� ��带 ������ ���� ���� ��� ����� ����.
		// ���� ã�� ��ġ�� �������� ���������� ����� ����
		if (this->Jump_DirDD(iX, iY, &iOutX, &iOutY))
		{
			if (NULL == pDiagonalNode)
				pDiagonalNode = this->CreateNode(pParents, iX, iY, en_DIR_LD);
			else
			{
				if (iX != pDiagonalNode->_iPosX || iY != pDiagonalNode->_iPosY)
				{
					pDiagonalNode = this->CreateNode(pDiagonalNode, iX, iY, en_DIR_LD);
				}
			}

			if (this->_iEndX == iOutX && this->_iEndY == iOutY)
			{
				this->CreateNode(pDiagonalNode, iOutX, iOutY, en_DIR_NN);
				return true;
			}

			if (false == bLoop)
				return true;
		}

		// �� �� ��ȸ �� �߰� ��带 ������ ���� ���� ��� ����� ����.
		// ���� ã�� ��ġ�� �������� ���������� ����� ����
		if (this->Jump_DirLL(iX, iY, &iOutX, &iOutY))
		{
			if (NULL == pDiagonalNode)
				pDiagonalNode = this->CreateNode(pParents, iX, iY, en_DIR_LD);
			else
			{
				if (iX != pDiagonalNode->_iPosX || iY != pDiagonalNode->_iPosY)
				{
					pDiagonalNode = this->CreateNode(pDiagonalNode, iX, iY, en_DIR_LD);
				}
			}

			if (this->_iEndX == iOutX && this->_iEndY == iOutY)
			{
				this->CreateNode(pDiagonalNode, iOutX, iOutY, en_DIR_NN);
				return true;
			}

			if (false == bLoop)
				return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
//
//	�Լ� : Jump_DirLU(NODE *pParents, int iPosX, int iPosY, bool bLoop)
//	���� : 
//		NODE *	: ��� ���� �� �θ� ���� ������ ��� ������
//		int		: ������ ������ X ��ǥ
//		int		: ������ ������ Y ��ǥ
//		bool	: �߰� ��带 ã���� �ش� �������� ��� Ž������ ����
//
//	���� ������ ����� ��ġ���� �� �� �� �������� �˻��ϴ� �Լ�
/////////////////////////////////////////////////////////////////////
bool CJumpPointSearch::Jump_DirLU(NODE *pParents, int iPosX, int iPosY, bool bLoop)
{
	int iX = iPosX;
	int iY = iPosY;
	int iOutX;
	int iOutY;
	NODE *pDiagonalNode = NULL;

	while (true)
	{
		// ���� �� ĭ �հ� �� �� �� ĭ ���� �� �� ��ֹ��̸� ����
		if ((en_TILE_FIX_OBSTACLE == map[iY - 1][iX]) &&
			(en_TILE_FIX_OBSTACLE == map[iY][iX - 1]))
			return false;

		iX--;
		iY--;

		// ���� ����ų� ���� ĭ�� ��ֹ��̸� ����
		if (!this->CheckRange(iX, iY) || (en_TILE_FIX_OBSTACLE == map[iY][iX]))
			return false;

		// ���� ��ġ�� �������̸� ��� ����� ����
		if (this->CheckDestination(iX, iY))
		{
			if (NULL == pDiagonalNode)
				this->CreateNode(pParents, iX, iY, en_DIR_NN);
			else
				this->CreateNode(pDiagonalNode, iX, iY, en_DIR_NN);

			return true;
		}

		// �� �� �� ĭ ���� ���� �ƴϸ�
		if (0 <= (iX - 1))
		{
			if (en_TILE_FIX_OBSTACLE == this->map[iY + 1][iX] &&
				en_TILE_NONE == this->map[iY + 1][iX - 1] &&
				en_TILE_NONE == this->map[iY][iX - 1])
			{
				if (NULL == pDiagonalNode)
					pDiagonalNode = this->CreateNode(pParents, iX, iY, en_DIR_LU);
				else
				{
					if (iX != pDiagonalNode->_iPosX || iY != pDiagonalNode->_iPosY)
					{
						pDiagonalNode = this->CreateNode(pDiagonalNode, iX, iY, en_DIR_LU);
					}
				}

				this->CreateNode(pDiagonalNode, (iX - 1), (iY - 1), en_DIR_LD);
			}
		}
		// �� �� ��ĭ ���� ���� �ƴϸ�
		else if (0 <= (iY - 1))
		{
			if (en_TILE_FIX_OBSTACLE == this->map[iY][iX + 1] &&
				en_TILE_NONE == this->map[iY - 1][iX] &&
				en_TILE_NONE == this->map[iY - 1][iX + 1])
			{
				if (NULL == pDiagonalNode)
					pDiagonalNode = this->CreateNode(pParents, iX, iY, en_DIR_LU);
				else
				{
					if (iX != pDiagonalNode->_iPosX || iY != pDiagonalNode->_iPosY)
					{
						pDiagonalNode = this->CreateNode(pDiagonalNode, iX, iY, en_DIR_LU);
					}
				}

				this->CreateNode(pDiagonalNode, (iX - 1), (iY - 1), en_DIR_RU);
			}
		}

		// �� �� ��ȸ �� �߰� ��带 ������ ���� ���� ��� ����� ����.
		// ���� ã�� ��ġ�� �������� ���������� ����� ����
		if (this->Jump_DirLL(iX, iY, &iOutX, &iOutY))
		{
			if (NULL == pDiagonalNode)
				pDiagonalNode = this->CreateNode(pParents, iX, iY, en_DIR_LU);
			else
			{
				if (iX != pDiagonalNode->_iPosX || iY != pDiagonalNode->_iPosY)
				{
					pDiagonalNode = this->CreateNode(pDiagonalNode, iX, iY, en_DIR_LU);
				}
			}

			if (this->_iEndX == iOutX && this->_iEndY == iOutY)
			{
				this->CreateNode(pDiagonalNode, iOutX, iOutY, en_DIR_NN);
				return true;
			}

			if (false == bLoop)
				return true;
		}

		// �� �� ��ȸ �� �߰� ��带 ������ ���� ���� ��� ����� ����.
		// ���� ã�� ��ġ�� �������� ���������� ����� ����
		if (this->Jump_DirUU(iX, iY, &iOutX, &iOutY))
		{
			if (NULL == pDiagonalNode)
				pDiagonalNode = this->CreateNode(pParents, iX, iY, en_DIR_LU);
			else
			{
				if (iX != pDiagonalNode->_iPosX || iY != pDiagonalNode->_iPosY)
				{
					pDiagonalNode = this->CreateNode(pDiagonalNode, iX, iY, en_DIR_LU);
				}
			}

			if (this->_iEndX == iOutX && this->_iEndY == iOutY)
			{
				this->CreateNode(pDiagonalNode, iOutX, iOutY, en_DIR_NN);
				return true;
			}

			if (false == bLoop)
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