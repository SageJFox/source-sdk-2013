//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// battery.cpp
//
// implementation of CHudStatSTR class
//
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"

#include "vgui_controls/AnimationController.h"
#include "vgui/ILocalize.h"
#include "c_hl2mp_player.h"
#include "repose_stats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_STAT	-1
//very annoying that I couldn't get these to just all inherit from a CHudStats base class, but it wouldn't stop throwing errors
//TODO: try again another day


//-----------------------------------------------------------------------------
// Purpose: Displays STR stat on hud
//-----------------------------------------------------------------------------
class CHudStatSTR : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudStatSTR, CHudNumericDisplay );

public:
	CHudStatSTR( const char *pElementName );
	void Init( void );
	void Reset( void );
	void VidInit( void );
	void OnThink( void );
	//bool ShouldDraw();
	//virtual void Paint(void);
	//void AdjustX();
	
private:
	int		m_iStatInQuestion = STR;
	int		m_iStat;	
	int		m_iNewStat;
	int		m_iMod;
	int		m_iNewMod;
};

DECLARE_HUDELEMENT( CHudStatSTR );
//DECLARE_HUD_MESSAGE( CHudStatSTR, Battery );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudStatSTR::CHudStatSTR( const char *pElementName ) : BaseClass(NULL, "HudStatSTR"), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_NEEDSUIT );
	m_bDisplaySecondaryValue = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudStatSTR::Init()
{
//	HOOK_HUD_MESSAGE( CHudStatSTR, Battery);
	Reset();
	//AdjustX();
	m_iStat		= INIT_STAT;
	m_iNewStat  = 0;
	m_iMod		= INIT_STAT;
	m_iNewMod   = 0;
	m_bDisplaySecondaryValue = true;
	m_bIsMod = true;
	m_bCentered = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudStatSTR::Reset()
{
	/*wchar_t* tempString = L"\0";
	wchar_t* def = L"\0";

	switch (m_iStatInQuestion)
	{
	case STR:
		tempString = g_pVGuiLocalize->Find("#Repose_Hud_STR");
		def = L"STR";
		break;
	case DEX:
		tempString = g_pVGuiLocalize->Find("#Repose_Hud_DEX");
		def = L"DEX";
		break;
	case INT:
		tempString = g_pVGuiLocalize->Find("#Repose_Hud_INT");
		def = L"INT";
		break;
	case CHA:
		tempString = g_pVGuiLocalize->Find("#Repose_Hud_CHA");
		def = L"CHA";
		break;
	default:
		tempString = g_pVGuiLocalize->Find("#Repose_Hud_NaN");
		def = L"???";
		break;
	}
	
	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(def);
	}*/
	wchar_t* tempString = g_pVGuiLocalize->Find("#Repose_Hud_STR");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"STR");
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudStatSTR::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
/*bool CHudStatSTR::ShouldDraw( void )
{
	bool bNeedsDraw = ( m_iStat != m_iNewStat ) || ( GetAlpha() > 0 ) || ( m_iMod != m_iNewMod );

	return ( bNeedsDraw && CHudElement::ShouldDraw() );
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudStatSTR::OnThink( void )
{
	C_BasePlayer* local = C_BasePlayer::GetLocalPlayer();
	if (local)
	{
		C_HL2MP_Player* pStats = dynamic_cast<C_HL2MP_Player*>(local);
		if (pStats)
			m_iNewStat = pStats->checkStat(m_iStatInQuestion);
	}

	if ( m_iStat == m_iNewStat /*&& m_iMod == m_iNewMod*/ )
		return;
	
	if ( m_iNewStat < m_iStat )
	{
		// battery power has decreased, so play the damaged animation
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("StatDecreasedSTR");

		// play an extra animation if we're super low
		if ( m_iNewStat < 3 )
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("StatLowSTR");
	}
	else if ( m_iNewStat > m_iStat )
	{
		// stat has increased (if we just loaded the game, don't use alert state)
		if (m_iStat != INIT_STAT)
		{
			if (m_iNewStat >= 3)
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("StatIncreasedAbove2STR");
			else
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("StatIncreasedBelow3STR");
		}
	}

	m_iStat = m_iNewStat;
	m_iNewMod = m_iStat - 10;
	m_iNewMod % 2 ? m_iNewMod-- : +m_iNewMod; //"round down" before division 
	m_iNewMod /= 2;
	/* --No need to check this at this point, we're never gonna have a mod change without a stat change (once we get some effects in, maybe)
	if ( m_iNewMod < m_iMod )
	{
		// battery power has decreased, so play the damaged animation
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ModDecreased");
	}
	else if ( m_iNewMod > m_iMod )
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ModIncreased");
	*/
	
	m_iMod = m_iNewMod;

	SetDisplayValue(m_iMod);
	SetSecondaryValue(m_iStat);
}
/*
void CHudStatSTR::Paint(void)
{
	if (m_bDisplayValue)
	{
		// draw our numbers
		g_pVGuiSurface->DrawSetTextColor(GetFgColor());
		//m_bIndent = true;
		int iCentered = UTIL_ComputeStringWidth(m_hNumberFont,"+0")/2;
		PaintModNumbers(m_hNumberFont, digit_xpos - iCentered, digit_ypos, m_iValue);

		// draw the overbright blur
		/*for (float fl = m_flBlur; fl > 0.0f; fl -= 1.0f)
		{
			if (fl >= 1.0f)
			{
				PaintModNumbers(m_hNumberGlowFont, digit_xpos - iCentered, digit_ypos, m_iValue);
			}
			else
			{
				// draw a percentage of the last one
				Color col = GetFgColor();
				col[3] *= fl;
				g_pVGuiSurface->DrawSetTextColor(col);
				PaintModNumbers(m_hNumberGlowFont, digit_xpos - iCentered, digit_ypos, m_iValue);
			}
			}*//*
	}

	// total ammo
	if (m_bDisplaySecondaryValue)
	{
		g_pVGuiSurface->DrawSetTextColor(GetFgColor());
		//m_bIndent = false;
		int iCentered = UTIL_ComputeStringWidth(m_hSmallNumberFont, (char*)m_iSecondaryValue) / 2;
		PaintNumbers(m_hSmallNumberFont, digit2_xpos - iCentered, digit2_ypos, m_iSecondaryValue);
		// draw the overbright blur - TODO: eventually!
		/*
		for (float fl = m_flBlur; fl > 0.0f; fl -= 1.0f)
		{
			if (fl >= 1.0f)
			{
				PaintNumbers(m_hSmallNumberGlowFont, digit2_xpos - iCentered, digit2_ypos, m_iSecondaryValue);
			}
			else
			{
				// draw a percentage of the last one
				Color col = GetFgColor();
				col[3] *= fl;
				g_pVGuiSurface->DrawSetTextColor(col);
				PaintModNumbers(m_hNumberGlowFont, digit2_xpos - iCentered, digit2_ypos, m_iSecondaryValue);
			}
		}
		*//*
	}

	PaintLabel(true);
}
/*
void CHudStatSTR::AdjustX()
{
	int iAdjust = (GetXPos() + GetWide()) * m_iStatInQuestion;
	return SetPos(GetXPos()+iAdjust,GetYPos());
}*/


//-----------------------------------------------------------------------------
// Purpose: Displays DEX stat on hud
//-----------------------------------------------------------------------------
class CHudStatDEX : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudStatDEX, CHudNumericDisplay);

public:
	CHudStatDEX(const char *pElementName);
	void Init(void);
	void Reset(void);
	void VidInit(void);
	void OnThink(void);

private:
	int		m_iStatInQuestion = DEX;
	int		m_iStat;
	int		m_iNewStat;
	int		m_iMod;
	int		m_iNewMod;
};

DECLARE_HUDELEMENT(CHudStatDEX);
//DECLARE_HUD_MESSAGE( CHudStatDEX, Battery );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudStatDEX::CHudStatDEX(const char *pElementName) : BaseClass(NULL, "HudStatDEX"), CHudElement(pElementName)
{
	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_NEEDSUIT);
	m_bDisplaySecondaryValue = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudStatDEX::Init()
{
	//	HOOK_HUD_MESSAGE( CHudStatDEX, Battery);
	Reset();
	m_iStat = INIT_STAT;
	m_iNewStat = 0;
	m_iMod = INIT_STAT;
	m_iNewMod = 0;
	m_bDisplaySecondaryValue = true;
	m_bIsMod = true;
	m_bCentered = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudStatDEX::Reset()
{
	wchar_t* tempString = g_pVGuiLocalize->Find("#Repose_Hud_DEX");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"DEX");
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudStatDEX::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudStatDEX::OnThink(void)
{
	C_BasePlayer* local = C_BasePlayer::GetLocalPlayer();
	if (local)
	{
		C_HL2MP_Player* pStats = dynamic_cast<C_HL2MP_Player*>(local);
		if (pStats)
			m_iNewStat = pStats->checkStat(m_iStatInQuestion);
	}

	if (m_iStat == m_iNewStat)
		return;

	if (m_iNewStat < m_iStat)
	{
		// battery power has decreased, so play the damaged animation
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("StatDecreasedDEX");

		// play an extra animation if we're super low
		if (m_iNewStat < 3)
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("StatLowDEX");
	}
	else if (m_iNewStat > m_iStat)
	{
		// stat has increased (if we just loaded the game, don't use alert state)
		if (m_iStat != INIT_STAT)
		{
			if (m_iNewStat >= 3)
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("StatIncreasedAbove2DEX");
			else
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("StatIncreasedBelow3DEX");
		}
	}

	m_iStat = m_iNewStat;
	m_iNewMod = m_iStat - 10;
	m_iNewMod % 2 ? m_iNewMod-- : +m_iNewMod; //"round down" before division 
	m_iNewMod /= 2;
	
	m_iMod = m_iNewMod;

	SetDisplayValue(m_iMod);
	SetSecondaryValue(m_iStat);
}

//-----------------------------------------------------------------------------
// Purpose: Displays INT stat on hud
//-----------------------------------------------------------------------------
class CHudStatINT : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudStatINT, CHudNumericDisplay);

public:
	CHudStatINT(const char *pElementName);
	void Init(void);
	void Reset(void);
	void VidInit(void);
	void OnThink(void);

private:
	int		m_iStatInQuestion = INT;
	int		m_iStat;
	int		m_iNewStat;
	int		m_iMod;
	int		m_iNewMod;
};

DECLARE_HUDELEMENT(CHudStatINT);
//DECLARE_HUD_MESSAGE( CHudStatINT, Battery );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudStatINT::CHudStatINT(const char *pElementName) : BaseClass(NULL, "HudStatINT"), CHudElement(pElementName)
{
	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_NEEDSUIT);
	m_bDisplaySecondaryValue = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudStatINT::Init()
{
	//	HOOK_HUD_MESSAGE( CHudStatINT, Battery);
	Reset();
	m_iStat = INIT_STAT;
	m_iNewStat = 0;
	m_iMod = INIT_STAT;
	m_iNewMod = 0;
	m_bDisplaySecondaryValue = true;
	m_bIsMod = true;
	m_bCentered = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudStatINT::Reset()
{
	wchar_t* tempString = g_pVGuiLocalize->Find("#Repose_Hud_INT");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"INT");
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudStatINT::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudStatINT::OnThink(void)
{
	C_BasePlayer* local = C_BasePlayer::GetLocalPlayer();
	if (local)
	{
		C_HL2MP_Player* pStats = dynamic_cast<C_HL2MP_Player*>(local);
		if (pStats)
			m_iNewStat = pStats->checkStat(m_iStatInQuestion);
	}

	if (m_iStat == m_iNewStat)
		return;
	
	if (m_iNewStat < m_iStat)
	{
		// battery power has decreased, so play the damaged animation
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("StatDecreasedINT");

		// play an extra animation if we're super low
		if (m_iNewStat < 3)
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("StatLowINT");
	}
	else if (m_iNewStat > m_iStat)
	{
		// stat has increased (if we just loaded the game, don't use alert state)
		if (m_iStat != INIT_STAT)
		{
			if (m_iNewStat >= 3)
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("StatIncreasedAbove2INT");
			else
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("StatIncreasedBelow3INT");
		}
	}

	m_iStat = m_iNewStat;
	m_iNewMod = m_iStat - 10;
	m_iNewMod % 2 ? m_iNewMod-- : +m_iNewMod; //"round down" before division 
	m_iNewMod /= 2;
	
	m_iMod = m_iNewMod;

	SetDisplayValue(m_iMod);
	SetSecondaryValue(m_iStat);
}

//-----------------------------------------------------------------------------
// Purpose: Displays CHA stat on hud
//-----------------------------------------------------------------------------
class CHudStatCHA : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudStatCHA, CHudNumericDisplay);

public:
	CHudStatCHA(const char *pElementName);
	void Init(void);
	void Reset(void);
	void VidInit(void);
	void OnThink(void);

private:
	int		m_iStatInQuestion = CHA;
	int		m_iStat;
	int		m_iNewStat;
	int		m_iMod;
	int		m_iNewMod;
};

DECLARE_HUDELEMENT(CHudStatCHA);
//DECLARE_HUD_MESSAGE( CHudStatCHA, Battery );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudStatCHA::CHudStatCHA(const char *pElementName) : BaseClass(NULL, "HudStatCHA"), CHudElement(pElementName)
{
	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_NEEDSUIT);
	m_bDisplaySecondaryValue = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudStatCHA::Init()
{
	//	HOOK_HUD_MESSAGE( CHudStatCHA, Battery);
	Reset();
	m_iStat = INIT_STAT;
	m_iNewStat = 0;
	m_iMod = INIT_STAT;
	m_iNewMod = 0;
	m_bDisplaySecondaryValue = true;
	m_bIsMod = true;
	m_bCentered = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudStatCHA::Reset()
{
	wchar_t* tempString =  g_pVGuiLocalize->Find("#Repose_Hud_CHA");
	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"CHA");
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudStatCHA::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudStatCHA::OnThink(void)
{
	C_BasePlayer* local = C_BasePlayer::GetLocalPlayer();
	if (local)
	{
		C_HL2MP_Player* pStats = dynamic_cast<C_HL2MP_Player*>(local);
		if (pStats)
			m_iNewStat = pStats->checkStat(m_iStatInQuestion);
	}

	if (m_iStat == m_iNewStat)
		return;
	
	if (m_iNewStat < m_iStat)
	{
		// battery power has decreased, so play the damaged animation
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("StatDecreasedCHA");

		// play an extra animation if we're super low
		if (m_iNewStat < 3)
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("StatLowCHA");
	}
	else if (m_iNewStat > m_iStat)
	{
		// stat has increased (if we just loaded the game, don't use alert state)
		if (m_iStat != INIT_STAT)
		{
			if (m_iNewStat >= 3)
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("StatIncreasedAbove2CHA");
			else
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("StatIncreasedBelow3CHA");
		}
	}

	m_iStat = m_iNewStat;
	m_iNewMod = m_iStat - 10;
	m_iNewMod % 2 ? m_iNewMod-- : +m_iNewMod; //"round down" before division 
	m_iNewMod /= 2;
	m_iMod = m_iNewMod;

	SetDisplayValue(m_iMod);
	SetSecondaryValue(m_iStat);
}