﻿#ifndef __LYNX_GAME_SERVER_ConfigData1_H__
#define __LYNX_GAME_SERVER_ConfigData1_H__
#include "stdafx.h"

using namespace std;

#define CARDNUMBER 120

enum HuTypeEnum
{
	None=0,
	PI,//屁胡
	PENGPENG,//碰碰胡
	QIDUI,//七对
	QINGYISE,//清一色
	QYSPENG,//清一色碰碰胡
	QYSQD,//清一色七对
};

enum HeiOrYing
{
	HEI,
	RUAN,
};

enum HUTYPE{
	ZIMO_TYPE,//自摸
	PAO_TYPE,//放炮
	PAOGANG_TYPE,//抢杠
	ZIMOGANG_TYPE//杠上开花
};

enum OTHERHANDPAO{
	MENQING_PAO,//门清
	KA_DU_PAO,//卡独
	GANGHU_PAO,//杠胡
	HONGBAN_PAO,//红中白板
	KE_PAO,//除去红中白板
};

enum PENGPAO{
	NONE_PAO,
	MGANG_PAO,
	AGANG_PAO,
	PENG_PAO,
};



struct HuItem
{
	HuTypeEnum	_hutype;
	HeiOrYing	_hy;
};

struct PaoItem
{
	map<HUTYPE,int> _hu;
	map<OTHERHANDPAO, int> _handpao;
	map<PENGPAO, int> _pengpao;
	int _fapao;
};

static int g_all_mjkind[] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
	0x31, 0x32, 0x33, 0x34,
	0x35, 0x36, 0x37
};

#define g_kind (sizeof(g_all_mjkind)/sizeof(int))

static int g_all_mj[] = {
	g_all_mjkind[0], g_all_mjkind[0], g_all_mjkind[0], g_all_mjkind[0],
	g_all_mjkind[1], g_all_mjkind[1], g_all_mjkind[1], g_all_mjkind[1],
	g_all_mjkind[2], g_all_mjkind[2], g_all_mjkind[2], g_all_mjkind[2],
	g_all_mjkind[3], g_all_mjkind[3], g_all_mjkind[3], g_all_mjkind[3],
	g_all_mjkind[4], g_all_mjkind[4], g_all_mjkind[4], g_all_mjkind[4],
	g_all_mjkind[5], g_all_mjkind[5], g_all_mjkind[5], g_all_mjkind[5],
	g_all_mjkind[6], g_all_mjkind[6], g_all_mjkind[6], g_all_mjkind[6],
	g_all_mjkind[7], g_all_mjkind[7], g_all_mjkind[7], g_all_mjkind[7],
	g_all_mjkind[8], g_all_mjkind[8], g_all_mjkind[8], g_all_mjkind[8],

	g_all_mjkind[9], g_all_mjkind[9], g_all_mjkind[9], g_all_mjkind[9],
	g_all_mjkind[10], g_all_mjkind[10], g_all_mjkind[10], g_all_mjkind[10],
	g_all_mjkind[11], g_all_mjkind[11], g_all_mjkind[11], g_all_mjkind[11],
	g_all_mjkind[12], g_all_mjkind[12], g_all_mjkind[12], g_all_mjkind[12],
	g_all_mjkind[13], g_all_mjkind[13], g_all_mjkind[13], g_all_mjkind[13],
	g_all_mjkind[14], g_all_mjkind[14], g_all_mjkind[14], g_all_mjkind[14],
	g_all_mjkind[15], g_all_mjkind[15], g_all_mjkind[15], g_all_mjkind[15],
	g_all_mjkind[16], g_all_mjkind[16], g_all_mjkind[16], g_all_mjkind[16],
	g_all_mjkind[17], g_all_mjkind[17], g_all_mjkind[17], g_all_mjkind[17],

	g_all_mjkind[18], g_all_mjkind[18], g_all_mjkind[18], g_all_mjkind[18],
	g_all_mjkind[19], g_all_mjkind[19], g_all_mjkind[19], g_all_mjkind[19],
	g_all_mjkind[20], g_all_mjkind[20], g_all_mjkind[20], g_all_mjkind[20],
	g_all_mjkind[21], g_all_mjkind[21], g_all_mjkind[21], g_all_mjkind[21],
	g_all_mjkind[22], g_all_mjkind[22], g_all_mjkind[22], g_all_mjkind[22],
	g_all_mjkind[23], g_all_mjkind[23], g_all_mjkind[23], g_all_mjkind[23],
	g_all_mjkind[24], g_all_mjkind[24], g_all_mjkind[24], g_all_mjkind[24],
	g_all_mjkind[25], g_all_mjkind[25], g_all_mjkind[25], g_all_mjkind[25],
	g_all_mjkind[26], g_all_mjkind[26], g_all_mjkind[26], g_all_mjkind[26],

	g_all_mjkind[27], g_all_mjkind[27], g_all_mjkind[27], g_all_mjkind[27],
	g_all_mjkind[28], g_all_mjkind[28], g_all_mjkind[28], g_all_mjkind[28],
	g_all_mjkind[29], g_all_mjkind[29], g_all_mjkind[29], g_all_mjkind[29],
	g_all_mjkind[30], g_all_mjkind[30], g_all_mjkind[30], g_all_mjkind[30],
	
	g_all_mjkind[31], g_all_mjkind[31], g_all_mjkind[31], g_all_mjkind[31],
	g_all_mjkind[32], g_all_mjkind[32], g_all_mjkind[32], g_all_mjkind[32],
	g_all_mjkind[33], g_all_mjkind[33], g_all_mjkind[33], g_all_mjkind[33],
};

class ConfigData1 
{
public:
	ConfigData1();
	virtual ~ConfigData1();
	
	static ConfigData1 *getIns();
	void init();
	void quickSort(int *s, int l, int r);
	void setKezi();
	void setFengKezi(int jj);
	void setShunzi();
	map<int, vector<int>> getKindCard(int *temppai);
	HuItem isHu(int *pai);
	HuItem isHu(int *pai, int bao);
private:
	void setValueZero(int *a, int v, int len,int &baocount);
	vector<int> isTing(int *pai,int bao);
	map<int, vector<int>>chuTing(int *pai,int bao);
	PaoItem getHandOtherPao(int *a, int *peng, PENGPAO *ptype, int facount, int bao, int baoniang, int zhua, bool isgang);
	void test();
	HuTypeEnum isFit(vector<int> p);
	HuTypeEnum isFit1(int index, map<string, vector<int>>&vec, vector<int>&p);
	HuTypeEnum isFit2(int index, map<string, vector<int>>&vec, vector<int>&p);
	HuTypeEnum isFit3(int index, map<string, vector<int>>&vec, vector<int>&p);
	HuTypeEnum isFit4(int index, map<string, vector<int>>&vec, vector<int>&p);
	HuTypeEnum eraseVec(vector<int>&p, vector<int>ep);
	vector<int> copyVec(vector<int> p);
	void setTemp(vector<int>&tp,int v1,int v2=0,int v3=0,int v4=0);
private:
	static ConfigData1 *m_ins;
	map<string,bool>m_kezi;
	map<string, bool>m_shunzi;
	redis *m_predis;
};


#endif // __LYNX_GAME_SERVER_ConfigData1_H__