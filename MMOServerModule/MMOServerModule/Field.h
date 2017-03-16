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
	CField(unsigned short shXTileMax, unsigned short shYTileMax)
	{
		this->_shX = shXTileMax;
		this->_shY = shYTileMax;

		// 鸥老 甘 积己
		this->tileMap = new TILE*[this->_shY];

		for (int i = 0; i < this->_shY; i++)
			this->tileMap[i] = new TILE[this->_shX];
	}

	~CField()
	{
		// 鸥老 甘 秦力
		for (int i = 0; i < _shY; ++i)
		{
			delete[] this->tileMap[i];
		}

		delete[] this->tileMap;
	}

	bool AddTileObject(T content, short shX, short shY)
	{
		if (!this->CheckRange(shX, shY))
			return false;

		if (this->CheckDuplicate(content, shX, shY))
			return false;

		tileMap[shY][shX].contents_list.push_back(content);
		return true;
	}

	bool DelTileObject(T content, short shX, short shY)
	{
		if (!this->CheckRange(shX, shY))
			return false;

		std::list<T> *list = &this->tileMap[shY][shX].contents_list;

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

	bool GetTileObject(short shX, short shY, std::list<T> *outList)
	{
		if (!this->CheckRange(shX, shY))
			return false;

		std::list<T> *list = &this->tileMap[shY][shX].contents_list;

		for (auto iter = list->begin(); iter != list->end(); ++iter)
		{
			outList->push_back((*iter));
		}

		return true;
	}

private:
	bool CheckRange(unsigned short shX, unsigned short shY)
	{
		if (shX >= this->_shX || shX < 0 || shY >= this->_shY || shY < 0)
			return false;

		return true;
	}

	bool CheckDuplicate(T content, unsigned short shX, unsigned short shY)
	{
		std::list<T> *list = &this->tileMap[shY][shX].contents_list;

		for (auto iter = list->begin(); iter != list->end(); ++iter)
		{
			if (content == (*iter))
				return true;
		}

		return false;
	}

private:
	unsigned short _shX;
	unsigned short _shY;

	TILE **tileMap;
};