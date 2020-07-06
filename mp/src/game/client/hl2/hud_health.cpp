//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// Health.cpp
//
// implementation of CHudHealth class
//
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"

#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_HEALTH -1

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudHealth : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudHealth, CHudNumericDisplay );

public:
	CHudHealth( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();
			void MsgFunc_Damage( bf_read &msg );
	virtual void Paint(void);

private:
	// old variables
	int		m_iHealth;
	int		m_iMaxHealth;
	bool	m_bHealthHitMax = true;
	
	int		m_bitsDamage;
};	

DECLARE_HUDELEMENT( CHudHealth );
DECLARE_HUD_MESSAGE( CHudHealth, Damage );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHealth::CHudHealth( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudHealth")
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
	SetShouldDisplaySecondaryValue(true);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::Init()
{
	HOOK_HUD_MESSAGE( CHudHealth, Damage );
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::Reset()
{
	m_iHealth		= INIT_HEALTH;
	m_iMaxHealth = m_iHealth;
	m_bitsDamage = 0;

	wchar_t *tempString = g_pVGuiLocalize->Find("#Repose_Hud_HEALTH");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"HP");
	}
	SetShouldDisplaySecondaryValue(true);
	SetDisplayValue(m_iHealth);
	SetSecondaryValue(m_iMaxHealth);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::OnThink()
{
	int newHealth = 0;
	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();
	if ( local )
	{
		// Never below zero
		newHealth = MAX( local->GetHealth(), 0 );
		m_iMaxHealth = local->GetMaxHealth();
	}

	// Only update the fade if we've changed health
	if ( newHealth == m_iHealth )
	{
		return;
	}

	m_iHealth = newHealth;

	if ( m_iHealth >= 20 )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthIncreasedAbove20");
	}
	else if ( m_iHealth > 0 )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthIncreasedBelow20");
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthLow");
	}

	//uncomment to hide max health when at full health
	if (m_iHealth >= m_iMaxHealth)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthMaxedOut");
		m_bHealthHitMax = true;
	}
	else if (m_bHealthHitMax)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthNotMaxed");
		m_bHealthHitMax = false;
	}

	SetDisplayValue(m_iHealth);
	SetSecondaryValue(m_iMaxHealth);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::MsgFunc_Damage( bf_read &msg )
{

	int armor = msg.ReadByte();	// armor
	int damageTaken = msg.ReadByte();	// health
	long bitsDamage = msg.ReadLong(); // damage bits
	bitsDamage; // variable still sent but not used

	Vector vecFrom;

	vecFrom.x = msg.ReadBitCoord();
	vecFrom.y = msg.ReadBitCoord();
	vecFrom.z = msg.ReadBitCoord();

	// Actually took damage?
	if ( damageTaken > 0 || armor > 0 )
	{
		if ( damageTaken > 0 )
		{
			// start the animation
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthDamageTaken");
		}
	}
}

void CHudHealth::Paint(void)
{
	if (m_bDisplayValue)
	{
		// draw our numbers
		surface()->DrawSetTextColor(GetFgColor());
		m_bIndent = true;
		PaintNumbers(m_hNumberFont, digit_xpos, digit_ypos, m_iValue);

		// draw the overbright blur
		for (float fl = m_flBlur; fl > 0.0f; fl -= 1.0f)
		{
			if (fl >= 1.0f)
			{
				PaintNumbers(m_hNumberGlowFont, digit_xpos, digit_ypos, m_iValue);
			}
			else
			{
				// draw a percentage of the last one
				Color col = GetFgColor();
				col[3] *= fl;
				surface()->DrawSetTextColor(col);
				PaintNumbers(m_hNumberGlowFont, digit_xpos, digit_ypos, m_iValue);
			}
		}
	}

	// total ammo
	if (m_bDisplaySecondaryValue)
	{
		surface()->DrawSetTextColor(GetFgColor());
		m_bIndent = false;
		PaintSlashNumbers(m_hSmallNumberFont, digit2_xpos, digit2_ypos, m_iSecondaryValue);
	}

	PaintLabel();
}