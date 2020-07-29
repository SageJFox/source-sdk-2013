//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_hl2mpbasebasebludgeon.h"
#ifndef CLIENT_DLL
#include "entityoutput.h"
#include "finds.h"
#endif // !CLIENT_DLL
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
		/*
		Q_snprintf(tempstr, sizeof(tempstr), "    min value: %n", m_nMin);
		EntityText(text_offset, tempstr, 0);
		text_offset++;

		Q_snprintf(tempstr, sizeof(tempstr), "    max value: %n", m_nMax);
		EntityText(text_offset, tempstr, 0);
		text_offset++;
		*/
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
		//
		// Fire an output any time we reach or exceed our maximum value.
		//
		if (nNewValue >= m_nMax[find])
		{
			if (!m_bHitMax[find])
			{
				m_bHitMax[find] = true;
				m_OnHitMax[find].FireOutput(pActivator, this);
			}
		}
		else
		{
			m_bHitMax[find] = false;
		}

		//
		// Fire an output any time we reach or go below our minimum value.
		//
		if (nNewValue <= m_nMin)
		{
			if (!m_bHitMin[find])
			{
				m_bHitMin[find] = true;
				m_OnHitMin[find].FireOutput(pActivator, this);
			}
		}
		else
		{
			m_bHitMin[find] = false;
		}

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
#endif // !CLIENT_DLL