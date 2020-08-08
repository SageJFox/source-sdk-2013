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

enum Repose_Stat
{
	STR,
	DEX,
	//CON,
	INT,
	//WIS,
	CHA,
	STAT_COUNT,
};

enum Repose_SubStats
{
	STAT_HACKING = 1 << 0,			//DEVELOPERS DEVELOPERS DEVELOPERS DEVELOPERS
	STAT_ANIMALCONTROL = 1 << 1,	//animals (guard dogs, etc.) like you
	STAT_BLUECOLLAR = 1 << 2,		//low-level employees (guards, maintainance, etc.) like you
	STAT_WHITECOLLAR = 1 << 3,		//mid-level employees (researchers, etc.) like you
	STAT_MEDICINE = 1 << 4,			//organic healing
	STAT_REPAIR = 1 << 5,			//synthetic healing
	STAT_SMALLARMS = 1 << 6,		//pistols, smgs
	STAT_MEDIUMARMS = 1 << 7,		//rifles, shotguns
	STAT_HEAVYARMS = 1 << 8,		//crossbows, rockets
	STAT_MELEE = 1 << 9,			//fists, knives, swords, hammers, etc.
	STAT_EXPLOSIVES = 1 << 10,		//grenades, oildrums(?)

};
#endif