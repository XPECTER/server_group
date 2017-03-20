#pragma once

template <typename T>
class CField
{
private:
	typedef struct st_TILE
	{
		std::list<T> contents_list;
	}TILE;

public:
	CField(int iWidth, int iHeight)
	{
		this->_width = iWidth;
		this->_height = iHeight;

		// 鸥老 甘 积己
		this->_tileMap = new TILE*[iHeight];

		for (int i = 0; i < iHeight; i++)
			this->_tileMap[i] = new TILE[iWidth];
	}

	~CField()
	{
		// 鸥老 甘 秦力
		for (int i = 0; i < _height; ++i)
		{
			delete[] this->_tileMap[i];
		}

		delete[] this->_tileMap;
	}

	bool AddTileObject(int PosX, int PosY, T content)
	{
		if (!this->CheckRange(PosX, PosY))
			return false;

		if (this->CheckDuplicate(PosX, PosY, content))
			return false;

		_tileMap[PosY][PosX].contents_list.push_back(content);
		return true;
	}

	bool DelTileObject(int PosX, int PosY, T content)
	{
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