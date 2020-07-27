//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_hl2mpbasebasebludgeon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CWeaponSuitcase C_WeaponSuitcase
#endif

class CWeaponSuitcase : public CBaseHL2MPBludgeonWeapon
{
	DECLARE_CLASS(CWeaponSuitcase, CBaseHL2MPBludgeonWeapon);

public:

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif
	void		PrimaryAttack(void)	{}
	void		SecondaryAttack(void)	{}
};

//-----------------------------------------------------------------------------
// CWeaponSuitcase
//-----------------------------------------------------------------------------
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponSuitcase, DT_WeaponSuitcase)

BEGIN_NETWORK_TABLE(CWeaponSuitcase, DT_WeaponSuitcase)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponSuitcase)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_suitcase, CWeaponSuitcase);
PRECACHE_WEAPON_REGISTER(weapon_suitcase);


#ifndef CLIENT_DLL

acttable_t	CWeaponSuitcase::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_MELEE, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_MELEE, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_MELEE, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_MELEE, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_MELEE, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_MELEE, false },
	//NPC
	{ ACT_MELEE_ATTACK1, ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_MELEE, true },
};

IMPLEMENT_ACTTABLE(CWeaponSuitcase);

#endif