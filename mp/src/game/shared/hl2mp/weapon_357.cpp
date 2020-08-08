
//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
#endif

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define CWeapon357 C_Weapon357
#endif

//-----------------------------------------------------------------------------
// CWeapon357
//-----------------------------------------------------------------------------

class CWeapon357 : public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeapon357, CBaseHL2MPCombatWeapon );
public:

	CWeapon357( void );

	void	PrimaryAttack( void );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	DECLARE_ACTTABLE();
#endif

private:
	
	CWeapon357( const CWeapon357 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( Weapon357, DT_Weapon357 )

BEGIN_NETWORK_TABLE( CWeapon357, DT_Weapon357 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeapon357 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_357, CWeapon357 );
PRECACHE_WEAPON_REGISTER( weapon_357 );


#ifndef CLIENT_DLL
acttable_t CWeapon357::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_PISTOL,					false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_PISTOL,					false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_PISTOL,			false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_PISTOL,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_PISTOL,		false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_PISTOL,					false },
	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_PISTOL,				false },
	//NPC
	{ ACT_IDLE, ACT_IDLE_PISTOL, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_PISTOL, true },
	//{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_PISTOL, true },
	{ ACT_RELOAD, ACT_RELOAD_PISTOL, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_PISTOL, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_PISTOL, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_PISTOL, true },
	{ ACT_RELOAD_LOW, ACT_RELOAD_PISTOL_LOW, false },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_PISTOL_LOW, false },
	{ ACT_COVER_LOW, ACT_COVER_PISTOL_LOW, false },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_PISTOL_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_PISTOL, false },
	{ ACT_WALK, ACT_WALK_PISTOL, false },
	{ ACT_RUN, ACT_RUN_PISTOL, false },
};



IMPLEMENT_ACTTABLE( CWeapon357 );

#endif

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon357::CWeapon357( void )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeapon357::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		CBaseCombatCharacter* pOwner = GetOwner();

		if (!pOwner)
			return;

		pOwner->DoMuzzleFlash();

		SendWeaponAnim(GetPrimaryAttackActivity());

		// player "shoot" animation
		pOwner->SetSequence(ACT_RANGE_ATTACK1);

		FireBulletsInfo_t info;
#ifndef CLIENT_DLL
		info.m_vecSrc = pOwner->Weapon_ShootPosition();
#else
		info.m_vecSrc = pOwner->EyePosition();
#endif // !CLIENT_DLL

		info.m_vecDirShooting = pOwner->EyePosition(); //pOwner->EyeAngles();

		info.m_iShots = 1;

		// Make sure we don't fire more than the amount in the clip
		if (UsesClipsForAmmo1())
		{
			info.m_iShots = MIN(info.m_iShots, m_iClip1);
			m_iClip1 -= info.m_iShots;
		}
		else
		{
			info.m_iShots = MIN(info.m_iShots, pOwner->GetAmmoCount(m_iPrimaryAmmoType));
			pOwner->RemoveAmmo(info.m_iShots, m_iPrimaryAmmoType);
		}

		info.m_flDistance = MAX_TRACE_LENGTH;
		info.m_iAmmoType = m_iPrimaryAmmoType;
		info.m_iTracerFreq = 2;

#if !defined( CLIENT_DLL )
		// Fire the bullets
		info.m_vecSpread = pOwner->GetAttackSpread(this);
#else
		//!!!HACKHACK - what does the client want this function for? 
		info.m_vecSpread = GetActiveWeapon()->GetBulletSpread();
#endif // CLIENT_DLL

		pOwner->FireBullets(info);
		return;
	}

	if ( m_iClip1 <= 0 )
	{
		if ( !m_bFireOnEmpty )
		{
			Reload();
		}
		else
		{
			WeaponSound( EMPTY );
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	WeaponSound( SINGLE );
	pPlayer->DoMuzzleFlash();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.75;
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.75;

	m_iClip1--;

	Vector vecSrc		= pPlayer->Weapon_ShootPosition();
	Vector vecAiming	= pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );	

	Vector vBulletSpread = vec3_origin;
	CHL2MP_Player* pOwner = dynamic_cast<CHL2MP_Player*>(GetOwner());
	if (pOwner)
	{
		float base = 3.0f; //3 degrees
		int dex = pOwner->checkMod(DEX) - 1; // - 1 since we need at least a +1 mod to pick this up
		base -= (float)dex / 2.0f;
		base = Max(0.0f, base);
		base = sin(DEG2RAD(base / 2.0f));
		vBulletSpread = Vector(base, base, base);
	}

	FireBulletsInfo_t info( 1, vecSrc, vecAiming, vBulletSpread, MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
	info.m_pAttacker = pPlayer;

	// Fire the bullets, and force the first shot to be perfectly accuracy
	pPlayer->FireBullets( info );

	//Disorient the player
	QAngle angles = pPlayer->GetLocalAngles();

	angles.x += random->RandomInt( -1, 1 );
	angles.y += random->RandomInt( -1, 1 );
	angles.z = 0;

#ifndef CLIENT_DLL
	pPlayer->SnapEyeAngles( angles );
#endif

	pPlayer->ViewPunch( QAngle( -8, random->RandomFloat( -2, 2 ), 0 ) );

	if ( !m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 ); 
	}
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon357::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_PISTOL_FIRE:
	{
		Vector vecShootOrigin, vecShootDir;
		vecShootOrigin = pOperator->Weapon_ShootPosition();

		CAI_BaseNPC *npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);

		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

		CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

		WeaponSound(SINGLE_NPC);
		pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
		pOperator->DoMuzzleFlash();
		m_iClip1 = m_iClip1 - 1;
	}
	break;
	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
#endif // !CLIENT_DLL