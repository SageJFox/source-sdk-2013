//================================ Repose ===================================//
//
// Purpose: stat definitions for usage 
//
// $NoKeywords: $
//
//===========================================================================//
#ifndef REPOSE_STATS_H
#define REPOSE_STATS_H
#pragma once
#define BASESTAT_NORMAL_MAX 20
#define STAT_NORMAL_MAX 24

class CReposeStats
{
public:
	enum Repose_Stat
	{
		STR,
		DEX,
		CON,
		INT,
		WIS,
		CHA,
		STAT_COUNT,
	};
	enum Repose_Stat_BaseType
	{
		BASETYPE_PLAYER,
		BASETYPE_LOWLEVEL,
		BASETYPE_MEDLEVEL,
		BASETYPE_HIGHLEVEL,
		BASETYPE_COUNT,
	};
	//constructor
	//CReposeStats(int);
	//destructor
	//~CReposeStats();

	void calcMods(void);
	void calcMod(int);
	int calcStat(int);
	void addToStatBase(int, int);
	int checkStat(int);
	void checkStats(void);
	void checkStatBase(int);

	void statBaseInit(int);
	void statBasesInit(void);
	void dumpStats();
	void dumpStats(int);
	void dumpStat(int);
	void dumpStat(int,int);

private:

	//current stats, including any temporary item or status effect adjustments
	int stats[STAT_COUNT];// = {};

	//base stats
	int statsBase[STAT_COUNT];// = {};

	//modifiers
	int modifiers[STAT_COUNT];// = {};

	//base types for stat generation (basetype,stat,min0/max1)
	int statBaseType[BASETYPE_COUNT][STAT_COUNT][2];// = {};

	
};
#endif