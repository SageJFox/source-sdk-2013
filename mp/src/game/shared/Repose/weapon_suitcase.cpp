//=============================================================================//
//
// Purpose: defines weapons/items related to player finds.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_hl2mpbasebasebludgeon.h"
#ifndef CLIENT_DLL
#include "entityoutput.h"
#include "items.h"
#include "finds.h"
#endif // !CLIENT_DLL

#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_hl2mp_player.h"
#include "c_te_effect_dispatch.h"
#else
#include "hl2mp_player.h"
#include "te_effect_dispatch.h"
#endif

#include "weapon_ar2.h"
#include "effect_dispatch_data.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define GRENADE_TIMER	2.5f //Seconds

#define GRENADE_PAUSED_NO			0
#define GRENADE_PAUSED_PRIMARY		1
#define GRENADE_PAUSED_SECONDARY	2

#define GRENADE_RADIUS	4.0f // inches

#define GRENADE_DAMAGE_RADIUS 250.0f

#define MAX_FINDS 3
ConVar sk_max_find("sk_max_find", "3");


#ifdef CLIENT_DLL
#define CWeaponSuitcase C_WeaponSuitcase
#define CWeaponFind C_WeaponFind
#endif

//-----------------------------------------------------------------------------
//	Briefcase weapon. Negotiator carries this to organize their finds for meetup with the base boss.
//-----------------------------------------------------------------------------

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

	void InputAdd(int);
	void InputSubtract(int);
	void InputGetValue(int);

#ifndef CLIENT_DLL
private:
	int m_nMin = 0;		// Minimum clamp value. If min and max are BOTH zero, no clamping is done.
	int m_nMax[FIND_COUNT];		// Maximum clamp value.
	bool m_bHitMin[FIND_COUNT];		// Set when we reach or go below our minimum value, cleared if we go above it again.
	bool m_bHitMax[FIND_COUNT];		// Set when we reach or exceed our maximum value, cleared if we fall below it again.

	bool KeyValue(const char *szKeyName, const char *szValue);
	void Spawn(void);

	int DrawDebugTextOverlays(void);

	void UpdateOutValue(CBaseEntity *pActivator, int find, int nNewValue);
	bool InputAddRandom(CBaseEntity *pActivator);

	// Inputs
	void InputAdd(inputdata_t &inputdata);
	void InputSubtract(inputdata_t &inputdata);
	void InputGetValue(inputdata_t &inputdata);

	// Outputs
	COutputInt m_OutValue[FIND_COUNT] = {};
	COutputInt m_OnGetValue[FIND_COUNT] = {};	// Used for polling the counter value.
	COutputEvent m_OnHitMin[FIND_COUNT];
	COutputEvent m_OnHitMax[FIND_COUNT];

	DECLARE_DATADESC();
#endif // !CLIENT_DLL
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



BEGIN_DATADESC(CWeaponSuitcase)

DEFINE_FIELD(m_bHitMax, FIELD_BOOLEAN),
DEFINE_FIELD(m_bHitMin, FIELD_BOOLEAN),

// Keys


// Inputs
DEFINE_INPUTFUNC(FIELD_INTEGER, "Add", InputAdd),
DEFINE_INPUTFUNC(FIELD_INTEGER, "Subtract", InputSubtract),
DEFINE_INPUTFUNC(FIELD_INTEGER, "GetValue", InputGetValue),

// Outputs

DEFINE_OUTPUT(m_OutValue[FIND_BLACKMAIL_INCONVENIENT], "OutValue_BlackMailSmall"),
DEFINE_OUTPUT(m_OnHitMin[FIND_BLACKMAIL_INCONVENIENT], "OnHitMin_BlackMailSmall"),
DEFINE_OUTPUT(m_OnHitMax[FIND_BLACKMAIL_INCONVENIENT], "OnHitMax_BlackMailSmall"),
DEFINE_OUTPUT(m_OnGetValue[FIND_BLACKMAIL_INCONVENIENT], "OnGetValue_BlackMailSmall"),

DEFINE_OUTPUT(m_OutValue[FIND_BLACKMAIL_DAMNING], "OutValue_BlackMailBig"),
DEFINE_OUTPUT(m_OnHitMin[FIND_BLACKMAIL_DAMNING], "OnHitMin_BlackMailBig"),
DEFINE_OUTPUT(m_OnHitMax[FIND_BLACKMAIL_DAMNING], "OnHitMax_BlackMailBig"),
DEFINE_OUTPUT(m_OnGetValue[FIND_BLACKMAIL_DAMNING], "OnGetValue_BlackMailBig"),

DEFINE_OUTPUT(m_OutValue[FIND_SECRET_JUICY], "OutValue_SecretSmall"),
DEFINE_OUTPUT(m_OnHitMin[FIND_SECRET_JUICY], "OnHitMin_SecretSmall"),
DEFINE_OUTPUT(m_OnHitMax[FIND_SECRET_JUICY], "OnHitMax_SecretSmall"),
DEFINE_OUTPUT(m_OnGetValue[FIND_SECRET_JUICY], "OnGetValue_SecretSmall"),

DEFINE_OUTPUT(m_OutValue[FIND_SECRET_DANGEROUS], "OutValue_SecretBig"),
DEFINE_OUTPUT(m_OnHitMin[FIND_SECRET_DANGEROUS], "OnHitMin_SecretBig"),
DEFINE_OUTPUT(m_OnHitMax[FIND_SECRET_DANGEROUS], "OnHitMax_SecretBig"),
DEFINE_OUTPUT(m_OnGetValue[FIND_SECRET_DANGEROUS], "OnGetValue_SecretBig"),

DEFINE_OUTPUT(m_OutValue[FIND_LIE_LITTLE], "OutValue_AdvLieSmall"),
DEFINE_OUTPUT(m_OnHitMin[FIND_LIE_LITTLE], "OnHitMin_AdvLieSmall"),
DEFINE_OUTPUT(m_OnHitMax[FIND_LIE_LITTLE], "OnHitMax_AdvLieSmall"),
DEFINE_OUTPUT(m_OnGetValue[FIND_LIE_LITTLE], "OnGetValue_AdvLieSmall"),

DEFINE_OUTPUT(m_OutValue[FIND_LIE_BOLDFACED], "OutValue_AdvLieBig"),
DEFINE_OUTPUT(m_OnHitMin[FIND_LIE_BOLDFACED], "OnHitMin_AdvLieBig"),
DEFINE_OUTPUT(m_OnHitMax[FIND_LIE_BOLDFACED], "OnHitMax_AdvLieBig"),
DEFINE_OUTPUT(m_OnGetValue[FIND_LIE_BOLDFACED], "OnGetValue_AdvLieBig"),

DEFINE_OUTPUT(m_OutValue[FIND_SOUND_ADVICE], "OutValue_AdvAdvice"),
DEFINE_OUTPUT(m_OnHitMin[FIND_SOUND_ADVICE], "OnHitMin_AdvAdvice"),
DEFINE_OUTPUT(m_OnHitMax[FIND_SOUND_ADVICE], "OnHitMax_AdvAdvice"),
DEFINE_OUTPUT(m_OnGetValue[FIND_SOUND_ADVICE], "OnGetValue_AdvAdvice"),

DEFINE_OUTPUT(m_OutValue[FIND_THREATENING_ACTION], "OutValue_AdvThreat"),
DEFINE_OUTPUT(m_OnHitMin[FIND_THREATENING_ACTION], "OnHitMin_AdvThreat"),
DEFINE_OUTPUT(m_OnHitMax[FIND_THREATENING_ACTION], "OnHitMax_AdvThreat"),
DEFINE_OUTPUT(m_OnGetValue[FIND_THREATENING_ACTION], "OnGetValue_AdvThreat"),

DEFINE_OUTPUT(m_OutValue[FIND_READ_TARGET], "OutValue_AdvRead"),
DEFINE_OUTPUT(m_OnHitMin[FIND_READ_TARGET], "OnHitMin_AdvRead"),
DEFINE_OUTPUT(m_OnHitMax[FIND_READ_TARGET], "OnHitMax_AdvRead"),
DEFINE_OUTPUT(m_OnGetValue[FIND_READ_TARGET], "OnGetValue_AdvRead"),

END_DATADESC()
//#endif // !CLIENT_DLL
//#ifndef CLIENT_DLL

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

//#endif
//#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Handles key values from the BSP before spawn is called.
//-----------------------------------------------------------------------------

bool CWeaponSuitcase::KeyValue(const char *szKeyName, const char *szValue)
{
	//
	// Setup initial finds.
	//
	if (!stricmp(szKeyName, "startvalue"))
	{
		// Maximum clamp value.
		m_nMax[FIND_BLACKMAIL_INCONVENIENT] = 10;
		m_nMax[FIND_BLACKMAIL_DAMNING] = 10;
		m_nMax[FIND_SECRET_JUICY] = 10;
		m_nMax[FIND_SECRET_DANGEROUS] = 10;
		m_nMax[FIND_LIE_LITTLE] = 10;
		m_nMax[FIND_LIE_BOLDFACED] = 10;
		m_nMax[FIND_SOUND_ADVICE] = 2;
		m_nMax[FIND_THREATENING_ACTION] = 1;
		m_nMax[FIND_READ_TARGET] = 2;

		for (int i = atoi(szValue); i > 0; i--)
			InputAddRandom(this);
		return(true);
	}

	return(BaseClass::KeyValue(szKeyName, szValue));
}


//-----------------------------------------------------------------------------
// Purpose: Called before spawning, after key values have been set.
//-----------------------------------------------------------------------------
void CWeaponSuitcase::Spawn(void)
{
	//
	// Make sure max and min are ordered properly or clamp won't work.
	//
	for (int i = 0; i < FIND_COUNT; i++)
	{
		if (m_nMin > m_nMax[i])
		{
			int nTemp = m_nMax[i];
			m_nMax[i] = m_nMin;
			m_nMin = nTemp;
		}

		//
		// Clamp initial value to within the valid range.
		//
		if ((m_nMin != 0) || (m_nMax != 0))
		{
			int nStartValue = clamp(m_OutValue[i].Get(), m_nMin, m_nMax[i]);
			m_OutValue[i].Init(nStartValue);
		}
	}
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Draw any debug text overlays
// Input  :
// Output : Current text offset from the top
//-----------------------------------------------------------------------------
int CWeaponSuitcase::DrawDebugTextOverlays(void)
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		char tempstr[512];
		for (int i = 0; i < FIND_COUNT; i++)
		{
			Q_snprintf(tempstr, sizeof(tempstr), "find %n: %n/%n", i, m_OutValue[i].Get(), m_nMax);
			EntityText(text_offset, tempstr, 0);
			text_offset++;
		}
		EntityText(text_offset, tempstr, 0);
		text_offset++;

	}
	return text_offset;
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for adding to the accumulator value.
// Input  : Integer value to add.
//-----------------------------------------------------------------------------
void CWeaponSuitcase::InputAdd(inputdata_t &inputdata)
{
	if (inputdata.value.Int() < 0) //-1 is our random find
	{
		InputAddRandom(inputdata.pActivator);
		return;
	}
	int nNewValue = m_OutValue[inputdata.value.Int()].Get();
	nNewValue++;
	if (nNewValue > m_nMax[inputdata.value.Int()])
		InputAddRandom(inputdata.pActivator);
	else
		UpdateOutValue(inputdata.pActivator, inputdata.value.Int(), nNewValue);
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for subtracting from the current value.
// Input  : Integer value to subtract.
//-----------------------------------------------------------------------------
void CWeaponSuitcase::InputSubtract(inputdata_t &inputdata)
{
	if (inputdata.value.Int() < 0) //-1 is our random find
	{
		DevMsg("Find Counter %s cannot subtract from a random find!\n", GetDebugName());
		return;
	}
	int nNewValue = m_OutValue[inputdata.value.Int()].Get();
	nNewValue--;
	if (nNewValue >= m_nMin)
		UpdateOutValue(inputdata.pActivator, inputdata.value.Int(), nNewValue);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponSuitcase::InputGetValue(inputdata_t &inputdata)
{
	int nOutValue = m_OutValue[inputdata.value.Int()].Get();
	m_OnGetValue[inputdata.value.Int()].Set(nOutValue, inputdata.pActivator, inputdata.pCaller);
}

//-----------------------------------------------------------------------------
// Purpose: Sets the value to the new value, clamping and firing the output value.
// Input  : fNewValue - Value to set.
//-----------------------------------------------------------------------------
void CWeaponSuitcase::UpdateOutValue(CBaseEntity *pActivator, int find, int nNewValue)
{
	if ((m_nMin != 0) || (m_nMax != 0))
	{
		// Fire an output any time we reach or exceed our maximum value.
		if (nNewValue >= m_nMax[find])
		{
			if (!m_bHitMax[find])
			{
				m_bHitMax[find] = true;
				m_OnHitMax[find].FireOutput(pActivator, this);
				//TODO: randomize finds of this type here
			}
		}
		else
		{
			m_bHitMax[find] = false;
			//TODO: de-randomize finds of this type here
		}
		

		// Fire an output any time we reach or go below our minimum value.
		if (nNewValue <= m_nMin)
		{
			if (!m_bHitMin[find])
			{
				m_bHitMin[find] = true;
				m_OnHitMin[find].FireOutput(pActivator, this);
			}
		}
		else
			m_bHitMin[find] = false;

		nNewValue = clamp(nNewValue, m_nMin, m_nMax[find]);
	}

	m_OutValue[find].Set(nNewValue, pActivator, this);
}

bool CWeaponSuitcase::InputAddRandom(CBaseEntity *pActivator)
{
	//first, get a list of all finds that can accept an increase
	int nTotal = 0;
	int nRemap[FIND_COUNT] = {};
	for (int i = 0; i < FIND_COUNT; i++)
	{
		if (!m_bHitMax[i])
			nRemap[nTotal++] = i;
	}
	if (!nTotal) // all finds are full
	{
		DevMsg("Find Counter %s cannot ADD because all finds are full!\n", GetDebugName());
		return false;
	}
	int nRandom = 0;
	if (--nTotal) //we have more than one find to pick from (and we need to shave that last increment off anyway)
		nRandom = RandomInt(0, nTotal);
	UpdateOutValue(pActivator, nRemap[nRandom], m_OutValue[nRemap[nRandom]].Get() + 1);
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Input handler for adding to the accumulator value.
// Input  : Integer value to add.
//-----------------------------------------------------------------------------
void CWeaponSuitcase::InputAdd(int find)
{
	if (find < 0) //-1 is our random find
	{
		InputAddRandom(GetOwner());
		return;
	}
	int nNewValue = m_OutValue[find].Get();
	nNewValue++;
	if (nNewValue > m_nMax[find])
		InputAddRandom(GetOwner());
	else
		UpdateOutValue(GetOwner(), find, nNewValue);
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for subtracting from the current value.
// Input  : Integer value to subtract.
//-----------------------------------------------------------------------------
void CWeaponSuitcase::InputSubtract(int find)
{
	if (find < 0) //-1 is our random find
	{
		DevMsg("Find Counter %s cannot subtract from a random find!\n", GetDebugName());
		return;
	}
	int nNewValue = m_OutValue[find].Get();
	nNewValue--;
	if (nNewValue >= m_nMin)
		UpdateOutValue(GetOwner(), find, nNewValue);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponSuitcase::InputGetValue(int find)
{
	int nOutValue = m_OutValue[find].Get();
	m_OnGetValue[find].Set(nOutValue, GetOwner(), GetOwner());
}


#endif // !CLIENT_DLL

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
//
// Find Item. Placed into briefcase if the player has it, otherwise gives the find carry weapon.
//
//-----------------------------------------------------------------------------

#define FIND_MODEL "models/weapons/w_find.mdl"
#define FIND_DROPTIME 1.0f

class CItemFind : public CItem
{
public:
	DECLARE_CLASS(CItemFind, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel(FIND_MODEL);
		BaseClass::Spawn();
	}
	void SetInitial(int);
	void Precache(void)
	{
		//UTIL_PrecacheOther("weapon_find");
		PrecacheModel(FIND_MODEL);
	}

	void SetOwnerTemporary(CBaseEntity* pOwner)
	{
		SetOwnerEntity(pOwner);
		m_flClearOwnerTime = gpGlobals->curtime + FIND_DROPTIME;
	}
	int GetType(void)
	{
		return m_nType;
	}
	bool IsOriginalType(void)
	{
		return m_nType == m_nTypeOriginal;
	}
	void Randomize(void)
	{
		m_nType = FIND_RANDOM;
		m_nSkin = 0;
	}
	void Derandomize(void)
	{
		m_nType = m_nTypeOriginal;
		m_nSkin = m_nType + 1;
	}
	void Derandomize(int type)
	{
		if (m_nType == type)
		{
			m_nType = m_nTypeOriginal;
			m_nSkin = m_nType + 1;
		}
	}
	bool MyTouch(CBasePlayer *pPlayer);
private:
	bool KeyValue(const char *szKeyName, const char *szValue);
	int m_nType = FIND_RANDOM;
	int m_nTypeOriginal = FIND_RANDOM; //In case this find is randomized from being maxed out, we can return it to its level-designer designated type if it drops back below max.
	//CHL2MP_Player* m_pOwner = NULL;
	float m_flClearOwnerTime = FLT_MAX; //when the player willingly drops us, we want to delay removing them as our owner so they don't pick us up again immediately.
	//void Think(void);
};

LINK_ENTITY_TO_CLASS(item_find, CItemFind);

#endif

//-----------------------------------------------------------------------------
//
// Find weapon. Allows a player without the briefcase to carry up to three finds and still use other weapons. Alerts guards if you're seen with it out! 
//
//-----------------------------------------------------------------------------

class CWeaponFind : public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS(CWeaponFind, CBaseHL2MPCombatWeapon);
public:

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponFind();

	void	Precache(void);
	void	Spawn(void);
	void	PrimaryAttack(void);
	void	SecondaryAttack(void) {}
	void	DecrementAmmo(CBaseCombatCharacter *pOwner);
	void	ItemPostFrame(void);

	bool	Deploy(void);
	bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);

	bool	Reload(void);

	void	UpdateBodygroups(void)
	{
		CBasePlayer* pOwner = ToBasePlayer(GetOwner());
		if (pOwner)
		{
			SetBodygroup(1, min(0, max(MAX_FINDS, pOwner->GetAmmoCount(GetPrimaryAmmoType())) - 1));
			CBaseViewModel *pViewModel = pOwner->GetViewModel();

			if (pViewModel)
				pViewModel->SetBodygroup(1, min(0, max(MAX_FINDS, pOwner->GetAmmoCount(GetPrimaryAmmoType())) - 1));
		}
	}
	void	UpdateSkin(int nSkin)
	{
		m_nSkin = nSkin;
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());

		if (pOwner == NULL)
			return;

		CBaseViewModel *pViewModel = pOwner->GetViewModel();

		if (pViewModel == NULL)
			return;

		pViewModel->m_nSkin = nSkin;
	}

#ifndef CLIENT_DLL
	void	UpdateOnRemove(void);
	
	void	AddFind(int);
	void	ReactivateFinds(bool lob = false);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
#endif

private:

	void	LobGrenade(CBasePlayer *pPlayer);
	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
	void	CheckThrowPosition(CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc);

	CNetworkVar(bool, m_bRedraw);	//Draw the weapon again after throwing a grenade

	CNetworkVar(int, m_AttackPaused);
	CNetworkVar(bool, m_fDrawbackFinished);

	CWeaponFind(const CWeaponFind &);

#ifndef CLIENT_DLL
	int	m_nFinds[2];
	DECLARE_ACTTABLE();
#endif
};

#ifndef CLIENT_DLL

//-----------------------------------------------------------------------------
//
// Find Item members
//
//-----------------------------------------------------------------------------

//Player has touched us. Put us in the briefcase, or pick us up as our own weapon if not.
bool CItemFind::MyTouch(CBasePlayer *pPlayer)
{
	if (m_flClearOwnerTime <= gpGlobals->curtime)
	{
		m_flClearOwnerTime = FLT_MAX;
		SetOwnerEntity(NULL);
	}
	if (pPlayer == GetOwnerEntity())
		return false;
	CWeaponSuitcase* pSuitcase = dynamic_cast<CWeaponSuitcase*>(pPlayer->Weapon_OwnsThisType("weapon_suitcase"));
	CWeaponFind* pFindWep = dynamic_cast<CWeaponFind*>(pPlayer->Weapon_OwnsThisType("weapon_find"));
	if (pSuitcase)
	{
		pSuitcase->InputAdd(m_nType);
		UTIL_Remove(this);
		return true;
	}
	else if (pFindWep)
	{
		if (pPlayer->GiveAmmo(1, pFindWep->GetPrimaryAmmoType(), true))
		{
			SetOwnerEntity(pPlayer);
			m_flClearOwnerTime = FLT_MAX;
			pFindWep->UpdateBodygroups();
			pFindWep->AddFind(m_nType);
			UTIL_Remove(this);
			return true;
		}
	}
	else
	{
		pPlayer->GiveNamedItem("weapon_find");
		SetOwnerEntity(pPlayer);
		m_flClearOwnerTime = FLT_MAX;
		//set the find's skin to reflect this first find
		pFindWep = dynamic_cast<CWeaponFind*>(pPlayer->Weapon_OwnsThisType("weapon_find"));
		if (pFindWep)
		{
			pFindWep->AddFind(m_nType);
			pFindWep->UpdateSkin(m_nSkin);
			UTIL_Remove(this);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Sets our find's initial state, either with keyvalues or when spawned by the find weapon
//-----------------------------------------------------------------------------

void CItemFind::SetInitial(int nType)
{
	
	if (nType >= FIND_COUNT || nType < FIND_RANDOM)
		nType = FIND_RANDOM;
	m_nType = nType;
	m_nTypeOriginal = m_nType;
	m_nSkin = m_nType + 1;
}

//-----------------------------------------------------------------------------
// Purpose: Handles key values from the BSP before spawn is called.
//-----------------------------------------------------------------------------

bool CItemFind::KeyValue(const char *szKeyName, const char *szValue)
{
	// Set our find type
	if (!stricmp(szKeyName, "type"))
	{
		SetInitial(atoi(szValue));
		return true;
	}
	return(BaseClass::KeyValue(szKeyName, szValue));
}
#endif // !CLIENT_DLL

//-----------------------------------------------------------------------------
//
// Find weapon members.
//
//-----------------------------------------------------------------------------

#ifndef CLIENT_DLL

acttable_t	CWeaponFind::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_PHYSGUN, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_PHYSGUN, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_PHYSGUN, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_PHYSGUN, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SLAM, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_PHYSGUN, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_PHYSGUN, false },
	//NPC
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },
};

IMPLEMENT_ACTTABLE(CWeaponFind);

#endif

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponFind, DT_WeaponFind)

BEGIN_NETWORK_TABLE(CWeaponFind, DT_WeaponFind)

#ifdef CLIENT_DLL
RecvPropBool(RECVINFO(m_bRedraw)),
RecvPropBool(RECVINFO(m_fDrawbackFinished)),
RecvPropInt(RECVINFO(m_AttackPaused)),
#else
SendPropBool(SENDINFO(m_bRedraw)),
SendPropBool(SENDINFO(m_fDrawbackFinished)),
SendPropInt(SENDINFO(m_AttackPaused)),
#endif

END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponFind)
DEFINE_PRED_FIELD(m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_fDrawbackFinished, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_AttackPaused, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_find, CWeaponFind);
PRECACHE_WEAPON_REGISTER(weapon_find);

CWeaponFind::CWeaponFind(void) :
CBaseHL2MPCombatWeapon()
{
	m_bRedraw = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFind::Precache(void)
{
	BaseClass::Precache();
	PrecacheScriptSound("WeaponFind.Throw");
}

//-----------------------------------------------------------------------------
// Purpose: Called before spawning
//-----------------------------------------------------------------------------
void CWeaponFind::Spawn(void)
{
#ifndef CLIENT_DLL
	// "null" out our finds
	for (int i = 0; i < MAX_FINDS; i++)
		m_nFinds[i] = FIND_COUNT;
#endif // !CLIENT_DLL
	BaseClass::Spawn();
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponFind::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	bool fThrewGrenade = false;

	switch (pEvent->event)
	{
	case EVENT_WEAPON_SEQUENCE_FINISHED:
		m_fDrawbackFinished = true;
		break;
	case EVENT_WEAPON_THROW3:
		LobGrenade(pOwner);
		DecrementAmmo(pOwner);
		fThrewGrenade = true;
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}

#define RETHROW_DELAY	0.5
	if (fThrewGrenade)
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + RETHROW_DELAY;
		m_flNextSecondaryAttack = FLT_MAX;
		m_flTimeWeaponIdle = FLT_MAX; //NOTE: This is set once the animation has finished up!
	}
}

//-----------------------------------------------------------------------------
// Purpose: Add a find to our list, so we know what we're dealing with
//-----------------------------------------------------------------------------
void CWeaponFind::AddFind(int nFind)
{
	for (int i = 0; i < MAX_FINDS; i++)
	{
		if (m_nFinds[i] == FIND_COUNT)
		{
			m_nFinds[i] = nFind;
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Jettison our carried finds, either by lobbing or dropping them.
//-----------------------------------------------------------------------------
void CWeaponFind::ReactivateFinds(bool lob)
{
	if (GetOwner())
	{
		for (int i = 0; i < MAX_FINDS; i++)
		{
			if (m_nFinds[i] != FIND_COUNT)
			{
				CBaseEntity* pEnt = CreateEntityByName("item_find");
				CItemFind* pFind = dynamic_cast<CItemFind*>(pEnt);
				if (pFind)
				{
					pFind->SetInitial(m_nFinds[i]);
					pFind->SetOwnerTemporary(GetOwner());
					pFind->Spawn();
					CBasePlayer* pOwner = ToBasePlayer(GetOwner());
					if (pOwner)
					{
						Vector vecZero = Vector(0.0f, 0.0f, 0.0f);
						QAngle qRot = pOwner->GetAbsAngles();

						if (!lob) //just drop them
						{
							Vector vecSpawn = pOwner->GetAbsOrigin();
							vecSpawn.z += 8 * i; //don't spawn them all inside one-another
							pFind->Teleport(&vecSpawn, &qRot, &vecZero);
						}
						else //give'em a toss!
						{
							Vector	vecEye = pOwner->EyePosition();
							Vector	vForward, vRight;

							pOwner->EyeVectors(&vForward, &vRight, NULL);
							Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f + Vector(0, 0, -8);
							CheckThrowPosition(pOwner, vecEye, vecSrc);
							vecSrc.z += 8 * i;

							Vector vecThrow;
							pOwner->GetVelocity(&vecThrow, NULL);
							vecThrow += vForward * 350 + Vector(0, 0, 50);
							if (i)
								vecThrow.y += RandomFloat(-15.0f, 15.0f); //stagger later papers some so they don't stack neatly
							pFind->Teleport(&vecSrc, &qRot, &vecThrow); //A bit of a delay, but...
						}
					}
				}
				m_nFinds[i] = FIND_COUNT;
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFind::UpdateOnRemove(void)
{
	ReactivateFinds();
	BaseClass::UpdateOnRemove();
}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponFind::Deploy(void)
{
	m_bRedraw = false;
	m_fDrawbackFinished = false;

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponFind::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	m_bRedraw = false;
	m_fDrawbackFinished = false;

	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponFind::Reload(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
		return false;
	CBasePlayer *pPlayer = ToBasePlayer(pOwner);

	if (!pPlayer)
		return false;

	if (!HasPrimaryAmmo())
	{
		pPlayer->SwitchToNextBestWeapon(this);
#ifndef CLIENT_DLL
		UTIL_Remove(this);
#endif // !CLIENT_DLL
		return false;
	}

	if ((m_bRedraw) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		//Redraw the weapon
		SendWeaponAnim(ACT_VM_DRAW);

		//Update our times
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		m_flNextSecondaryAttack = FLT_MAX;
		m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();

		//Mark this as done
		m_bRedraw = false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFind::PrimaryAttack(void)
{
	if (m_bRedraw)
		return;

	if (!HasPrimaryAmmo())
		return;

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());;

	if (!pPlayer)
		return;

	// Note that this is a primary attack and prepare the grenade attack to pause.
	m_AttackPaused = GRENADE_PAUSED_PRIMARY;
	//SendWeaponAnim( ACT_VM_PULLBACK_HIGH );
	SendWeaponAnim(ACT_VM_PULLBACK_LOW);

	// Put both of these off indefinitely. We do not know how long
	// the player will hold the grenade.
	m_flTimeWeaponIdle = FLT_MAX;
	m_flNextPrimaryAttack = FLT_MAX;

	// If I'm now out of ammo, switch away and remove
	if (!HasPrimaryAmmo())
	{
		pPlayer->SwitchToNextBestWeapon(this);
#ifndef CLIENT_DLL
		UTIL_Remove(this);
#endif // !CLIENT_DLL
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOwner - 
//-----------------------------------------------------------------------------
void CWeaponFind::DecrementAmmo(CBaseCombatCharacter *pOwner)
{
	pOwner->RemoveAmmo(pOwner->GetAmmoCount(m_iPrimaryAmmoType), m_iPrimaryAmmoType); //use it all at once
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFind::ItemPostFrame(void)
{
	if (m_fDrawbackFinished)
	{
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());

		if (pOwner)
		{
			switch (m_AttackPaused)
			{
			case GRENADE_PAUSED_PRIMARY:
				if (!(pOwner->m_nButtons & IN_ATTACK))
				{
					SendWeaponAnim(ACT_VM_HAULBACK); //replacing primary attack with "drop" functionality
					m_fDrawbackFinished = false;
				}
			default:
				break;
			}
		}
	}

	

	BaseClass::ItemPostFrame();

	if (m_bRedraw && IsViewModelSequenceFinished())
		Reload();
}

// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
void CWeaponFind::CheckThrowPosition(CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc)
{
	trace_t tr;

	UTIL_TraceHull(vecEye, vecSrc, -Vector(GRENADE_RADIUS + 2, GRENADE_RADIUS + 2, GRENADE_RADIUS + 2), Vector(GRENADE_RADIUS + 2, GRENADE_RADIUS + 2, GRENADE_RADIUS + 2),
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr);

	if (tr.DidHit())
		vecSrc = tr.endpos;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponFind::LobGrenade(CBasePlayer *pPlayer)
{
#ifndef CLIENT_DLL
	ReactivateFinds(true);
	/*Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors(&vForward, &vRight, NULL);
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f + Vector(0, 0, -8);
	CheckThrowPosition(pPlayer, vecEye, vecSrc);

	Vector vecThrow;
	pPlayer->GetVelocity(&vecThrow, NULL);
	vecThrow += vForward * 350 + Vector(0, 0, 50);
	
	CBaseGrenade *pGrenade = Fraggrenade_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(200,random->RandomInt(-600,600),0), pPlayer, GRENADE_TIMER, false );

	if ( pGrenade )
	{
	pGrenade->SetDamage( GetHL2MPWpnData().m_iPlayerDamage );
	pGrenade->SetDamageRadius( GRENADE_DAMAGE_RADIUS );
	}*/
#endif

	WeaponSound(WPN_DOUBLE);

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_bRedraw = true;
}