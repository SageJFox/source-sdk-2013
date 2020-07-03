//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "repose_stats.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
#else
	#include "grenade_ar2.h"
	#include "hl2mp_player.h"
	#include "basegrenade_shared.h"
#endif

#include "weapon_hl2mpbase.h"
#include "weapon_hl2mpbase_machinegun.h"

#ifdef CLIENT_DLL
#define CWeaponHealthkit C_WeaponHealthkit
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define HEALTHKIT_RATE_BASE 0.1f
#define HEALTHKIT_RATE_POSITIVE 0.01f
#define HEALTHKIT_RATE_NEGATIVE 0.02f
#define HEALTHKIT_DENY_SOUNDTIMER 1.0f

#define	CROWBAR_RANGE	75.0f //range copied from crowbar, hence the name

class CWeaponHealthkit : public CHL2MPMachineGun
{
public:
	DECLARE_CLASS( CWeaponHealthkit, CHL2MPMachineGun );

	CWeaponHealthkit();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	
	void	Precache( void );
	void	AddViewKick( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );

	int		GetMinBurst() { return 2; }
	int		GetMaxBurst() { return 5; }

	virtual void Equip( CBaseCombatCharacter *pOwner );
	bool	Reload( void );
#ifndef CLIENT_DLL
	float	GetFireRate( void )
	{
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		CHL2_Player *pPlayerRepose = dynamic_cast<CHL2_Player*>(pPlayer); //cast to CHL2_Player since CBasePlayer doesn't access Repose stats

		if (pPlayer == NULL || pPlayerRepose == NULL)
			return HEALTHKIT_RATE_BASE * 10.0f; //return a really slow firerate so we can tell that something's not right
		//define our rate of fire
		float rate = HEALTHKIT_RATE_BASE;
		int mod = pPlayerRepose->checkMod(CReposeStats::DEX);
		mod > 0 ? rate -= (HEALTHKIT_RATE_POSITIVE * mod) : rate -= (HEALTHKIT_RATE_NEGATIVE * mod);
		return rate;
	}	

	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
#endif // !CLIENT_DLL

	Activity	GetPrimaryAttackActivity( void );

	virtual const Vector& GetBulletSpread( void )
	{
		static const Vector cone = Vector(0.0f, 0.0f, 0.0f);//= VECTOR_CONE_5DEGREES;
		return cone;
	}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

#ifndef CLIENT_DLL
	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();
#endif

protected:

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
	
private:
	CWeaponHealthkit( const CWeaponHealthkit & );
	void PlayDenySound(float);
	float m_flNextDenySound = 0.0f;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponHealthkit, DT_WeaponHealthkit )

BEGIN_NETWORK_TABLE( CWeaponHealthkit, DT_WeaponHealthkit )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponHealthkit )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_healthkit, CWeaponHealthkit );
PRECACHE_WEAPON_REGISTER(weapon_healthkit);

#ifndef CLIENT_DLL
acttable_t	CWeaponHealthkit::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_SLAM,					false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_SLAM,						false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_SLAM,				false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_SLAM,				false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SLAM,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_SLAM,			false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_SLAM,					false },
	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_SLAM,					false },

	//NPC
	//{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, true },
	{ ACT_RELOAD, ACT_RELOAD_SMG1, true },
	{ ACT_IDLE, ACT_IDLE_SMG1, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SMG1, true },

	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SMG1_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

	{ ACT_WALK_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_STIMULATED, ACT_WALK_RIFLE_STIMULATED, false },
	{ ACT_WALK_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_STIMULATED, ACT_RUN_RIFLE_STIMULATED, false },
	{ ACT_RUN_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED, ACT_IDLE_AIM_RIFLE_STIMULATED, false },
	{ ACT_IDLE_AIM_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

	{ ACT_WALK_AIM_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_AIM_STIMULATED, ACT_WALK_AIM_RIFLE_STIMULATED, false },
	{ ACT_WALK_AIM_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_AIM_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_AIM_STIMULATED, ACT_RUN_AIM_RIFLE_STIMULATED, false },
	{ ACT_RUN_AIM_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims
	//End readiness activities

	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_RIFLE, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SMG1, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SMG1_LOW, true },
	{ ACT_COVER_LOW, ACT_COVER_SMG1_LOW, false },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_SMG1_LOW, false },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SMG1_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, true },
};

IMPLEMENT_ACTTABLE(CWeaponHealthkit);
#endif

//=========================================================
CWeaponHealthkit::CWeaponHealthkit( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1 = CROWBAR_RANGE;//1400
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHealthkit::Precache( void )
{
#ifndef CLIENT_DLL
//	UTIL_PrecacheOther("grenade_ar2");
#endif

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponHealthkit::Equip( CBaseCombatCharacter *pOwner )
{
	m_fMaxRange1 = CROWBAR_RANGE;

	BaseClass::Equip( pOwner );
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHealthkit::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir)
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	//WeaponSoundRealtime(SINGLE_NPC);

	/*CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());
	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0);*/

	//pOperator->DoMuzzleFlash();
	//m_iClip1 = m_iClip1 - 1;
	return;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHealthkit::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SMG1:
	{
		Vector vecShootOrigin, vecShootDir;
		QAngle angDiscard;

		// Support old style attachment point firing
		if ((pEvent->options == NULL) || (pEvent->options[0] == '\0') || (!pOperator->GetAttachment(pEvent->options, vecShootOrigin, angDiscard)))
		{
			vecShootOrigin = pOperator->Weapon_ShootPosition();
		}

		CAI_BaseNPC *npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);
		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

		FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
	}
	break;
	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponHealthkit::GetPrimaryAttackActivity( void )
{
	/*if ( m_nShotsFired < 2 )
		return ACT_VM_PRIMARYATTACK;

	if ( m_nShotsFired < 3 )
		return ACT_VM_RECOIL1;
	
	if ( m_nShotsFired < 4 )
		return ACT_VM_RECOIL2;*/

	return ACT_VM_PRIMARYATTACK;// ACT_VM_RECOIL3;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponHealthkit::Reload( void )
{
	bool fRet;
	float fCacheTime = m_flNextSecondaryAttack;

	fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
	if ( fRet )
	{
		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;

		WeaponSound( RELOAD );
	}

	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHealthkit::AddViewKick( void )
{
	//#define	EASY_DAMPEN			0.5f
	//#define	MAX_VERTICAL_KICK	1.0f	//Degrees
	//#define	SLIDE_LIMIT			2.0f	//Seconds
	
	//Get the view kick
	//CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	//if ( pPlayer == NULL )
		return;

	//DoMachineGunKick( pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT );
}
//-----------------------------------------------------------------------------
// Purpose: Healing self
//-----------------------------------------------------------------------------
void CWeaponHealthkit::PrimaryAttack( void )
{

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	// Abort here to handle burst and auto fire modes
	if ((UsesClipsForAmmo1() && m_iClip1 == 0) || (!UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType)))
		return;

	//We can't heal ourself if our health is full
	if (pPlayer->HealthFraction() >= 1.0f)
	{
		PlayDenySound(HEALTHKIT_DENY_SOUNDTIMER);
		return;
	}
#ifndef CLIENT_DLL
	m_nShotsFired++;

	//pPlayer->DoMuzzleFlash();

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	int iBulletsToFire = 0;
	float fireRate = GetFireRate();


	while (m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		// MUST call sound before removing a round from the clip of a CHLMachineGun
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		iBulletsToFire++;
	}

	//Make sure we don't try to use more healing than we need
	while (pPlayer->m_iHealth + iBulletsToFire > pPlayer->m_iMaxHealth && iBulletsToFire > 0)
		iBulletsToFire--;

	// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
	if (UsesClipsForAmmo1())
	{
		if (iBulletsToFire > m_iClip1)
			iBulletsToFire = m_iClip1;
		m_iClip1 -= iBulletsToFire;
	}
	else
	{
		if (iBulletsToFire > pPlayer->GetAmmoCount(m_iPrimaryAmmoType))
			iBulletsToFire = pPlayer->GetAmmoCount(m_iPrimaryAmmoType);
		pPlayer->RemoveAmmo(iBulletsToFire, m_iPrimaryAmmoType);
	}

	// Fire the "bullets"
	pPlayer->TakeHealth(iBulletsToFire, DMG_GENERIC);

	//Factor in the view kick
	//AddViewKick();

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}
#endif // !CLIENT_DLL
	SendWeaponAnim(GetPrimaryAttackActivity());
	pPlayer->SetAnimation(PLAYER_ATTACK1);
}

//-----------------------------------------------------------------------------
// Purpose: Healing other
//-----------------------------------------------------------------------------
void CWeaponHealthkit::SecondaryAttack(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	// Make sure we have a valid target in range
	CHL2MP_Player *pHL2MPPlayer = ToHL2MPPlayer(pPlayer);
	Vector m_vecSrc = pHL2MPPlayer->Weapon_ShootPosition();
	Vector m_vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
	float m_flDistance = CROWBAR_RANGE;
	trace_t check;
	UTIL_TraceLine(m_vecSrc, m_vecSrc + m_vecDirShooting * m_flDistance, MASK_ALL, pPlayer, COLLISION_GROUP_PLAYER, &check);

	if (!check.m_pEnt) //we're aiming at absolutely nothing
	{
		PlayDenySound(HEALTHKIT_DENY_SOUNDTIMER);
		return;
	}

	CBaseCombatCharacter *pTarget = dynamic_cast<CBaseCombatCharacter*>(check.m_pEnt);
	if (pTarget == NULL) //we're aiming at something we can't heal
	{
		PlayDenySound(HEALTHKIT_DENY_SOUNDTIMER);
		return;
	}



	// Abort here to handle burst and auto fire modes
	if ((UsesClipsForAmmo1() && m_iClip1 == 0) || (!UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType)))
		return;

	//We can't heal our target if their health is full
	if (pTarget->HealthFraction() >= 1.0f)
	{
		PlayDenySound(HEALTHKIT_DENY_SOUNDTIMER);
		return;
	}
#ifndef CLIENT_DLL
	m_nShotsFired++;

	//pPlayer->DoMuzzleFlash();

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	int iBulletsToFire = 0;
	float fireRate = GetFireRate();


	while (m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		// MUST call sound before removing a round from the clip of a CHLMachineGun
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		iBulletsToFire++;
	}

	//Make sure we don't try to use more healing than we need
	while (pTarget->m_iHealth + iBulletsToFire > pTarget->m_iMaxHealth && iBulletsToFire > 0)
		iBulletsToFire--;

	// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
	if (UsesClipsForAmmo1())
	{
		if (iBulletsToFire > m_iClip1)
			iBulletsToFire = m_iClip1;
		m_iClip1 -= iBulletsToFire;
	}
	else
	{
		if (iBulletsToFire > pPlayer->GetAmmoCount(m_iPrimaryAmmoType))
			iBulletsToFire = pPlayer->GetAmmoCount(m_iPrimaryAmmoType);
		pPlayer->RemoveAmmo(iBulletsToFire, m_iPrimaryAmmoType);
	}
	// Fire the "bullets"
	pTarget->TakeHealth(iBulletsToFire, DMG_GENERIC);

	//Factor in the view kick
	//AddViewKick();

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}
#endif // !CLIENT_DLL
	SendWeaponAnim(ACT_VM_SECONDARYATTACK);
	pPlayer->SetAnimation(PLAYER_ATTACK1);
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponHealthkit::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75	},
		{ 5.00,		0.75	},
		{ 10.0/3.0, 0.75	},
		{ 5.0/3.0,	0.75	},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}

//-----------------------------------------------------------------------------
// Purpose: Deny sound if we don't have a valid heal target
//-----------------------------------------------------------------------------
void CWeaponHealthkit::PlayDenySound(float flDelay)
{
	if (gpGlobals->curtime > m_flNextDenySound)
	{
		m_flNextDenySound = gpGlobals->curtime + flDelay; //don't let us play a sound again until we've expired our delay
		EmitSound("Weapon_Healthkit.Empty");
	}
}