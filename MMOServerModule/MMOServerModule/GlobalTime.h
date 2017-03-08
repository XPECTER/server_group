#pragma once

#include <Windows.h>

class CSystemTime
{
public:
	CSystemTime();
	~CSystemTime();

	time_t GetTickCount(void);
private:
	
};