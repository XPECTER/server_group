#pragma once

template <typename T>
class CField
{
private:
	enum en_TILE_TYPE
	{
		en_TILE_NORMAL = 0,
		en_TILE_OBSTACLE,
	};

	typedef struct st_TILE
	{
		en_TILE_TYPE _property;
		std::list<T> contents_list;
	}TILE;

public:
	CField(int iWidth, int iHeight)
	{
		this->_width = iWidth;
		this->_height = iHeight;
		this->_tileMap = NULL;
	}

	~CField()
	{
		// Ÿ�� �� ����
		for (int i = 0; i < _height; ++i)
		{
			delete[] this->_tileMap[i];
		}

		delete[] this->_tileMap;
	}

	void Field_Init(void)
	{
		// Ÿ�� �� ����
		if (NULL == this->_tileMap)
		{
			this->_tileMap = new TILE*[this->_height];

			for (int i = 0; i < _height; i++)
				this->_tileMap[i] = new TILE[this->_width];
		}

		for (int iY = 0; iY < this->_height; ++iY)
			for (int iX = 0; iX < this->_width; ++iX)
				SetTileProperty(iX, iY, en_TILE_NORMAL);
	}

	bool LoadTextMap(wchar_t *szFileName, char *obstacle)
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
					SetTileProperty(iWidthCnt, iHeightCnt, en_TILE_OBSTACLE);
				}
				
				token = strtok_s(NULL, ",\n", &next);

				if (NULL == token)
					break;
			}
		}

		return true;
	}

	void SetTileProperty(int PosX, int PosY, en_TILE_TYPE type)
	{
		this->_tileMap[PosY][PosX]._property = type;
	}


	/////////////////////////////////////////////////////////////////////
	//
	//	�Լ� : MoveTileObject(int iCurrX, int iCurrY, int iMoveX, iMoveY, T content)
	//	���� : 
	//		int iCurrX	: �̵� �� X�� Ÿ�� ��ǥ
	//		int iCurrY	: �̵� �� Y�� Ÿ�� ��ǥ
	//		int iMoveX		: �̵��� X�� Ÿ�� ��ǥ
	//		int iMoveY		: �̵��� Y�� Ÿ�� ��ǥ
	//		T	content		: ����Ʈ�� ���� ���� (������ CLIENT_ID�� �����)
	//
	//	�̵� �� Ÿ�� ��ǥ���� ������ ������ �̵��� Ÿ�� ��ǥ�� �־��ش�.
	//	�̵� �� ��ǥ�� (-1, -1)�� ������ �ű� ������ Ÿ�Ͽ� �߰��� �ϰ�,
	//	�̵��� ��ǥ�� (-1, -1)�� ������ ������ ������ Ÿ�Ͽ��� ���Ÿ� �Ѵ�.
	/////////////////////////////////////////////////////////////////////
	bool MoveTileObject(int iCurrX, int iCurrY, int iMoveX, int iMoveY, T content)
	{
		if (!DelTileObject(iCurrX, iCurrY, content))
		{
			SYSLOG(L"FIELD", LOG::LEVEL_DEBUG, L"DelTileObject Failed");
			CCrashDump::Crash();
			return false;
		}

		if (!AddTileObject(iMoveX, iMoveY, content))
		{
			SYSLOG(L"FIELD", LOG::LEVEL_DEBUG, L"AddTileObject Failed");
			CCrashDump::Crash();
			return false;
		}

		return true;
	}

	bool GetTileObject(int PosX, int PosY, std::list<T> *outList)
	{
		if (!this->CheckRange(PosX, PosY))
			return false;

		std::list<T> *list = &this->_tileMap[PosY][PosX].contents_list;

		for (auto iter = list->begin(); iter != list->end(); ++iter)
		{
			outList->push_back((*iter));
		}

		return true;
	}

private:
	bool AddTileObject(int PosX, int PosY, T content)
	{
		if ((-1) == PosX && (-1) == PosY)
			return true;

		if (!this->CheckRange(PosX, PosY))
			return false;

		if (this->CheckDuplicate(PosX, PosY, content))
			return false;

		_tileMap[PosY][PosX].contents_list.push_back(content);
		return true;
	}

	bool DelTileObject(int PosX, int PosY, T content)
	{
		if ((-1) == PosX && (-1) == PosY)
			return true;

		if (!this->CheckRange(PosX, PosY))
			return false;

		std::list<T> *list = &this->_tileMap[PosY][PosX].contents_list;

		for (auto iter = list->begin(); iter != list->end(); ++iter)
		{
			if (content == *iter)
			{
				list->erase(iter);
				return true;
			}
		}

		return false;
	}

	bool CheckRange(int PosX, int PosY)
	{
		if (PosX >= this->_width || PosX < 0 || PosY >= this->_height || PosY < 0)
			return false;

		return true;
	}

	bool CheckDuplicate(int PosX, int PosY, T content)
	{
		std::list<T> *list = &this->_tileMap[PosY][PosX].contents_list;

		for (auto iter = list->begin(); iter != list->end(); ++iter)
		{
			if (content == (*iter))
				return true;
		}

		return false;
	}

private:
	int _width;
	int _height;

	TILE **_tileMap;
};