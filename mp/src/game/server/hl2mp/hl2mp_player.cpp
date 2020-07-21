//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL2.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "hl2mp_player.h"
#include "globalstate.h"
#include "game.h"
#include "gamerules.h"
#include "hl2mp_player_shared.h"
#include "predicted_viewmodel.h"
#include "in_buttons.h"
#include "hl2mp_gamerules.h"
#include "KeyValues.h"
#include "team.h"
#include "weapon_hl2mpbase.h"
#include "grenade_satchel.h"
#include "eventqueue.h"
#include "gamestats.h"

#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

#include "ilagcompensationmanager.h"

int g_iLastCitizenModel = 0;
int g_iLastCombineModel = 0;

CBaseEntity	 *g_pLastCombineSpawn = NULL;
CBaseEntity	 *g_pLastRebelSpawn = NULL;
extern CBaseEntity				*g_pLastSpawn;

#define HL2MP_COMMAND_MAX_RATE 0.3

void DropPrimedFragGrenade( CHL2MP_Player *pPlayer, CBaseCombatWeapon *pGrenade );

extern ConVar sv_stickysprint;

LINK_ENTITY_TO_CLASS( player, CHL2MP_Player );

LINK_ENTITY_TO_CLASS( info_player_combine, CPointEntity );
LINK_ENTITY_TO_CLASS( info_player_rebel, CPointEntity );

IMPLEMENT_SERVERCLASS_ST(CHL2MP_Player, DT_HL2MP_Player)
	SendPropArray3(SENDINFO_ARRAY3(stats), SendPropInt(SENDINFO_ARRAY(stats), 10)),
	SendPropArray3(SENDINFO_ARRAY3(statsBase), SendPropInt(SENDINFO_ARRAY(statsBase), 10)),
	SendPropArray3(SENDINFO_ARRAY3(modifiers), SendPropInt(SENDINFO_ARRAY(modifiers), 10)),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 11, SPROP_CHANGES_OFTEN ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 11, SPROP_CHANGES_OFTEN ),
	SendPropEHandle( SENDINFO( m_hRagdoll ) ),
	SendPropInt( SENDINFO( m_iSpawnInterpCounter), 4 ),
	SendPropInt( SENDINFO( m_iPlayerSoundType), 3 ),
	
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseFlex", "m_viewtarget" ),

//	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
//	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),
	
END_SEND_TABLE()

BEGIN_DATADESC( CHL2MP_Player )
END_DATADESC()

const char *g_ppszRandomCitizenModels[] = 
{
	"models/players/male_01.mdl",
	"models/players/male_02.mdl",
	"models/players/female_01.mdl",
	"models/players/male_03.mdl",
	"models/players/female_02.mdl",
	"models/players/male_04.mdl",
	"models/players/female_03.mdl",
	"models/players/male_05.mdl",
	"models/players/female_04.mdl",
	"models/players/male_06.mdl",
	"models/players/female_06.mdl",
	"models/players/male_07.mdl",
	"models/players/female_07.mdl",
	"models/players/male_08.mdl",
	"models/players/male_09.mdl",
};

const char *g_ppszRandomCombineModels[] =
{
	"models/players/combine_soldier.mdl",
	"models/players/combine_soldier_prisonguard.mdl",
	"models/players/combine_super_soldier.mdl",
	"models/players/police.mdl",
};


#define MAX_COMBINE_MODELS 4
#define MODEL_CHANGE_INTERVAL 5.0f
#define TEAM_CHANGE_INTERVAL 5.0f

#define HL2MPPLAYER_PHYSDAMAGE_SCALE 4.0f

#pragma warning( disable : 4355 )

CHL2MP_Player::CHL2MP_Player() : m_PlayerAnimState( this )
{
	m_angEyeAngles.Init();

	m_iLastWeaponFireUsercmd = 0;

	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;

	m_iSpawnInterpCounter = 0;

    m_bEnterObserver = false;
	m_bReady = false;

	BaseClass::ChangeTeam( 0 );
	
//	UseClientSideAnimation();

	//Repose
	for (int i = 0; i < STAT_COUNT; i++)
	{
		stats.Set(i, 0);
		statsBase.Set(i, 0);
		modifiers.Set(i, 0);
	}
}

CHL2MP_Player::~CHL2MP_Player( void )
{

}

void CHL2MP_Player::UpdateOnRemove( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	BaseClass::UpdateOnRemove();
}

void CHL2MP_Player::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel ( "sprites/glow01.vmt" );

	//Precache Citizen models
	int nHeads = ARRAYSIZE( g_ppszRandomCitizenModels );
	int i;	

	for ( i = 0; i < nHeads; ++i )
	   	 PrecacheModel( g_ppszRandomCitizenModels[i] );

	//Precache Combine Models
	nHeads = ARRAYSIZE( g_ppszRandomCombineModels );

	for ( i = 0; i < nHeads; ++i )
	   	 PrecacheModel( g_ppszRandomCombineModels[i] );

	PrecacheFootStepSounds();

	PrecacheScriptSound( "NPC_MetroPolice.Die" );
	PrecacheScriptSound( "NPC_CombineS.Die" );
	PrecacheScriptSound( "NPC_Citizen.die" );
}

void CHL2MP_Player::GiveAllItems( void )
{
	EquipSuit();

	CBasePlayer::GiveAmmo( 255,	"Pistol");
	CBasePlayer::GiveAmmo( 255,	"AR2" );
	CBasePlayer::GiveAmmo( 5,	"AR2AltFire" );
	CBasePlayer::GiveAmmo( 255,	"SMG1");
	CBasePlayer::GiveAmmo( 3,	"smg1_grenade");
	CBasePlayer::GiveAmmo( 255,	"Buckshot");
	CBasePlayer::GiveAmmo( 32,	"357" );
	CBasePlayer::GiveAmmo( 3,	"rpg_round");
	CBasePlayer::GiveAmmo( 30,	"xbowbolt");
	CBasePlayer::GiveAmmo( 5,	"grenade" );
	CBasePlayer::GiveAmmo( 5,	"slam" );
	CBasePlayer::GiveAmmo(30,	"healthkit" );

	GiveNamedItem( "weapon_crowbar" );
	GiveNamedItem( "weapon_stunstick" );
	GiveNamedItem( "weapon_pistol" );
	GiveNamedItem( "weapon_357" );

	GiveNamedItem( "weapon_smg1" );
	GiveNamedItem( "weapon_ar2" );
	
	GiveNamedItem( "weapon_shotgun" );
	GiveNamedItem( "weapon_frag" );
	
	GiveNamedItem( "weapon_crossbow" );
	
	GiveNamedItem( "weapon_rpg" );

	GiveNamedItem( "weapon_slam" );

	GiveNamedItem( "weapon_physcannon" );

	GiveNamedItem( "weapon_healthkit" );
	GiveNamedItem( "weapon_bugbait" );
}

void CHL2MP_Player::GiveDefaultItems( void )
{
	EquipSuit();

	CBasePlayer::GiveAmmo( 255,	"Pistol");
	CBasePlayer::GiveAmmo( 45,	"SMG1");
	CBasePlayer::GiveAmmo( 1,	"grenade" );
	CBasePlayer::GiveAmmo( 6,	"Buckshot");
	CBasePlayer::GiveAmmo( 6,	"357" );

	if ( GetPlayerModelType() == PLAYER_SOUNDS_METROPOLICE || GetPlayerModelType() == PLAYER_SOUNDS_COMBINESOLDIER )
	{
		GiveNamedItem( "weapon_stunstick" );
	}
	else if ( GetPlayerModelType() == PLAYER_SOUNDS_CITIZEN )
	{
		GiveNamedItem( "weapon_crowbar" );
	}
	
	GiveNamedItem( "weapon_pistol" );
	GiveNamedItem( "weapon_smg1" );
	GiveNamedItem( "weapon_frag" );
	GiveNamedItem( "weapon_physcannon" );

	const char *szDefaultWeaponName = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_defaultweapon" );

	CBaseCombatWeapon *pDefaultWeapon = Weapon_OwnsThisType( szDefaultWeaponName );

	if ( pDefaultWeapon )
	{
		Weapon_Switch( pDefaultWeapon );
	}
	else
	{
		Weapon_Switch( Weapon_OwnsThisType( "weapon_physcannon" ) );
	}
}

void CHL2MP_Player::PickDefaultSpawnTeam( void )
{
	if ( GetTeamNumber() == 0 )
	{
		if ( HL2MPRules()->IsTeamplay() == false )
		{
			if ( GetModelPtr() == NULL )
			{
				const char *szModelName = NULL;
				szModelName = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_playermodel" );

				if ( ValidatePlayerModel( szModelName ) == false )
				{
					char szReturnString[512];

					Q_snprintf( szReturnString, sizeof (szReturnString ), "cl_playermodel models/players/combine_soldier.mdl\n" );
					engine->ClientCommand ( edict(), szReturnString );
				}

				ChangeTeam( TEAM_UNASSIGNED );
			}
		}
		else
		{
			CTeam *pCombine = g_Teams[TEAM_COMBINE];
			CTeam *pRebels = g_Teams[TEAM_REBELS];

			if ( pCombine == NULL || pRebels == NULL )
			{
				ChangeTeam( random->RandomInt( TEAM_COMBINE, TEAM_REBELS ) );
			}
			else
			{
				if ( pCombine->GetNumPlayers() > pRebels->GetNumPlayers() )
				{
					ChangeTeam( TEAM_REBELS );
				}
				else if ( pCombine->GetNumPlayers() < pRebels->GetNumPlayers() )
				{
					ChangeTeam( TEAM_COMBINE );
				}
				else
				{
					ChangeTeam( random->RandomInt( TEAM_COMBINE, TEAM_REBELS ) );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets HL2 specific defaults.
//-----------------------------------------------------------------------------
void CHL2MP_Player::Spawn(void)
{
	//SetPreventWeaponPickup(false);
	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;

	PickDefaultSpawnTeam();

	BaseClass::Spawn();
	
	if ( !IsObserver() )
	{
		pl.deadflag = false;
		RemoveSolidFlags( FSOLID_NOT_SOLID );

		RemoveEffects( EF_NODRAW );
		
		GiveDefaultItems();
		statBaseInit();
		//set max health by CON
		SetHealthMax(100 + (checkMod(STR/*CON*/) * 10));
		m_iHealth = m_iMaxHealth = m_iHealthMax;
	}

	SetNumAnimOverlays( 3 );
	ResetAnimation();

	m_nRenderFX = kRenderNormal;

	m_Local.m_iHideHUD = 0;
	
	AddFlag(FL_ONGROUND); // set the player on the ground at the start of the round.

	m_impactEnergyScale = HL2MPPLAYER_PHYSDAMAGE_SCALE;

	if ( HL2MPRules()->IsIntermission() )
	{
		AddFlag( FL_FROZEN );
	}
	else
	{
		RemoveFlag( FL_FROZEN );
	}

	m_iSpawnInterpCounter = (m_iSpawnInterpCounter + 1) % 8;

	m_Local.m_bDucked = false;

	SetPlayerUnderwater(false);

	m_bReady = false;

	for (int i = 0; i < STAT_COUNT; i++) //flush stats to reset effect
	{
		addToStatBase(i, 1);
		addToStatBase(i, -1);
	}
}

void CHL2MP_Player::PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize )
{
	/*
	CBaseCombatWeapon* pPhyscannon = Weapon_OwnsThisType("weapon_physcannon");

	if (Weapon_CanSwitchTo(pPhyscannon) || GetActiveWeapon() == pPhyscannon)
	{
		if (GetActiveWeapon() != pPhyscannon) Weapon_Switch(pPhyscannon);
		pPhyscannon->SecondaryAttack();
	}
	return;*/
}
extern ConVar physcannon_hl2mode;
extern ConVar hands_tracelength;
void CHL2MP_Player::PlayerUse(void)
{
	if (!physcannon_hl2mode.GetBool())
	{
		// Was use pressed or released?
		if (!((m_nButtons | m_afButtonPressed | m_afButtonReleased) & IN_USE))
			return;

		if ((m_afButtonPressed & IN_USE))
		{
			// this should drop our held object if we have one (as the following checks don't pick it up)
			CBaseCombatWeapon* pPhyscannon = Weapon_OwnsThisType("weapon_physcannon");
			if (pPhyscannon)
			{
				if (GetActiveWeapon() == pPhyscannon)
					pPhyscannon->SecondaryAttack();

				Vector forward, up;
				EyeVectors(&forward, NULL, &up);
				trace_t tr;
				Vector searchCenter = EyePosition();
				int useableContents = MASK_SOLID | CONTENTS_DEBRIS | CONTENTS_PLAYERCLIP;
				UTIL_TraceLine(searchCenter, searchCenter + forward * hands_tracelength.GetFloat(), useableContents, this, COLLISION_GROUP_NONE, &tr);
				CBaseEntity* pEntity = tr.m_pEnt;
				if (pEntity)
				{
					// These don't seem to want to work, so we'll just not bother looking at them for now.
					if (pEntity->MyNPCPointer() /*&&						//we're not an NPC, unless we're a physics based NPC (that can be safely picked up by hand)
					!(pEntity->ClassMatches("npc_grenade_frag")
					|| pEntity->ClassMatches("npc_turret_floor")
					|| pEntity->ClassMatches("npc_cscanner")
					|| pEntity->ClassMatches("npc_clawscanner")
					)*/)
					{
						pEntity = NULL;
					}
					else if (pEntity->GetMoveType() == MOVETYPE_NONE ||
						//our "exception" list, these are cases we want to pull the hands out for
						!(
						//Physics props not flagged "Prevent Pickup", or "Motion Disabled" *UNLESS* flagged "Enable Motion on Physcannon Grab" or "Generate output on +USE" (which seems to work for enabling motion too, per dm_lockdown?)
						(pEntity->ClassMatches("prop_physics*") && (!pEntity->HasSpawnFlags(1 << 7) && (!pEntity->HasSpawnFlags(1 << 3) || pEntity->HasSpawnFlags(1 << 4) || pEntity->HasSpawnFlags(1 << 6))))
						//weapons not flagged "Start Constrained"
						|| (pEntity->ClassMatches("weapon_*") && !pEntity->HasSpawnFlags(1 << 0))
						//items that are not HEV suit, not wall mounted chargers, and not flagged "Start Constrained"
						|| (pEntity->ClassMatches("item_*") && !pEntity->ClassMatches("item_suit*") && !pEntity->ClassMatches("item_healthcharger") && !pEntity->HasSpawnFlags(1 << 0))
						//TODO: Enable ragdoll pickup (not a rabbit hole I'm willing to go down at the moment)
						//ragdolls not flagged "Motion Disabled"
						//|| (pEntity->ClassMatches("prop_ragdoll") && !(pEntity->HasSpawnFlags(1 << 2))
						//func_physbox not flagged "Ignore +USE for Pickup", "Motion Disabled", or "Physgun is NOT allowed to pick this up.", *UNLESS* flagged "Enable Motion on Physcannon Grab"
						|| (pEntity->ClassMatches("func_physbox") && (!(pEntity->HasSpawnFlags(1 << 1) || pEntity->HasSpawnFlags(1 << 3) || pEntity->HasSpawnFlags(1 << 9)) || pEntity->HasSpawnFlags(1 << 5)))
						//|| pEntity->ClassMatches("combine_mine")
						|| pEntity->ClassMatches("physics_cannister")
						))
					{
						pEntity = NULL;
					}
				}
				if (!pEntity)
				{
					BaseClass::PlayerUse();
					if (GetActiveWeapon() == pPhyscannon)
						pPhyscannon->SecondaryAttack(); // this should drop our held object if we have one (as the previous checks don't pick it up), and shouldn't cause harm if we're not
				}
				else if (Weapon_CanSwitchTo(pPhyscannon) || GetActiveWeapon() == pPhyscannon)
				{
					if (GetActiveWeapon() != pPhyscannon)
						Weapon_Switch(pPhyscannon);
					pPhyscannon->SecondaryAttack();
				}
			}
		}
		else
			BaseClass::PlayerUse();
	}
	else
		BaseClass::PlayerUse();
}

bool CHL2MP_Player::ValidatePlayerModel( const char *pModel )
{
	int iModels = ARRAYSIZE( g_ppszRandomCitizenModels );
	int i;	

	for ( i = 0; i < iModels; ++i )
	{
		if ( !Q_stricmp( g_ppszRandomCitizenModels[i], pModel ) )
		{
			return true;
		}
	}

	iModels = ARRAYSIZE( g_ppszRandomCombineModels );

	for ( i = 0; i < iModels; ++i )
	{
	   	if ( !Q_stricmp( g_ppszRandomCombineModels[i], pModel ) )
		{
			return true;
		}
	}

	return false;
}

void CHL2MP_Player::SetPlayerTeamModel( void )
{
	const char *szModelName = NULL;
	szModelName = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_playermodel" );

	int modelIndex = modelinfo->GetModelIndex( szModelName );

	if ( modelIndex == -1 || ValidatePlayerModel( szModelName ) == false )
	{
		szModelName = "models/players/Combine_Soldier.mdl";
		m_iModelType = TEAM_COMBINE;

		char szReturnString[512];

		Q_snprintf( szReturnString, sizeof (szReturnString ), "cl_playermodel %s\n", szModelName );
		engine->ClientCommand ( edict(), szReturnString );
	}

	if ( GetTeamNumber() == TEAM_COMBINE )
	{
		if (Q_stristr(szModelName, "male"))
		{
			int nHeads = ARRAYSIZE( g_ppszRandomCombineModels );
		
			g_iLastCombineModel = ( g_iLastCombineModel + 1 ) % nHeads;
			szModelName = g_ppszRandomCombineModels[g_iLastCombineModel];
		}

		m_iModelType = TEAM_COMBINE;
	}
	else if ( GetTeamNumber() == TEAM_REBELS )
	{
		if (!Q_stristr(szModelName, "male"))
		{
			int nHeads = ARRAYSIZE( g_ppszRandomCitizenModels );

			g_iLastCitizenModel = ( g_iLastCitizenModel + 1 ) % nHeads;
			szModelName = g_ppszRandomCitizenModels[g_iLastCitizenModel];
		}

		m_iModelType = TEAM_REBELS;
	}
	
	SetModel( szModelName );
	SetupPlayerSoundsByModel( szModelName );

	m_flNextModelChangeTime = gpGlobals->curtime + MODEL_CHANGE_INTERVAL;
}

void CHL2MP_Player::SetPlayerModel( void )
{
	const char *szModelName = NULL;
	const char *pszCurrentModelName = modelinfo->GetModelName( GetModel());

	szModelName = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_playermodel" );

	if ( ValidatePlayerModel( szModelName ) == false )
	{
		char szReturnString[512];

		if ( ValidatePlayerModel( pszCurrentModelName ) == false )
		{
			pszCurrentModelName = "models/players/Combine_Soldier.mdl";
		}

		Q_snprintf( szReturnString, sizeof (szReturnString ), "cl_playermodel %s\n", pszCurrentModelName );
		engine->ClientCommand ( edict(), szReturnString );

		szModelName = pszCurrentModelName;
	}

	if ( GetTeamNumber() == TEAM_COMBINE )
	{
		int nHeads = ARRAYSIZE( g_ppszRandomCombineModels );
		
		g_iLastCombineModel = ( g_iLastCombineModel + 1 ) % nHeads;
		szModelName = g_ppszRandomCombineModels[g_iLastCombineModel];

		m_iModelType = TEAM_COMBINE;
	}
	else if ( GetTeamNumber() == TEAM_REBELS )
	{
		int nHeads = ARRAYSIZE( g_ppszRandomCitizenModels );

		g_iLastCitizenModel = ( g_iLastCitizenModel + 1 ) % nHeads;
		szModelName = g_ppszRandomCitizenModels[g_iLastCitizenModel];

		m_iModelType = TEAM_REBELS;
	}
	else
	{
		if ( Q_strlen( szModelName ) == 0 ) 
		{
			szModelName = g_ppszRandomCitizenModels[0];
		}

		if (Q_stristr(szModelName, "male"))
		{
			m_iModelType = TEAM_REBELS;
		}
		else
		{
			m_iModelType = TEAM_COMBINE;
		}
	}

	int modelIndex = modelinfo->GetModelIndex( szModelName );

	if ( modelIndex == -1 )
	{
		szModelName = "models/players/Combine_Soldier.mdl";
		m_iModelType = TEAM_COMBINE;

		char szReturnString[512];

		Q_snprintf( szReturnString, sizeof (szReturnString ), "cl_playermodel %s\n", szModelName );
		engine->ClientCommand ( edict(), szReturnString );
	}

	SetModel( szModelName );
	SetupPlayerSoundsByModel( szModelName );

	m_flNextModelChangeTime = gpGlobals->curtime + MODEL_CHANGE_INTERVAL;
}

void CHL2MP_Player::SetupPlayerSoundsByModel( const char *pModelName )
{
	if (Q_stristr(pModelName, "male"))
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_CITIZEN;
	}
	else if ( Q_stristr(pModelName, "police" ) )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_METROPOLICE;
	}
	else if ( Q_stristr(pModelName, "combine" ) )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_COMBINESOLDIER;
	}
}

void CHL2MP_Player::ResetAnimation( void )
{
	if ( IsAlive() )
	{
		SetSequence ( -1 );
		SetActivity( ACT_INVALID );

		if (!GetAbsVelocity().x && !GetAbsVelocity().y)
			SetAnimation( PLAYER_IDLE );
		else if ((GetAbsVelocity().x || GetAbsVelocity().y) && ( GetFlags() & FL_ONGROUND ))
			SetAnimation( PLAYER_WALK );
		else if (GetWaterLevel() > 1)
			SetAnimation( PLAYER_WALK );
	}
}


bool CHL2MP_Player::Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex )
{
	bool bRet = BaseClass::Weapon_Switch( pWeapon, viewmodelindex );

	if ( bRet == true )
	{
		ResetAnimation();
	}

	return bRet;
}

void CHL2MP_Player::PreThink( void )
{
	QAngle vOldAngles = GetLocalAngles();
	QAngle vTempAngles = GetLocalAngles();

	vTempAngles = EyeAngles();

	if ( vTempAngles[PITCH] > 180.0f )
	{
		vTempAngles[PITCH] -= 360.0f;
	}

	SetLocalAngles( vTempAngles );

	BaseClass::PreThink();
	State_PreThink();

	//Reset bullet force accumulator, only lasts one frame
	m_vecTotalBulletForce = vec3_origin;
	SetLocalAngles( vOldAngles );
}

void CHL2MP_Player::PostThink( void )
{
	BaseClass::PostThink();
	
	if ( GetFlags() & FL_DUCKING )
	{
		SetCollisionBounds( VEC_CROUCH_TRACE_MIN, VEC_CROUCH_TRACE_MAX );
	}

	m_PlayerAnimState.Update();

	// Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();

	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles( angles );
}

void CHL2MP_Player::PlayerDeathThink()
{
	if( !IsObserver() )
	{
		BaseClass::PlayerDeathThink();
	}
}

void CHL2MP_Player::FireBullets ( const FireBulletsInfo_t &info )
{
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( this, this->GetCurrentCommand() );

	FireBulletsInfo_t modinfo = info;

	CWeaponHL2MPBase *pWeapon = dynamic_cast<CWeaponHL2MPBase *>( GetActiveWeapon() );

	if ( pWeapon )
	{
		modinfo.m_iPlayerDamage = modinfo.m_flDamage = pWeapon->GetHL2MPWpnData().m_iPlayerDamage;
	}

	NoteWeaponFired();

	BaseClass::FireBullets( modinfo );

	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( this );
}

void CHL2MP_Player::NoteWeaponFired( void )
{
	Assert( m_pCurrentCommand );
	if( m_pCurrentCommand )
	{
		m_iLastWeaponFireUsercmd = m_pCurrentCommand->command_number;
	}
}

extern ConVar sv_maxunlag;

bool CHL2MP_Player::WantsLagCompensationOnEntity( const CBaseEntity *pEntity, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const
{
	// No need to lag compensate at all if we're not attacking in this command and
	// we haven't attacked recently.
	if ( !( pCmd->buttons & IN_ATTACK ) && (pCmd->command_number - m_iLastWeaponFireUsercmd > 5) )
		return false;

	return BaseClass::WantsLagCompensationOnEntity(pEntity, pCmd, pEntityTransmitBits);
}

Activity CHL2MP_Player::TranslateTeamActivity( Activity ActToTranslate )
{
	if ( m_iModelType == TEAM_COMBINE )
		 return ActToTranslate;
	
	if ( ActToTranslate == ACT_RUN )
		 return ACT_RUN_AIM_AGITATED;

	if ( ActToTranslate == ACT_IDLE )
		 return ACT_IDLE_AIM_AGITATED;

	if ( ActToTranslate == ACT_WALK )
		 return ACT_WALK_AIM_AGITATED;

	return ActToTranslate;
}

extern ConVar hl2_normspeed;

// Set the activity based on an event or current state
void CHL2MP_Player::SetAnimation( PLAYER_ANIM playerAnim )
{
	int animDesired;

	float speed;

	speed = GetAbsVelocity().Length2D();

	
	// bool bRunning = true;

	//Revisit!
/*	if ( ( m_nButtons & ( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT ) ) )
	{
		if ( speed > 1.0f && speed < hl2_normspeed.GetFloat() - 20.0f )
		{
			bRunning = false;
		}
	}*/

	if ( GetFlags() & ( FL_FROZEN | FL_ATCONTROLS ) )
	{
		speed = 0;
		playerAnim = PLAYER_IDLE;
	}

	Activity idealActivity = ACT_HL2MP_RUN;

	// This could stand to be redone. Why is playerAnim abstracted from activity? (sjb)
	if ( playerAnim == PLAYER_JUMP )
	{
		idealActivity = ACT_HL2MP_JUMP;
	}
	else if ( playerAnim == PLAYER_DIE )
	{
		if ( m_lifeState == LIFE_ALIVE )
		{
			return;
		}
	}
	else if ( playerAnim == PLAYER_ATTACK1 )
	{
		if ( GetActivity( ) == ACT_HOVER	|| 
			 GetActivity( ) == ACT_SWIM		||
			 GetActivity( ) == ACT_HOP		||
			 GetActivity( ) == ACT_LEAP		||
			 GetActivity( ) == ACT_DIESIMPLE )
		{
			idealActivity = GetActivity( );
		}
		else
		{
			idealActivity = ACT_HL2MP_GESTURE_RANGE_ATTACK;
		}
	}
	else if ( playerAnim == PLAYER_RELOAD )
	{
		idealActivity = ACT_HL2MP_GESTURE_RELOAD;
	}
	else if ( playerAnim == PLAYER_IDLE || playerAnim == PLAYER_WALK )
	{
		if ( !( GetFlags() & FL_ONGROUND ) && GetActivity( ) == ACT_HL2MP_JUMP )	// Still jumping
		{
			idealActivity = GetActivity( );
		}
		/*
		else if ( GetWaterLevel() > 1 )
		{
			if ( speed == 0 )
				idealActivity = ACT_HOVER;
			else
				idealActivity = ACT_SWIM;
		}
		*/
		else
		{
			if ( GetFlags() & FL_DUCKING )
			{
				if ( speed > 0 )
				{
					idealActivity = ACT_HL2MP_WALK_CROUCH;
				}
				else
				{
					idealActivity = ACT_HL2MP_IDLE_CROUCH;
				}
			}
			else
			{
				if ( speed > 0 )
				{
					/*
					if ( bRunning == false )
					{
						idealActivity = ACT_WALK;
					}
					else
					*/
					{
						idealActivity = ACT_HL2MP_RUN;
					}
				}
				else
				{
					idealActivity = ACT_HL2MP_IDLE;
				}
			}
		}

		idealActivity = TranslateTeamActivity( idealActivity );
	}
	
	if ( idealActivity == ACT_HL2MP_GESTURE_RANGE_ATTACK )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );

		// FIXME: this seems a bit wacked
		Weapon_SetActivity( Weapon_TranslateActivity( ACT_RANGE_ATTACK1 ), 0 );

		return;
	}
	else if ( idealActivity == ACT_HL2MP_GESTURE_RELOAD )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );
		return;
	}
	else
	{
		SetActivity( idealActivity );

		animDesired = SelectWeightedSequence( Weapon_TranslateActivity ( idealActivity ) );

		if (animDesired == -1)
		{
			animDesired = SelectWeightedSequence( idealActivity );

			if ( animDesired == -1 )
			{
				animDesired = 0;
			}
		}
	
		// Already using the desired animation?
		if ( GetSequence() == animDesired )
			return;

		m_flPlaybackRate = 1.0;
		ResetSequence( animDesired );
		SetCycle( 0 );
		return;
	}

	// Already using the desired animation?
	if ( GetSequence() == animDesired )
		return;

	//Msg( "Set animation to %d\n", animDesired );
	// Reset to first frame of desired animation
	ResetSequence( animDesired );
	SetCycle( 0 );
}


extern int	gEvilImpulse101;
//-----------------------------------------------------------------------------
// Purpose: Player reacts to bumping a weapon. 
// Input  : pWeapon - the weapon that the player bumped into.
// Output : Returns true if player picked up the weapon
//-----------------------------------------------------------------------------
bool CHL2MP_Player::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	CBaseCombatCharacter *pOwner = pWeapon->GetOwner();

	// Can I have this weapon type?
	if ( !IsAllowedToPickupWeapons() )
		return false;

	if ( pOwner || !Weapon_CanUse( pWeapon ) || !g_pGameRules->CanHavePlayerItem( this, pWeapon ) )
	{
		if ( gEvilImpulse101 )
		{
			UTIL_Remove( pWeapon );
		}
		return false;
	}

	// Don't let the player fetch weapons through walls (use MASK_SOLID so that you can't pickup through windows)
	if( !pWeapon->FVisible( this, MASK_SOLID ) && !(GetFlags() & FL_NOTARGET) )
	{
		return false;
	}

	bool bOwnsWeaponAlready = !!Weapon_OwnsThisType( pWeapon->GetClassname(), pWeapon->GetSubType());

	if ( bOwnsWeaponAlready == true ) 
	{
		//If we have room for the ammo, then "take" the weapon too.
		 if ( Weapon_EquipAmmoOnly( pWeapon ) )
		 {
			 pWeapon->CheckRespawn();

			 UTIL_Remove( pWeapon );
			 return true;
		 }
		 else
		 {
			 return false;
		 }
	}

	pWeapon->CheckRespawn();
	Weapon_Equip( pWeapon );

	return true;
}

void CHL2MP_Player::ChangeTeam( int iTeam )
{
/*	if ( GetNextTeamChangeTime() >= gpGlobals->curtime )
	{
		char szReturnString[128];
		Q_snprintf( szReturnString, sizeof( szReturnString ), "Please wait %d more seconds before trying to switch teams again.\n", (int)(GetNextTeamChangeTime() - gpGlobals->curtime) );

		ClientPrint( this, HUD_PRINTTALK, szReturnString );
		return;
	}*/

	bool bKill = false;

	if ( HL2MPRules()->IsTeamplay() != true && iTeam != TEAM_SPECTATOR )
	{
		//don't let them try to join combine or rebels during deathmatch.
		iTeam = TEAM_UNASSIGNED;
	}

	if ( HL2MPRules()->IsTeamplay() == true )
	{
		if ( iTeam != GetTeamNumber() && GetTeamNumber() != TEAM_UNASSIGNED )
		{
			bKill = true;
		}
	}

	BaseClass::ChangeTeam( iTeam );

	m_flNextTeamChangeTime = gpGlobals->curtime + TEAM_CHANGE_INTERVAL;

	if ( HL2MPRules()->IsTeamplay() == true )
	{
		SetPlayerTeamModel();
	}
	else
	{
		SetPlayerModel();
	}

	if ( iTeam == TEAM_SPECTATOR )
	{
		RemoveAllItems( true );

		State_Transition( STATE_OBSERVER_MODE );
	}

	if ( bKill == true )
	{
		CommitSuicide();
	}
}

bool CHL2MP_Player::HandleCommand_JoinTeam( int team )
{
	if ( !GetGlobalTeam( team ) || team == 0 )
	{
		Warning( "HandleCommand_JoinTeam( %d ) - invalid team index.\n", team );
		return false;
	}

	if ( team == TEAM_SPECTATOR )
	{
		// Prevent this is the cvar is set
		if ( !mp_allowspectators.GetInt() )
		{
			ClientPrint( this, HUD_PRINTCENTER, "#Cannot_Be_Spectator" );
			return false;
		}

		if ( GetTeamNumber() != TEAM_UNASSIGNED && !IsDead() )
		{
			m_fNextSuicideTime = gpGlobals->curtime;	// allow the suicide to work

			CommitSuicide();

			// add 1 to frags to balance out the 1 subtracted for killing yourself
			IncrementFragCount( 1 );
		}

		ChangeTeam( TEAM_SPECTATOR );

		return true;
	}
	else
	{
		StopObserverMode();
		State_Transition(STATE_ACTIVE);
	}

	// Switch their actual team...
	ChangeTeam( team );

	return true;
}

bool CHL2MP_Player::ClientCommand( const CCommand &args )
{
	if ( FStrEq( args[0], "spectate" ) )
	{
		if ( ShouldRunRateLimitedCommand( args ) )
		{
			// instantly join spectators
			HandleCommand_JoinTeam( TEAM_SPECTATOR );	
		}
		return true;
	}
	else if ( FStrEq( args[0], "jointeam" ) ) 
	{
		if ( args.ArgC() < 2 )
		{
			Warning( "Player sent bad jointeam syntax\n" );
		}

		if ( ShouldRunRateLimitedCommand( args ) )
		{
			int iTeam = atoi( args[1] );
			HandleCommand_JoinTeam( iTeam );
		}
		return true;
	}
	else if ( FStrEq( args[0], "joingame" ) )
	{
		return true;
	}

	return BaseClass::ClientCommand( args );
}

void CHL2MP_Player::CheatImpulseCommands( int iImpulse )
{
	switch ( iImpulse )
	{
		case 101:
			{
				if( sv_cheats->GetBool() )
				{
					GiveAllItems();
				}
			}
			break;

		default:
			BaseClass::CheatImpulseCommands( iImpulse );
	}
}

bool CHL2MP_Player::ShouldRunRateLimitedCommand( const CCommand &args )
{
	int i = m_RateLimitLastCommandTimes.Find( args[0] );
	if ( i == m_RateLimitLastCommandTimes.InvalidIndex() )
	{
		m_RateLimitLastCommandTimes.Insert( args[0], gpGlobals->curtime );
		return true;
	}
	else if ( (gpGlobals->curtime - m_RateLimitLastCommandTimes[i]) < HL2MP_COMMAND_MAX_RATE )
	{
		// Too fast.
		return false;
	}
	else
	{
		m_RateLimitLastCommandTimes[i] = gpGlobals->curtime;
		return true;
	}
}

void CHL2MP_Player::CreateViewModel( int index /*=0*/ )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );

	if ( GetViewModel( index ) )
		return;

	CPredictedViewModel *vm = ( CPredictedViewModel * )CreateEntityByName( "predicted_viewmodel" );
	if ( vm )
	{
		vm->SetAbsOrigin( GetAbsOrigin() );
		vm->SetOwner( this );
		vm->SetIndex( index );
		DispatchSpawn( vm );
		vm->FollowEntity( this, false );
		m_hViewModel.Set( index, vm );
	}
}

bool CHL2MP_Player::BecomeRagdollOnClient( const Vector &force )
{
	return true;
}

// -------------------------------------------------------------------------------- //
// Ragdoll entities.
// -------------------------------------------------------------------------------- //

class CHL2MPRagdoll : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS( CHL2MPRagdoll, CBaseAnimatingOverlay );
	DECLARE_SERVERCLASS();

	// Transmit ragdolls to everyone.
	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

public:
	// In case the client has the player entity, we transmit the player index.
	// In case the client doesn't have it, we transmit the player's model index, origin, and angles
	// so they can create a ragdoll in the right place.
	CNetworkHandle( CBaseEntity, m_hPlayer );	// networked entity handle 
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
};

LINK_ENTITY_TO_CLASS( hl2mp_ragdoll, CHL2MPRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CHL2MPRagdoll, DT_HL2MPRagdoll )
	SendPropVector( SENDINFO(m_vecRagdollOrigin), -1,  SPROP_COORD ),
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt		( SENDINFO(m_nForceBone), 8, 0 ),
	SendPropVector	( SENDINFO(m_vecForce), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ) )
END_SEND_TABLE()


void CHL2MP_Player::CreateRagdollEntity( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	// If we already have a ragdoll, don't make another one.
	CHL2MPRagdoll *pRagdoll = dynamic_cast< CHL2MPRagdoll* >( m_hRagdoll.Get() );
	
	if ( !pRagdoll )
	{
		// create a new one
		pRagdoll = dynamic_cast< CHL2MPRagdoll* >( CreateEntityByName( "hl2mp_ragdoll" ) );
	}

	if ( pRagdoll )
	{
		pRagdoll->m_hPlayer = this;
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_nModelIndex = m_nModelIndex;
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_vecForce = m_vecTotalBulletForce;
		pRagdoll->SetAbsOrigin( GetAbsOrigin() );
	}

	// ragdolls will be removed on round restart automatically
	m_hRagdoll = pRagdoll;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CHL2MP_Player::FlashlightIsOn( void )
{
	return IsEffectActive( EF_DIMLIGHT );
}

extern ConVar flashlight;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOn( void )
{
	if( flashlight.GetInt() > 0 && IsAlive() )
	{
		AddEffects( EF_DIMLIGHT );
		EmitSound( "HL2Player.FlashlightOn" );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOff( void )
{
	RemoveEffects( EF_DIMLIGHT );
	
	if( IsAlive() )
	{
		EmitSound( "HL2Player.FlashlightOff" );
	}
}

void CHL2MP_Player::Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget, const Vector *pVelocity )
{
	//Drop a grenade if it's primed.
	if ( GetActiveWeapon() )
	{
		CBaseCombatWeapon *pGrenade = Weapon_OwnsThisType("weapon_frag");

		if ( GetActiveWeapon() == pGrenade )
		{
			if ( ( m_nButtons & IN_ATTACK ) || (m_nButtons & IN_ATTACK2) )
			{
				DropPrimedFragGrenade( this, pGrenade );
				return;
			}
		}
	}

	BaseClass::Weapon_Drop( pWeapon, pvecTarget, pVelocity );
}


void CHL2MP_Player::DetonateTripmines( void )
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "npc_satchel" )) != NULL)
	{
		CSatchelCharge *pSatchel = dynamic_cast<CSatchelCharge *>(pEntity);
		if (pSatchel->m_bIsLive && pSatchel->GetThrower() == this )
		{
			g_EventQueue.AddEvent( pSatchel, "Explode", 0.20, this, this );
		}
	}

	// Play sound for pressing the detonator
	EmitSound( "Weapon_SLAM.SatchelDetonate" );
}

void CHL2MP_Player::Event_Killed( const CTakeDamageInfo &info )
{
	//update damage info with our accumulated physics force
	CTakeDamageInfo subinfo = info;
	subinfo.SetDamageForce( m_vecTotalBulletForce );

	SetNumAnimOverlays( 0 );

	// Note: since we're dead, it won't draw us on the client, but we don't set EF_NODRAW
	// because we still want to transmit to the clients in our PVS.
	CreateRagdollEntity();

	DetonateTripmines();

	BaseClass::Event_Killed( subinfo );

	if ( info.GetDamageType() & DMG_DISSOLVE )
	{
		if ( m_hRagdoll )
		{
			m_hRagdoll->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
		}
	}

	CBaseEntity *pAttacker = info.GetAttacker();

	if ( pAttacker )
	{
		int iScoreToAdd = 1;

		if ( pAttacker == this )
		{
			iScoreToAdd = -1;
		}

		GetGlobalTeam( pAttacker->GetTeamNumber() )->AddScore( iScoreToAdd );
	}

	FlashlightTurnOff();

	m_lifeState = LIFE_DEAD;

	RemoveEffects( EF_NODRAW );	// still draw player body
	StopZooming();
}

int CHL2MP_Player::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	//return here if the player is in the respawn grace period vs. slams.
	if ( gpGlobals->curtime < m_flSlamProtectTime &&  (inputInfo.GetDamageType() == DMG_BLAST ) )
		return 0;

	m_vecTotalBulletForce += inputInfo.GetDamageForce();
	
	gamestats->Event_PlayerDamage( this, inputInfo );

	return BaseClass::OnTakeDamage( inputInfo );
}

void CHL2MP_Player::DeathSound( const CTakeDamageInfo &info )
{
	if ( m_hRagdoll && m_hRagdoll->GetBaseAnimating()->IsDissolving() )
		 return;

	char szStepSound[128];

	Q_snprintf( szStepSound, sizeof( szStepSound ), "%s.Die", GetPlayerModelSoundPrefix() );

	const char *pModelName = STRING( GetModelName() );

	CSoundParameters params;
	if ( GetParametersForSound( szStepSound, params, pModelName ) == false )
		return;

	Vector vecOrigin = GetAbsOrigin();
	
	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

	EmitSound_t ep;
	ep.m_nChannel = params.channel;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = params.volume;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );
}

CBaseEntity* CHL2MP_Player::EntSelectSpawnPoint( void )
{
	CBaseEntity *pSpot = NULL;
	CBaseEntity *pLastSpawnPoint = g_pLastSpawn;
	edict_t		*player = edict();
	const char *pSpawnpointName = "info_player_deathmatch";

	if ( HL2MPRules()->IsTeamplay() == true )
	{
		if ( GetTeamNumber() == TEAM_COMBINE )
		{
			pSpawnpointName = "info_player_combine";
			pLastSpawnPoint = g_pLastCombineSpawn;
		}
		else if ( GetTeamNumber() == TEAM_REBELS )
		{
			pSpawnpointName = "info_player_rebel";
			pLastSpawnPoint = g_pLastRebelSpawn;
		}

		if ( gEntList.FindEntityByClassname( NULL, pSpawnpointName ) == NULL )
		{
			pSpawnpointName = "info_player_deathmatch";
			pLastSpawnPoint = g_pLastSpawn;
		}
	}

	pSpot = pLastSpawnPoint;
	// Randomize the start spot
	for ( int i = random->RandomInt(1,5); i > 0; i-- )
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
	if ( !pSpot )  // skip over the null point
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );

	CBaseEntity *pFirstSpot = pSpot;

	do 
	{
		if ( pSpot )
		{
			// check if pSpot is valid
			if ( g_pGameRules->IsSpawnPointValid( pSpot, this ) )
			{
				if ( pSpot->GetLocalOrigin() == vec3_origin )
				{
					pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
					continue;
				}

				// if so, go to pSpot
				goto ReturnSpot;
			}
		}
		// increment pSpot
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
	} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

	// we haven't found a place to spawn yet,  so kill any guy at the first spawn point and spawn there
	if ( pSpot )
	{
		CBaseEntity *ent = NULL;
		for ( CEntitySphereQuery sphere( pSpot->GetAbsOrigin(), 128 ); (ent = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity() )
		{
			// if ent is a client, kill em (unless they are ourselves)
			if ( ent->IsPlayer() && !(ent->edict() == player) )
				ent->TakeDamage( CTakeDamageInfo( GetContainingEntity(INDEXENT(0)), GetContainingEntity(INDEXENT(0)), 300, DMG_GENERIC ) );
		}
		goto ReturnSpot;
	}

	if ( !pSpot  )
	{
		pSpot = gEntList.FindEntityByClassname( pSpot, "info_player_start" );

		if ( pSpot )
			goto ReturnSpot;
	}

ReturnSpot:

	if ( HL2MPRules()->IsTeamplay() == true )
	{
		if ( GetTeamNumber() == TEAM_COMBINE )
		{
			g_pLastCombineSpawn = pSpot;
		}
		else if ( GetTeamNumber() == TEAM_REBELS ) 
		{
			g_pLastRebelSpawn = pSpot;
		}
	}

	g_pLastSpawn = pSpot;

	m_flSlamProtectTime = gpGlobals->curtime + 0.5;

	return pSpot;
} 


CON_COMMAND( timeleft, "prints the time remaining in the match" )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_GetCommandClient() );

	int iTimeRemaining = (int)HL2MPRules()->GetMapRemainingTime();
    
	if ( iTimeRemaining == 0 )
	{
		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "This game has no timelimit." );
		}
		else
		{
			Msg( "* No Time Limit *\n" );
		}
	}
	else
	{
		int iMinutes, iSeconds;
		iMinutes = iTimeRemaining / 60;
		iSeconds = iTimeRemaining % 60;

		char minutes[8];
		char seconds[8];

		Q_snprintf( minutes, sizeof(minutes), "%d", iMinutes );
		Q_snprintf( seconds, sizeof(seconds), "%2.2d", iSeconds );

		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "Time left in map: %s1:%s2", minutes, seconds );
		}
		else
		{
			Msg( "Time Remaining:  %s:%s\n", minutes, seconds );
		}
	}	
}


void CHL2MP_Player::Reset()
{	
	ResetDeathCount();
	ResetFragCount();
}

bool CHL2MP_Player::IsReady()
{
	return m_bReady;
}

void CHL2MP_Player::SetReady( bool bReady )
{
	m_bReady = bReady;
}

void CHL2MP_Player::CheckChatText( char *p, int bufsize )
{
	//Look for escape sequences and replace

	char *buf = new char[bufsize];
	int pos = 0;

	// Parse say text for escape sequences
	for ( char *pSrc = p; pSrc != NULL && *pSrc != 0 && pos < bufsize-1; pSrc++ )
	{
		// copy each char across
		buf[pos] = *pSrc;
		pos++;
	}

	buf[pos] = '\0';

	// copy buf back into p
	Q_strncpy( p, buf, bufsize );

	delete[] buf;	

	const char *pReadyCheck = p;

	HL2MPRules()->CheckChatForReadySignal( this, pReadyCheck );
}

void CHL2MP_Player::State_Transition( HL2MPPlayerState newState )
{
	State_Leave();
	State_Enter( newState );
}


void CHL2MP_Player::State_Enter( HL2MPPlayerState newState )
{
	m_iPlayerState = newState;
	m_pCurStateInfo = State_LookupInfo( newState );

	// Initialize the new state.
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnEnterState )
		(this->*m_pCurStateInfo->pfnEnterState)();
}


void CHL2MP_Player::State_Leave()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnLeaveState )
	{
		(this->*m_pCurStateInfo->pfnLeaveState)();
	}
}


void CHL2MP_Player::State_PreThink()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnPreThink )
	{
		(this->*m_pCurStateInfo->pfnPreThink)();
	}
}


CHL2MPPlayerStateInfo *CHL2MP_Player::State_LookupInfo( HL2MPPlayerState state )
{
	// This table MUST match the 
	static CHL2MPPlayerStateInfo playerStateInfos[] =
	{
		{ STATE_ACTIVE,			"STATE_ACTIVE",			&CHL2MP_Player::State_Enter_ACTIVE, NULL, &CHL2MP_Player::State_PreThink_ACTIVE },
		{ STATE_OBSERVER_MODE,	"STATE_OBSERVER_MODE",	&CHL2MP_Player::State_Enter_OBSERVER_MODE,	NULL, &CHL2MP_Player::State_PreThink_OBSERVER_MODE }
	};

	for ( int i=0; i < ARRAYSIZE( playerStateInfos ); i++ )
	{
		if ( playerStateInfos[i].m_iPlayerState == state )
			return &playerStateInfos[i];
	}

	return NULL;
}

bool CHL2MP_Player::StartObserverMode(int mode)
{
	//we only want to go into observer mode if the player asked to, not on a death timeout
	if ( m_bEnterObserver == true )
	{
		VPhysicsDestroyObject();
		return BaseClass::StartObserverMode( mode );
	}
	return false;
}

void CHL2MP_Player::StopObserverMode()
{
	m_bEnterObserver = false;
	BaseClass::StopObserverMode();
}

void CHL2MP_Player::State_Enter_OBSERVER_MODE()
{
	int observerMode = m_iObserverLastMode;
	if ( IsNetClient() )
	{
		const char *pIdealMode = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_spec_mode" );
		if ( pIdealMode )
		{
			observerMode = atoi( pIdealMode );
			if ( observerMode <= OBS_MODE_FIXED || observerMode > OBS_MODE_ROAMING )
			{
				observerMode = m_iObserverLastMode;
			}
		}
	}
	m_bEnterObserver = true;
	StartObserverMode( observerMode );
}

void CHL2MP_Player::State_PreThink_OBSERVER_MODE()
{
	// Make sure nobody has changed any of our state.
	//	Assert( GetMoveType() == MOVETYPE_FLY );
	Assert( m_takedamage == DAMAGE_NO );
	Assert( IsSolidFlagSet( FSOLID_NOT_SOLID ) );
	//	Assert( IsEffectActive( EF_NODRAW ) );

	// Must be dead.
	Assert( m_lifeState == LIFE_DEAD );
	Assert( pl.deadflag );
}


void CHL2MP_Player::State_Enter_ACTIVE()
{
	SetMoveType( MOVETYPE_WALK );
	
	// md 8/15/07 - They'll get set back to solid when they actually respawn. If we set them solid now and mp_forcerespawn
	// is false, then they'll be spectating but blocking live players from moving.
	// RemoveSolidFlags( FSOLID_NOT_SOLID );
	
	m_Local.m_iHideHUD = 0;
}


void CHL2MP_Player::State_PreThink_ACTIVE()
{
	//we don't really need to do anything here. 
	//This state_prethink structure came over from CS:S and was doing an assert check that fails the way hl2dm handles death
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHL2MP_Player::CanHearAndReadChatFrom( CBasePlayer *pPlayer )
{
	// can always hear the console unless we're ignoring all chat
	if ( !pPlayer )
		return false;

	return true;
}


//Repose

void CHL2MP_Player::statBaseInit()
{
	for (int i = 0; i < STAT_COUNT; i++)
	{
		statsBase.Set(i,random->RandomInt(1, 6) + random->RandomInt(1, 6) + random->RandomInt(1, 6));
		//stat = statMin + rand() % (statMax - statMin + 1);
		calcStat(i);
		//calcMod(i);
	}

	dumpStats(1);
}

//(Re)calculate our modifiers based on our current stats.
void CHL2MP_Player::calcMods(void)
{
	for (int i = 0; i < STAT_COUNT; i++)
	{
		calcMod(i);
	}
}

//(Re)calculate our modifier based on our current stat.
void CHL2MP_Player::calcMod(int stat)
{
	int statClean = checkStat(stat);
	statClean -= 10;
	statClean % 2 ? statClean-- : +statClean;
	modifiers.Set(stat, statClean / 2);
}

//Make sure one particular stat is in its usual bounds.
int CHL2MP_Player::checkStat(int stat)
{
	checkStatBase(stat);
	int statRef = stats.Get(stat);
	//statRef = statsBase[stat];
	//TODO: stat additions here
	if (statRef <= 0) //knockout/death depending on what score's been reduced to 0
	{
		stats.Set(stat, 0);
		switch (stat)
		{
		case STR:
			//TODO: STR 0: death
			break;
		case DEX:
			//TODO: DEX 0: unconsious (ragdoll [spasm]). Should it lead to death, or recovery?
			break;
			//case CON:
			//TODO: CON 0: death
			break;
			//case WIS:
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
		stats.Set(stat, STAT_NORMAL_MAX);
	}
	return stats.Get(stat);
}

//Make sure this base stat is in its usual bounds.
void CHL2MP_Player::checkStatBase(int stat)
{
	if (statsBase.Get(stat) < 0)
	{
		statsBase.Set(stat, 0);
		calcStat(stat); //we had to adjust our base stat, so our functional stat's probably different
	}
	else if (statsBase.Get(stat) > BASESTAT_NORMAL_MAX)
	{
		statsBase.Set(stat, BASESTAT_NORMAL_MAX);
		//TODO: allow exceptions
		calcStat(stat); //we had to adjust our base stat, so our functional stat's probably different
	}
}

//Make sure all stats are in their usual bounds.
void CHL2MP_Player::checkStats(void)
{
	for (int i = 0; i < STAT_COUNT; i++)
	{
		checkStat(i);
	}
}

//(Re)calculate our current stat based on our base stat, our equipment/items, and any buffs/debuffs applied.
int CHL2MP_Player::calcStat(int stat)
{
	//dumpStats(2);
	checkStatBase(stat);
	stats.Set(stat, statsBase.Get(stat));
	//TODO: equipment, items, and statues added here
	//dumpStat(stat,2);
	calcMod(stat);
	//dumpStats(1);
	return stats.Get(stat);
}

//Function to handle changes to the base stat
void CHL2MP_Player::addToStatBase(int stat, int amount)
{
	int statRef = statsBase.Get(stat);
	statsBase.Set(stat, statRef + amount);
	checkStatBase(stat);
	calcStat(stat);
	calcMod(stat);
}

//Dump all of our stat info to console at developer #.
void CHL2MP_Player::dumpStats(int devLevel)
{
	for (int i = 0; i < STAT_COUNT; i++)
	{
		dumpStat(i, devLevel);
	}
}

//Dump our stat info to console at developer #.
void CHL2MP_Player::dumpStat(int stat, int devLevel)
{
	switch (stat)
	{
	case STR:
		DevMsg("STR:", devLevel);
		break;
	case DEX:
		DevMsg("DEX:", devLevel);
		break;
		//case CON:
		//DevMsg("CON:", devLevel);
		break;
	case INT:
		DevMsg("INT:", devLevel);
		break;
		//case WIS:
		//DevMsg("WIS:", devLevel);
		break;
	case CHA:
		DevMsg("CHA:", devLevel);
		break;
	default:
		DevMsg("???: OUT OF BOUNDS?", devLevel); //if we're not one of our enums, we're probably out of bounds
		break;
	}
	//splits are just for formatting options, lines things up nicely when printed to console
	statsBase[stat] < 10 ? DevMsg("  %i ", statsBase.Get(stat), devLevel) : DevMsg(" %i ", statsBase.Get(stat), devLevel);
	stats[stat] < 10 ? DevMsg("|  %i ", stats.Get(stat), devLevel) : DevMsg("| %i ", stats.Get(stat), devLevel);
	modifiers[stat] >= 0 ? DevMsg("(+%i)\n", modifiers.Get(stat), devLevel) : DevMsg("(%i)\n", modifiers.Get(stat), devLevel);
}


//
// SUIT POWER DEVICES
//
#define SUITPOWER_CHARGE_RATE	12.5											// 100 units in 8 seconds
#ifdef HL2MP
CSuitPowerDevice SuitDeviceSprintMP(bits_SUIT_DEVICE_SPRINT, 20.0f); //100 units in 5 seconds			// 25.0f );				// 100 units in 4 seconds
#else
CSuitPowerDevice SuitDeviceSprintMP(bits_SUIT_DEVICE_SPRINT, 12.5f);				// 100 units in 8 seconds
#endif

#ifdef HL2_EPISODIC
CSuitPowerDevice SuitDeviceFlashlightMP(bits_SUIT_DEVICE_FLASHLIGHT, 1.111);	// 100 units in 90 second
#else
CSuitPowerDevice SuitDeviceFlashlightMP(bits_SUIT_DEVICE_FLASHLIGHT, 2.222);	// 100 units in 45 second
#endif
CSuitPowerDevice SuitDeviceBreatherMP(bits_SUIT_DEVICE_BREATHER, 6.7f);		// 100 units in 15 seconds (plus three padded seconds)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::SuitPower_Update(void)
{
	if (SuitPower_ShouldRecharge())
	{
		//REPOSE: Speed up/slow down recharge rate based on DEX


		float rate = float(checkMod(DEX));
		rate > 0 ? rate *= 2.0f : rate *= 1.7f; //nerf effects of rate for negative stats so they aren't quite such a terrible drain
		SuitPower_Charge((SUITPOWER_CHARGE_RATE + rate) * gpGlobals->frametime);
	}
	else if (m_HL2Local.m_bitsActiveDevices)
	{
		float flPowerLoad = m_flSuitPowerLoad;

		//Since stickysprint quickly shuts off sprint if it isn't being used, this isn't an issue.
		if (!sv_stickysprint.GetBool())
		{
			if (SuitPower_IsDeviceActive(SuitDeviceSprintMP))
			{
				if (!fabs(GetAbsVelocity().x) && !fabs(GetAbsVelocity().y))
				{
					// If player's not moving, don't drain sprint juice.
					int loops = -checkMod(DEX);
					float rate;
					if (loops <= 0) rate = float(loops) * 1.5f;
					else
					{
						rate = 2.0f;
						while (--loops > 0) rate *= 2.0f;
					}
					flPowerLoad -= SuitDeviceSprintMP.GetDeviceDrainRate() + rate;
				}
			}
		}

		if (SuitPower_IsDeviceActive(SuitDeviceFlashlightMP))
		{
			float factor;

			factor = 1.0f / m_flFlashlightPowerDrainScale;

			flPowerLoad -= (SuitDeviceFlashlightMP.GetDeviceDrainRate() * (1.0f - factor));
		}

		if (!SuitPower_Drain(flPowerLoad * gpGlobals->frametime))
		{
			// TURN OFF ALL DEVICES!!
			if (IsSprinting())
			{
				StopSprinting();
			}

			if (true)//(Flashlight_UseLegacyVersion())
			{
				if (FlashlightIsOn())
				{
#ifndef HL2MP
					FlashlightTurnOff();
#endif
				}
			}
		}

		if (true)//(Flashlight_UseLegacyVersion())
		{
			// turn off flashlight a little bit after it hits below one aux power notch (5%)
			if (m_HL2Local.m_flSuitPower < 4.8f && FlashlightIsOn())
			{
#ifndef HL2MP
				FlashlightTurnOff();
#endif
			}
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHL2MP_Player::SuitPower_AddDevice(const CSuitPowerDevice &device)
{
	// Make sure this device is NOT active!!
	if (m_HL2Local.m_bitsActiveDevices & device.GetDeviceID())
		return false;

	if (!IsSuitEquipped())
		return false;

	m_HL2Local.m_bitsActiveDevices |= device.GetDeviceID();
	m_flSuitPowerLoad += device.GetDeviceDrainRate();
	//REPOSE: Drain more/less quickly based on DEX
	if (device.GetDeviceID()&bits_SUIT_DEVICE_SPRINT)
	{
		int loops = -checkMod(DEX);
		float rate;
		if (loops <= 0) rate = float(loops) * 1.5f;
		else
		{
			rate = 2.0f;
			while (--loops > 0) rate *= 2.0f;
		}
		m_flSuitPowerLoad += rate;
	}
	return true;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHL2MP_Player::SuitPower_RemoveDevice(const CSuitPowerDevice &device)
{
	// Make sure this device is active!!
	if (!(m_HL2Local.m_bitsActiveDevices & device.GetDeviceID()))
		return false;

	if (!IsSuitEquipped())
		return false;

	// Take a little bit of suit power when you disable a device. If the device is shutting off
	// because the battery is drained, no harm done, the battery charge cannot go below 0. 
	// This code in combination with the delay before the suit can start recharging are a defense
	// against exploits where the player could rapidly tap sprint and never run out of power.
	SuitPower_Drain(max(0.1f, device.GetDeviceDrainRate() * 0.1f)); //always wanna drain something

	m_HL2Local.m_bitsActiveDevices &= ~device.GetDeviceID();
	m_flSuitPowerLoad -= device.GetDeviceDrainRate();

	if (m_flSuitPowerLoad < 0.0f) m_flSuitPowerLoad = 0.0f; //just in case something funny happens

	//REPOSE: Matches adjustments made above to remove the adjusted drain
	if (device.GetDeviceID()&bits_SUIT_DEVICE_SPRINT)
	{
		int loops = -checkMod(DEX);
		float rate;
		if (loops <= 0) rate = float(loops) * 1.5f;
		else
		{
			rate = 2.0f;
			while (--loops > 0) rate *= 2.0f;
		}
		m_flSuitPowerLoad -= rate;
	}

	if (m_HL2Local.m_bitsActiveDevices == 0x00000000)
	{
		m_flSuitPowerLoad = 0.0f; //REPOSE: safety added to make sure our drain is properly reset when no devices are in use
		// With this device turned off, we can set this timer which tells us when the
		// suit power system entered a no-load state.
		m_flTimeAllSuitDevicesOff = gpGlobals->curtime;
	}

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define SUITPOWER_BEGIN_RECHARGE_DELAY	0.5f
bool CHL2MP_Player::SuitPower_ShouldRecharge(void)
{
	// Make sure all devices are off.
	if (m_HL2Local.m_bitsActiveDevices != 0x00000000)
		return false;

	// Is the system fully charged?
	if (m_HL2Local.m_flSuitPower > 100.0f)
	{
		m_HL2Local.m_flSuitPower = 100.0f; //make sure we're not over
		return false;
	}
	else if ((m_HL2Local.m_flSuitPower == 100.0f))
		return false;

	// Has the system been in a no-load state for long enough
	// to begin recharging?
	//REPOSE: add to/subtract from recharge delay based on DEX
	float rate = float(checkMod(DEX));
	rate > 0 ? rate *= -0.05f : rate *= -0.15f; //reduce wait time to recharge for high dex, delay it for low.
	if (gpGlobals->curtime < m_flTimeAllSuitDevicesOff + SUITPOWER_BEGIN_RECHARGE_DELAY + rate)
		return false;

	return true;
}


bool CHL2MP_Player::Weapon_CanUse(CBaseCombatWeapon *pWeapon)
{
#ifndef HL2MP	
	if (pWeapon->ClassMatches("weapon_stunstick"))
	{
		if (ApplyBattery(0.5))
			UTIL_Remove(pWeapon);
		return false;
	}
#endif
	bool ret = BaseClass::Weapon_CanUse(pWeapon);
	//hate to YanDev this but can't do a switch statement for strings :c
	if (pWeapon->ClassMatches("weapon_smg1")) //starts with most likely weapons for players to see (NPCs carry them)
		(modifiers.Get(STR) >= 0) ? ret = true : ret = false;
	else if (pWeapon->ClassMatches("weapon_ar2"))
		(modifiers.Get(DEX) >= 0) ? ret = true : ret = false;
	else if (pWeapon->ClassMatches("weapon_shotgun"))
		(modifiers.Get(STR) >= 1) ? ret = true : ret = false;
	else if (pWeapon->ClassMatches("weapon_frag"))
		(modifiers.Get(INT) >= 0) ? ret = true : ret = false;
	else if (pWeapon->ClassMatches("weapon_357"))
		(modifiers.Get(DEX) >= 1) ? ret = true : ret = false;
	else if (pWeapon->ClassMatches("weapon_crossbow"))
		(modifiers.Get(DEX) >= 2) ? ret = true : ret = false;
	else if (pWeapon->ClassMatches("weapon_rpg"))
		(modifiers.Get(STR) >= 2) ? ret = true : ret = false;
	//pistol, crowbar, etc. have no special requirements so no need to check for them, our base check just returns true
	return ret;
}