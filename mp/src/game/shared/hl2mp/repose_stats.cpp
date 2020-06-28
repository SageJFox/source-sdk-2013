//================================ Repose ===================================//
//
// Purpose: stat definitions for usage 
//
// $NoKeywords: $
//
//===========================================================================//
#include "cbase.h"
#include "dbg.h"
#include "vstdlib/random.h"
#include "repose_stats.h"


//constructor
/*
CReposeStats::CReposeStats(int baseType)
{
	statBasesInit();
	statBaseInit(baseType);
}*/

//Define our base types.
void CReposeStats::statBasesInit(void)
{
	//BASETYPE_PLAYER
	for (int i = 0; i < STAT_COUNT; i++)
	{
		int& statMin = statBaseType[BASETYPE_PLAYER][i][0];
		statMin = 3;
		int& statMax = statBaseType[BASETYPE_PLAYER][i][1];
		statMax = 18;
	}
	//BASETYPE_LOWLEVEL
	for (int i = 0; i < STAT_COUNT; i++)
	{
		int& statMin = statBaseType[BASETYPE_LOWLEVEL][i][0];
		statMin = 3;
		int& statMax = statBaseType[BASETYPE_LOWLEVEL][i][1];
		statMax = 18;
	}
	//TODO: actually make this useful for more than just player gen.
}

//Initial setup of our stats.
void CReposeStats::statBaseInit(int baseType)
{
	for (int i = 0; i < STAT_COUNT; i++)
	{
		int statMin = statBaseType[baseType][i][0];
		int statMax = statBaseType[baseType][i][1];

		int& stat = statsBase[i];

		if (baseType == BASETYPE_PLAYER) stat = random->RandomInt(1, 6) + random->RandomInt(1, 6) + random->RandomInt(1, 6);
		else stat = random->RandomInt(statMin, statMax);

		//stat = statMin + rand() % (statMax - statMin + 1);
		calcStat(i);
		//calcMod(i);
	}

	dumpStats(1);
}

//(Re)calculate our modifiers based on our current stats.
void CReposeStats::calcMods(void)
{
	for (int i = 0; i < STAT_COUNT; i++)
	{
		calcMod(i);
	}
}

//(Re)calculate our modifier based on our current stat.
void CReposeStats::calcMod(int stat)
{
	int statClean = checkStat(stat);
	int& modRef = modifiers[stat];
	modRef = (statClean - 10) / 2;
}

//Make sure one particular stat is in its usual bounds.
int CReposeStats::checkStat(int stat)
{
	checkStatBase(stat);
	int& statRef = stats[stat];
	//statRef = statsBase[stat];
	//TODO: stat additions here
	if (statRef <= 0) //knockout/death depending on what score's been reduced to 0
	{
		statRef = 0;
		switch (stat)
		{
		case STR:
			//TODO: STR 0: death
			break;
		case DEX:
			//TODO: DEX 0: unconsious (ragdoll [spasm]). Should it lead to death, or recovery?
			break;
		case CON:
			//TODO: CON 0: death
			break;
		case WIS:
			//TODO: INT 0: unconsious (ragdoll)
			break;
		case INT:
			//TODO: WIS 0: unconsious (ragdoll)
			break;
		case CHA:
			//TODO: CHA 0: unconsious?
			break;
		}
	}
	else if (statRef > STAT_NORMAL_MAX)
	{
		//TODO: allow exceptions
		statRef = STAT_NORMAL_MAX;
	}
	return statRef;
}

//Make sure this base stat is in its usual bounds.
void CReposeStats::checkStatBase(int stat)
{
	//for (int i = 0; i < STAT_COUNT; i++)
	int& statRef = stats[stat];
	if (statsBase[stat] < 0)
	{
		statRef = 0;
		calcStat(stat); //we had to adjust our base stat, so our functional stat's probably different
	}
	else if (statsBase[stat] > BASESTAT_NORMAL_MAX)
	{
		statRef = BASESTAT_NORMAL_MAX;
		//TODO: allow exceptions
		calcStat(stat); //we had to adjust our base stat, so our functional stat's probably different
	}
}

//Make sure all stats are in their usual bounds.
void CReposeStats::checkStats(void)
{
	for (int i = 0; i < STAT_COUNT; i++)
	{
		checkStat(i);
	}
}

//(Re)calculate our current stat based on our base stat, our equipment/items, and any buffs/debuffs applied.
int CReposeStats::calcStat(int stat)
{
	//dumpStats(2);
	checkStatBase(stat);
	int calc = statsBase[stat];
	int& statRef = stats[stat];
	statRef = calc;
	//TODO: equipment, items, and statues added here
	calc = checkStat(stat);
	statRef = calc;
	//dumpStat(stat,2);
	calcMod(stat);
	//dumpStats(1);
	return calc;
}

//Function to handle changes to the base stat
void CReposeStats::addToStatBase(int stat, int amount)
{
	int& statRef = statsBase[stat];
	statRef += amount;
	checkStatBase(stat);
	calcStat(stat);
	calcMod(stat);
}

//Dump all of our stat info to console at developer #.
void CReposeStats::dumpStats(int devLevel)
{
	for (int i = 0; i < STAT_COUNT; i++)
	{
		dumpStat(i, devLevel);
	}
}

//Dump our stat info to console at developer #.
void CReposeStats::dumpStat(int stat, int devLevel)
{
	switch (stat)
	{
	case STR:
		DevMsg("STR:", devLevel);
		break;
	case DEX:
		DevMsg("DEX:", devLevel);
		break;
	case CON:
		DevMsg("CON:", devLevel);
		break;
	case INT:
		DevMsg("INT:", devLevel);
		break;
	case WIS:
		DevMsg("WIS:", devLevel);
		break;
	case CHA:
		DevMsg("CHA:", devLevel);
		break;
	default:
		DevMsg("???: OUT OF BOUNDS?", devLevel); //if we're not one of our enums, we're probably out of bounds
		break;
	}
	//splits are just for formatting options, lines things up nicely when printed to console
	statsBase[stat] < 10 ? DevMsg("  %i ", statsBase[stat], devLevel) : DevMsg(" %i ", statsBase[stat], devLevel);
	stats[stat] < 10 ? DevMsg("|  %i ", stats[stat], devLevel) : DevMsg("| %i ", stats[stat], devLevel);
	modifiers[stat] >= 0 ? DevMsg("(+%i)\n", modifiers[stat], devLevel) : DevMsg("(%i)\n", modifiers[stat], devLevel);
}