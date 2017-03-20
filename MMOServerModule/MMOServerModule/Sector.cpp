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

bool CGameServer::CSector::MoveSector(int iBeforeX, int iBeforeY, int iAfterX, int iAfterY, CLIENT_ID clientID, CPlayer *pPlayer)
{
	if (!this->DeletePlayer_Sector(iBeforeX, iBeforeY, clientID, pPlayer))
	{
		SYSLOG(L"SECTOR", LOG::LEVEL_DEBUG, L"Do not exists clientID");
		CCrashDump::Crash();
		return false;
	}

	if (!this->InsertPlayer_Sector(iAfterX, iAfterY, clientID, pPlayer))
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

void CGameServer::CSector::GetUpdateSector(CGameServer::stAROUND_SECTOR *beforeSector, CGameServer::stAROUND_SECTOR *afterSector)
{
	stAROUND_SECTOR removeSector, addSector;
	removeSector._iCount = 0;
	addSector._iCount = 0;

	int iSectorX;
	int iSectorY;

	int removeCnt = 0;
	int addCnt = 0;

	bool bFind;

	// 삭제할 섹터를 구함
	for (removeCnt; removeCnt < beforeSector->_iCount; ++removeCnt)
	{
		iSectorX = beforeSector->_around[removeCnt]._iSectorX;
		iSectorY = beforeSector->_around[removeCnt]._iSectorY;
		bFind = false;

		for (addCnt = 0; addCnt < afterSector->_iCount; ++addCnt)
		{
			if (iSectorX == afterSector->_around[addCnt]._iSectorX
				&& iSectorY == afterSector->_around[addCnt]._iSectorY)
			{
				bFind = true;
				break;
			}
		}

		if (!bFind)
		{
			removeSector._around[removeSector._iCount]._iSectorX = iSectorX;
			removeSector._around[removeSector._iCount]._iSectorY = iSectorY;
			removeSector._iCount++;
		}
	}

	// 추가할 섹터를 구함
	removeCnt = 0;
	addCnt = 0;

	for (addCnt; addCnt < afterSector->_iCount; ++addCnt)
	{
		iSectorX = afterSector->_around[addCnt]._iSectorX;
		iSectorY = afterSector->_around[addCnt]._iSectorY;
		bFind = false;

		for (removeCnt = 0; removeCnt < beforeSector->_iCount; ++removeCnt)
		{
			if (iSectorX == beforeSector->_around[removeCnt]._iSectorX
				&& iSectorY == beforeSector->_around[removeCnt]._iSectorY)
			{	
				bFind = true;
				break;
			}
		}

		if (!bFind)
		{
			addSector._around[addSector._iCount]._iSectorX = iSectorX;
			addSector._around[addSector._iCount]._iSectorY = iSectorY;
			addSector._iCount++;
		}
	}

	(*beforeSector)._iCount = removeSector._iCount;
	for (int i = 0; i < removeSector._iCount; ++i)
		(*beforeSector)._around[i] = removeSector._around[i];

	(*afterSector)._iCount = addSector._iCount;
	for (int i = 0; i < addSector._iCount; ++i)
		(*afterSector)._around[i] = addSector._around[i];

	return;
}