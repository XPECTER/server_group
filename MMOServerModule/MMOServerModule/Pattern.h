#pragma once

#define dfPATTERN_ATTACK_AREA_MAX	37

extern int g_Pattern_AttackRange[3][6];
extern int g_Pattern_AttackTime[3][6];
extern int g_Pattern_AttackPower[3][6];
extern int g_Pattern_AttackArea[6][dfPATTERN_ATTACK_AREA_MAX][2];


float Distance(int iTileX1, int iTileY1, int iTileX2, int iTileY2);
void AttackPushPos(float fPos1X, float fPos1Y, float fPos2X, float fPos2Y, float *pPushX, float *pPushY);
