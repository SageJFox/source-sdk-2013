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
#define HEALTHKIT_MAXAMMO_MOD 5

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
	bool StartReload(void);
	bool Reload( void );
	void FillClip( void );
	void FinishReload( void );
	void CheckHolsterReload( void );
	void ItemPostFrame( void );

	float	GetFireRate( void )
	{
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
#ifdef CLIENT_DLL
		C_HL2MP_Player* pPlayerStats = dynamic_cast<C_HL2MP_Player*>(pPlayer);
#else
		CHL2MP_Player* pPlayerStats = dynamic_cast<CHL2MP_Player*>(pPlayer);
#endif // CLIENT_DLL

		if (pPlayer == NULL || pPlayerStats == NULL)
			return HEALTHKIT_RATE_BASE * 10.0f; //return a really slow firerate so we can tell that something's not right
		//define our rate of fire
		float rate = HEALTHKIT_RATE_BASE;
		int mod = pPlayerStats->checkMod(INT);
		mod > 0 ? rate -= (HEALTHKIT_RATE_POSITIVE * mod) : rate -= (HEALTHKIT_RATE_NEGATIVE * mod);
		return rate;
	}	
#ifndef CLIENT_DLL
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
	CNetworkVar(bool, m_bDelayedFire1);	// Fire primary when finished reloading
	CNetworkVar(bool, m_bDelayedFire2);	// Fire secondary when finished reloading
	CNetworkVar(bool, m_bDelayedReload);	// Reload when finished pump
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponHealthkit, DT_WeaponHealthkit )

BEGIN_NETWORK_TABLE( CWeaponHealthkit, DT_WeaponHealthkit )
#ifdef CLIENT_DLL
RecvPropBool(RECVINFO(m_bDelayedFire1)),
RecvPropBool(RECVINFO(m_bDelayedFire2)),
RecvPropBool(RECVINFO(m_bDelayedReload)),
#else
SendPropBool(SENDINFO(m_bDelayedFire1)),
SendPropBool(SENDINFO(m_bDelayedFire2)),
SendPropBool(SENDINFO(m_bDelayedReload)),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponHealthkit)
DEFINE_PRED_FIELD(m_bDelayedFire1, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_bDelayedFire2, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_bDelayedReload, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

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
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_SMG1,			false },
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
	m_fMinRange1		= 0; //No minimum range. 
	m_fMaxRange1 = CROWBAR_RANGE; //1400;
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
#ifndef CLIENT_DLL
	if (!pPlayer->IsOrganic()) //can only heal organics
		return;
#endif // !CLIENT_DLL
	// Abort here to handle burst and auto fire modes
	if ((UsesClipsForAmmo1() && m_iClip1 == 0) || (!UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType)))
		return;

	//We can't heal ourself if our health is full
	if (pPlayer->HealthFraction() >= 1.0f)
		return PlayDenySound(HEALTHKIT_DENY_SOUNDTIMER);
	else m_flNextDenySound = gpGlobals->curtime + 0.01f + GetFireRate();
#ifndef CLIENT_DLL
	//m_nShotsFired++;

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
	Vector m_vecDirShooting = pPlayer->GetAutoaimVector(0.0f); //AUTOAIM_5DEGREES
	float m_flDistance = CROWBAR_RANGE;
	trace_t check;
	UTIL_TraceLine(m_vecSrc, m_vecSrc + m_vecDirShooting * m_flDistance, MASK_ALL, pPlayer, COLLISION_GROUP_PLAYER, &check);

	if (!check.m_pEnt) //we're aiming at absolutely nothing
		return PlayDenySound(HEALTHKIT_DENY_SOUNDTIMER);

	CBaseCombatCharacter *pTarget = dynamic_cast<CBaseCombatCharacter*>(check.m_pEnt);
	if (pTarget == NULL) //we're aiming at something we can't heal
		return PlayDenySound(HEALTHKIT_DENY_SOUNDTIMER);
#ifndef CLIENT_DLL
	if (!pTarget->IsOrganic()) //our target isn't organic, we can't heal them
		return PlayDenySound(HEALTHKIT_DENY_SOUNDTIMER);
#endif // !CLIENT_DLL

	// Abort here to handle burst and auto fire modes
	if ((UsesClipsForAmmo1() && m_iClip1 == 0) || (!UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType)))
		return;

	//We can't heal our target if their health is full
	if (pTarget->HealthFraction() >= 1.0f)
		return PlayDenySound(HEALTHKIT_DENY_SOUNDTIMER);
	else 
		m_flNextDenySound = gpGlobals->curtime + 0.01f + GetFireRate();
#ifndef CLIENT_DLL
	//m_nShotsFired++;

	//pPlayer->DoMuzzleFlash();

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	int iBulletsToFire = 0;
	float fireRate = GetFireRate();

	/*TODO: This fixes our problem of our secondary attack sometimes
	counting up hundreds of bullets to fire (before capping itself at the max
	healing needing to be done) but it potentially ties firerate to framerate*/
	if ((m_flNextPrimaryAttack < gpGlobals->curtime))
		m_flNextPrimaryAttack = gpGlobals->curtime;

	while (m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		// MUST call sound before removing a round from the clip of a CHLMachineGun
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		iBulletsToFire++;
		//DevMsg("Bulets to fire is %i\n", iBulletsToFire);
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
	pTarget->SetUpset(false, pHL2MPPlayer, 2.0f, 12, 17); //take healing a target as a sign of trying to calm them (moderately difficult because we shouldn't've been hurt to begin with)
	//Factor in the view kick
	//AddViewKick();
	Clip1();

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
#ifndef CLIENT_DLL
	if (gpGlobals->curtime > m_flNextDenySound)
	{
		m_flNextDenySound = gpGlobals->curtime + flDelay; //don't let us play a sound again until we've expired our delay
		//DevMsg("[Deny Sound] at %.2f\n",m_flNextDenySound);

		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		if (!pPlayer)
			return;
		const char* szSoundName = "Weapon_Healthkit.Empty";
		CHL2MP_Player *pHL2MPPlayer = ToHL2MPPlayer(pPlayer);
		Vector m_vecSrc = pHL2MPPlayer->Weapon_ShootPosition();
		CRecipientFilter filter;
		filter.AddRecipientsByPAS(m_vecSrc);
		IRecipientFilter& rFilter = filter;
		//this is so fucking stupid
		float flZero = 0.0f;
		float* pZero = &flZero;
		
		EmitSound(rFilter, entindex(), szSoundName, &m_vecSrc, 0.0f, pZero);
		SendWeaponAnim(ACT_VM_DRYFIRE);
	}
#endif
	return;
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponHealthkit::StartReload(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;


	int j = MIN(1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));

	if (j <= 0)
		return false;

	SendWeaponAnim(ACT_SHOTGUN_RELOAD_START);

	// Make shotgun shell visible
	SetBodygroup(1, 0);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

	m_bInReload = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponHealthkit::Reload(void)
{
	// Check that StartReload was called first
	if (!m_bInReload)
	{
		Warning("ERROR: Shotgun Reload called incorrectly!\n");
	}

	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;

	int j = MIN(1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));

	if (j <= 0)
		return false;

	FillClip();
	// Play reload on different channel as otherwise steals channel away from fire sound
	WeaponSound(RELOAD);
	//get our fast reload in based on curtime (tenths of second) and DEX
	float flCurTime = (gpGlobals->curtime - floor(gpGlobals->curtime)) * 10;
#ifdef CLIENT_DLL
	C_HL2MP_Player* pOwnerStats = dynamic_cast<C_HL2MP_Player*>(pOwner);
#else
	CHL2MP_Player* pOwnerStats = dynamic_cast<CHL2MP_Player*>(pOwner);
#endif // CLIENT_DLL
	if (pOwnerStats && 3 + pOwnerStats->checkMod(DEX) >= (int)floor(flCurTime))
		SendWeaponAnim(ACT_SHOTGUN_PUMP); //used as a faster reload
	else
		SendWeaponAnim(ACT_VM_RELOAD);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponHealthkit::FinishReload(void)
{
	// Make shotgun shell invisible
	SetBodygroup(1, 1);

	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	m_bInReload = false;

	// Finish reload animation
	SendWeaponAnim(ACT_SHOTGUN_RELOAD_FINISH);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponHealthkit::FillClip(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	// Add them to the clip
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) > 0)
	{
		if (Clip1() < GetMaxClip1())
		{
			m_iClip1++;
			pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Override so shotgun can do mulitple reloads in a row
//-----------------------------------------------------------------------------
void CWeaponHealthkit::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
	{
		return;
	}

	if (m_bInReload)
	{
		// If I'm primary firing and have 5 rounds stop reloading and fire
		if ((pOwner->m_nButtons & IN_ATTACK) && (m_iClip1 >= 5))
		{
			m_bInReload = false;
			m_bDelayedFire1 = true;
		}
		// If I'm secondary firing and have 5 rounds stop reloading and fire
		else if ((pOwner->m_nButtons & IN_ATTACK2) && (m_iClip1 >= 5))
		{
			m_bInReload = false;
			m_bDelayedFire2 = true;
		}
		else if (m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
			// If out of ammo end reload
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				FinishReload();
				return;
			}
			// If clip not full reload again
			if (m_iClip1 < GetMaxClip1())
			{
				Reload();
				return;
			}
			// Clip full, stop reloading
			else
			{
				FinishReload();
				return;
			}
		}
	}
	else
	{
		// Make shotgun shell invisible
		SetBodygroup(1, 1);
	}

	// Shotgun uses same timing and ammo for secondary attack
	if ((m_bDelayedFire2 || pOwner->m_nButtons & IN_ATTACK2) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		m_bDelayedFire2 = false;

		if ((m_iClip1 <= 0 && UsesClipsForAmmo1()))
		{
			if (!pOwner->GetAmmoCount(m_iPrimaryAmmoType))
				PlayDenySound(HEALTHKIT_DENY_SOUNDTIMER);
			else
				StartReload();
		}
		else
		{
			// If the firing button was just pressed, reset the firing time
			if (pOwner->m_afButtonPressed & IN_ATTACK)
			{
				m_flNextPrimaryAttack = gpGlobals->curtime;
			}
			SecondaryAttack();
		}
	}
	else if ((m_bDelayedFire1 || pOwner->m_nButtons & IN_ATTACK) && m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		m_bDelayedFire1 = false;
		if ((m_iClip1 <= 0 && UsesClipsForAmmo1()) || (!UsesClipsForAmmo1() && !pOwner->GetAmmoCount(m_iPrimaryAmmoType)))
		{
			if (!pOwner->GetAmmoCount(m_iPrimaryAmmoType))
				PlayDenySound(HEALTHKIT_DENY_SOUNDTIMER);
			else
				StartReload();
		}
		else
		{
			// If the firing button was just pressed, reset the firing time
			CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
			if (pPlayer && pPlayer->m_afButtonPressed & IN_ATTACK)
			{
				m_flNextPrimaryAttack = gpGlobals->curtime;
			}
			PrimaryAttack();
		}
	}

	if (pOwner->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		StartReload();
	}
	else
	{
		// no fire buttons down
		m_bFireOnEmpty = false;

		if (!HasAnyAmmo() && m_flNextPrimaryAttack < gpGlobals->curtime)
		{
			// weapon isn't useable, switch.
			if (!(GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && pOwner->SwitchToNextBestWeapon(this))
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
				return;
			}
		}
		else
		{
			// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
			if (m_iClip1 <= 0 && !(GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < gpGlobals->curtime)
			{
				if (StartReload())
				{
					// if we've successfully started to reload, we're done
					return;
				}
			}
		}

		WeaponIdle();
		return;
	}

}
