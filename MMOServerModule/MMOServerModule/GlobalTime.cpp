#include "GlobalTime.h"

CSystemTime::CSystemTime()
{

}

CSystemTime::~CSystemTime()
{

}

time_t CSystemTime::GetTickCount(void)
{
	return GetTickCount64();
}