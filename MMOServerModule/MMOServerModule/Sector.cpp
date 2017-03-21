#include "stdafx.h"
#include "GameServer.h"

CGameServer::CSector::CSector(int iWidth, int iHeight)
{
	this->_iWidth = iWidth;
	this->_iHeight = iHeight;

	this->_map = new std::map<CLIENT_ID, CPlayer *>*[iHeight];

	for (int i = 0; i < iHeight; ++i)
		this->_map[i] = new std::map<CLIENT_ID, CPlayer *>[iWidth];
}

CGameServer::CSector::~CSector()
{
	for (int i = 0; i < _iHeight; ++i)
		delete[] this->_map[i];

	delete[] this->_map;
}

bool CGameServer::CSector::CheckRange(int iSectorX, int iSectorY)
{
	if (0 > iSectorX
		|| this->_iWidth - 1 < iSectorX
		|| 0 > iSectorY
		|| this->_iHeight - 1 < iSectorY)
		return false;
	else
		return true;
}

bool CGameServer::CSector::InsertPlayer_Sector(int iSectorX, int iSectorY, CLIENT_ID clientID, CPlayer *pPlayer)
{
	if ((-1) == iSectorX && (-1) == iSectorY)
		return true;

	if (!this->CheckRange(iSectorX, iSectorY))
		return false;

	std::map<CLIENT_ID, CPlayer *> *map = &this->_map[iSectorY][iSectorX];
	auto iter = map->find(clientID);

	if (iter == map->end())
	{
		map->insert(std::pair<CLIENT_ID, CPlayer *>(clientID, pPlayer));
		return true;
	}
	else
		return false;
}

bool CGameServer::CSector::DeletePlayer_Sector(int iSectorX, int iSectorY, CLIENT_ID clientID, CPlayer *pPlayer)
{
	if ((-1) == iSectorX && (-1) == iSectorY)
		return true;

	if (!this->CheckRange(iSectorX, iSectorY))
		return false;

	std::map<CLIENT_ID, CPlayer *> *map = &this->_map[iSectorY][iSectorX];
	auto iter = map->find(clientID);

	if (iter != map->end())
	{
		map->erase(iter);
		return true;
	}
	else
		return false;
}

bool CGameServer::CSector::MoveSector(int iCurrX, int iCurrY, int iMoveX, int iMoveY, CLIENT_ID clientID, CPlayer *pPlayer)
{
	if (!this->DeletePlayer_Sector(iCurrX, iCurrY, clientID, pPlayer))
	{
		SYSLOG(L"SECTOR", LOG::LEVEL_DEBUG, L"Do not exists clientID");
		CCrashDump::Crash();
		return false;
	}

	if (!this->InsertPlayer_Sector(iMoveX, iMoveY, clientID, pPlayer))
	{
		SYSLOG(L"SECTOR", LOG::LEVEL_DEBUG, L"Already exists");
		CCrashDump::Crash();
		return false;
	}

	return true;
}

void CGameServer::CSector::GetAroundSector(int iSectorX, int iSectorY, CGameServer::stAROUND_SECTOR *around)
{
	iSectorX--;
	iSectorY--;

	around->_iCount = 0;

	for (int iCntY = 0; iCntY < 3; iCntY++)
	{
		if (iSectorY + iCntY < 0 || iSectorY + iCntY >= dfSECTOR_Y_MAX)
			continue;

		for (int iCntX = 0; iCntX < 3; iCntX++)
		{
			if (iSectorX + iCntX < 0 || iSectorX + iCntX >= dfSECTOR_X_MAX)
				continue;

			around->_around[around->_iCount]._iSectorX = iSectorX + iCntX;
			around->_around[around->_iCount]._iSectorY = iSectorY + iCntY;
			around->_iCount++;
		}
	}

	return;
}

void CGameServer::CSector::GetUpdateSector(CGameServer::stAROUND_SECTOR *removeSector, CGameServer::stAROUND_SECTOR *addSector)
{
	stAROUND_SECTOR removeAround, addAround;
	removeAround._iCount = 0;
	addAround._iCount = 0;

	int iSectorX;
	int iSectorY;

	int removeCnt = 0;
	int addCnt = 0;

	bool bFind;

	// 삭제할 섹터를 구함
	for (removeCnt; removeCnt < removeSector->_iCount; ++removeCnt)
	{
		iSectorX = removeSector->_around[removeCnt]._iSectorX;
		iSectorY = removeSector->_around[removeCnt]._iSectorY;
		bFind = false;

		for (addCnt = 0; addCnt < addSector->_iCount; ++addCnt)
		{
			if (iSectorX == addSector->_around[addCnt]._iSectorX
				&& iSectorY == addSector->_around[addCnt]._iSectorY)
			{
				bFind = true;
				break;
			}
		}

		if (!bFind)
		{
			removeAround._around[removeAround._iCount]._iSectorX = iSectorX;
			removeAround._around[removeAround._iCount]._iSectorY = iSectorY;
			removeAround._iCount++;
		}
	}

	// 추가할 섹터를 구함
	removeCnt = 0;
	addCnt = 0;

	for (addCnt; addCnt < addSector->_iCount; ++addCnt)
	{
		iSectorX = addSector->_around[addCnt]._iSectorX;
		iSectorY = addSector->_around[addCnt]._iSectorY;
		bFind = false;

		for (removeCnt = 0; removeCnt < removeSector->_iCount; ++removeCnt)
		{
			if (iSectorX == removeSector->_around[removeCnt]._iSectorX
				&& iSectorY == removeSector->_around[removeCnt]._iSectorY)
			{	
				bFind = true;
				break;
			}
		}

		if (!bFind)
		{
			addAround._around[addAround._iCount]._iSectorX = iSectorX;
			addAround._around[addAround._iCount]._iSectorY = iSectorY;
			addAround._iCount++;
		}
	}

	(*removeSector)._iCount = removeAround._iCount;
	for (int i = 0; i < removeAround._iCount; ++i)
		(*removeSector)._around[i] = removeAround._around[i];

	(*addSector)._iCount = addAround._iCount;
	for (int i = 0; i < addAround._iCount; ++i)
		(*addSector)._around[i] = addAround._around[i];

	return;
}

std::map<CLIENT_ID, CGameServer::CPlayer *>* CGameServer::CSector::GetList(int iSectorX, int iSectorY)
{
	if (this->CheckRange(iSectorX, iSectorY))
		return &this->_map[iSectorY][iSectorX];
	else
		return NULL;
}