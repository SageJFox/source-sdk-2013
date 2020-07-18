//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "ai_motor.h"
#include "ai_memory.h"
#include "ai_route.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "entitylist.h"
#include "ai_task.h"
#include "activitylist.h"
#include "engine/IEngineSound.h"
#include "npc_BaseZombie.h"
#include "movevars_shared.h"
#include "IEffects.h"
#include "props.h"
#include "physics_npc_solver.h"
#include "physics_prop_ragdoll.h"
#include "ai_behavior_assault.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define HOUND_IDLE_PITCH			90
#define HOUND_MIN_PITCH			95
#define HOUND_MAX_PITCH			130
#define HOUND_SOUND_UPDATE_FREQ	0.5

#define HOUND_MAXLEAP_Z		128

#define HOUND_EXCITE_DIST 480.0

#define HOUND_BASE_FREQ 1.5

// If flying at an enemy, and this close or closer, start playing the maul animation!!
#define HOUND_MAUL_RANGE	300

#define	HOUND_MIN_BUGBAIT_GOAL_TARGET_RADIUS	512
#define	HOUND_BUGBAIT_NAV_TOLERANCE	200

enum
{
	COND_HOUND_CLIMB_TOUCH	= LAST_BASE_ZOMBIE_CONDITION,
};

envelopePoint_t envHoundVolumeJump[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.1f,
	},
	{	0.0f, 0.0f,
		1.0f, 1.2f,
	},
};

envelopePoint_t envHoundVolumePain[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.1f,
	},
	{	0.0f, 0.0f,
		1.0f, 1.0f,
	},
};

envelopePoint_t envHoundInverseVolumePain[] =
{
	{	0.0f, 0.0f,
		0.1f, 0.1f,
	},
	{	1.0f, 1.0f,
		1.0f, 1.0f,
	},
};

envelopePoint_t envHoundVolumeJumpPostApex[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.1f,
	},
	{	0.0f, 0.0f,
		1.0f, 1.2f,
	},
};

envelopePoint_t envHoundMoanVolumeFast[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.1f,
	},
	{	0.0f, 0.0f,
		0.2f, 0.3f,
	},
};

envelopePoint_t envHoundMoanVolume[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.1f,
	},
	{	1.0f, 1.0f,
		0.2f, 0.2f,
	},
	{	0.0f, 0.0f,
		1.0f, 0.4f,
	},
};

envelopePoint_t envHoundFootstepVolume[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.1f,
	},
	{	0.7f, 0.7f,
		0.2f, 0.2f,
	},
};

envelopePoint_t envHoundVolumeFrenzy[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.1f,
	},
	{	0.0f, 0.0f,
		2.0f, 2.0f,
	},
};

ConVar	sk_hound_dmg_bite("sk_hound_dmg_bite", "0"); //6
ConVar	sk_hound_dmg_leap("sk_hound_dmg_leap", "0"); //15
ConVar	sk_hound_health("sk_hound_health", "0");	 //50
extern ConVar bugbait_radius;

//=========================================================
// animation events
//=========================================================
int AE_HOUND_LEAP;
int AE_HOUND_GALLOP_LEFT;
int AE_HOUND_GALLOP_RIGHT;

//=========================================================
// tasks
//=========================================================
enum 
{
	TASK_HOUND_DO_ATTACK = LAST_SHARED_TASK + 100,	// again, my !!!HACKHACK
	TASK_HOUND_LAND_RECOVER,
	TASK_HOUND_UNSTICK_JUMP,
	TASK_HOUND_JUMP_BACK,
	TASK_HOUND_VERIFY_ATTACK,
	TASK_HOUND_GET_PATH_TO_BUGBAIT,
	TASK_HOUND_FACE_BUGBAIT,
};

//=========================================================
// activities
//=========================================================
int ACT_HOUND_LEAP_SOAR;
int ACT_HOUND_LEAP_STRIKE;
int ACT_HOUND_LAND_RIGHT;
int ACT_HOUND_LAND_LEFT;
int ACT_HOUND_FRENZY;
int ACT_HOUND_BIG_SLASH;

//=========================================================
// schedules
//=========================================================
enum
{
	SCHED_HOUND_RANGE_ATTACK1 = LAST_SHARED_SCHEDULE + 100, // hack to get past the base zombie's schedules
	SCHED_HOUND_UNSTICK_JUMP,
	SCHED_HOUND_MELEE_ATTACK1,
	SCHED_HOUND_CHASE_BUGBAIT,
};



//=========================================================
//=========================================================
class CHound : public CNPC_BaseZombie
{
	DECLARE_CLASS( CHound, CNPC_BaseZombie );

public:
	void Spawn( void );
	void Precache( void );
	Class_T Classify(void);

	void SetZombieModel( void );
	bool CanSwatPhysicsObjects( void ) { return false; }

	int	TranslateSchedule( int scheduleType );

	void LeapAttackTouch( CBaseEntity *pOther );

	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );
	int SelectSchedule( void );
	void OnScheduleChange( void );

	void PrescheduleThink( void );

	float InnateRange1MaxRange( void );
	int RangeAttack1Conditions( float flDot, float flDist );
	int MeleeAttack1Conditions( float flDot, float flDist );

	virtual float GetClawAttackRange() const { return 50; }

	bool ShouldPlayFootstepMoan( void ) { return false; }

	void HandleAnimEvent( animevent_t *pEvent );

	void PostNPCInit( void );

	void LeapAttack( void );
	void LeapAttackSound( void );

	void BecomeTorso( const Vector &vecTorsoForce, const Vector &vecLegsForce );

	bool IsJumpLegal(const Vector &startPos, const Vector &apex, const Vector &endPos) const;
	bool MovementCost( int moveType, const Vector &vecStart, const Vector &vecEnd, float *pCost );
	bool ShouldFailNav( bool bMovementFailed );

	int	SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );

	const char *GetMoanSound( int nSound );

	void OnChangeActivity( Activity NewActivity );
	void OnStateChange( NPC_STATE OldState, NPC_STATE NewState );
	void Event_Killed( const CTakeDamageInfo &info );
	bool ShouldBecomeTorso( const CTakeDamageInfo &info, float flDamageThreshold );
	virtual HeadcrabRelease_t ShouldReleaseHeadcrab(const CTakeDamageInfo &info, float flDamageThreshold);

	virtual Vector GetAutoAimCenter() { return WorldSpaceCenter() - Vector( 0, 0, 12.0f ); }

	void PainSound( const CTakeDamageInfo &info );
	void DeathSound( const CTakeDamageInfo &info ); 
	void AlertSound( void );
	void IdleSound( void );
	void AttackSound( void );
	void AttackHitSound( void );
	void AttackMissSound( void );
	void FootstepSound( bool fRightFoot );
	void FootscuffSound( bool fRightFoot ) {}; // fast guy doesn't scuff
	void StopLoopingSounds( void );

	void SoundInit( void );
	void SetIdleSoundState( void );
	void SetAngrySoundState( void );

	void BuildScheduleTestBits( void );

	void BeginNavJump( void );
	void EndNavJump( void );

	bool IsNavJumping( void ) { return m_fIsNavJumping; }
	void OnNavJumpHitApex( void );

	void BeginAttackJump( void );
	void EndAttackJump( void );

	float		MaxYawSpeed( void );

	virtual const char *GetHeadcrabClassname( void );
	virtual const char *GetHeadcrabModel( void );
	virtual const char *GetLegsModel( void );
	virtual const char *GetTorsoModel( void );

	//bugbait chasing
	bool		CreateBehaviors(void);
	bool		IsValidEnemy(CBaseEntity *pEnemy);
	bool		QueryHearSound(CSound *pSound);

protected:

	static const char *pMoanSounds[];

	// Sound stuff
	float			m_flDistFactor; 
	bool			m_fIsNavJumping;
	bool			m_fIsAttackJumping;
	bool			m_fHitApex;
	mutable float	m_flJumpDist;

	bool			m_fHasScreamed;

private:
	float	m_flNextMeleeAttack;
	bool	m_fJustJumped;
	float	m_flJumpStartAltitude;
	float	m_flTimeUpdateSound;
	//How long until we can make an angry sound (so as to not spam them)
	float	m_flNextAngrySound = 0.0f;

	CSoundPatch	*m_pLayer2; // used when jumping (pre apex)

	//bugbait reaction

	CAI_AssaultBehavior			m_AssaultBehavior;

	bool	FindChasePosition(const Vector &targetPos, Vector &result);
	bool	GetGroundPosition(const Vector &testPos, Vector &result);
	void	LockJumpNode(void);
	float	m_flNextAcknowledgeTime = 0.0f; //allowed to make a sound affirming we've heard bugbait?

	Vector		m_vecHeardSound;
	bool		m_bHasHeardSound;
	float		m_flIgnoreSoundTime;		// Sound time to ignore if earlier than

public:
	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( npc_hound, CHound );


BEGIN_DATADESC( CHound )

	DEFINE_FIELD( m_flDistFactor, FIELD_FLOAT ),
	DEFINE_FIELD( m_fIsNavJumping, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fIsAttackJumping, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fHitApex, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flJumpDist, FIELD_FLOAT ),
	DEFINE_FIELD( m_fHasScreamed, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flNextMeleeAttack, FIELD_TIME ),
	DEFINE_FIELD( m_fJustJumped, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flJumpStartAltitude, FIELD_FLOAT ),
	DEFINE_FIELD( m_flTimeUpdateSound, FIELD_TIME ),
	DEFINE_FIELD(m_flNextAngrySound, FIELD_TIME),

	// Function Pointers
	DEFINE_ENTITYFUNC( LeapAttackTouch ),
	DEFINE_SOUNDPATCH( m_pLayer2 ),

END_DATADESC()


const char *CHound::pMoanSounds[] =
{
	"NPC_Hound.Moan1",
};

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CHound::Precache( void )
{
	PrecacheModel( "models/repose/hound.mdl" );

	PrecacheScriptSound( "NPC_Hound.LeapAttack" );
	PrecacheScriptSound( "NPC_Hound.FootstepRight" );
	PrecacheScriptSound( "NPC_Hound.FootstepLeft" );
	PrecacheScriptSound( "NPC_Hound.AttackHit" );
	PrecacheScriptSound( "NPC_Hound.AttackMiss" );
	PrecacheScriptSound( "NPC_Hound.LeapAttack" );
	PrecacheScriptSound( "NPC_Hound.Attack" );
	PrecacheScriptSound( "NPC_Hound.Idle" );
	PrecacheScriptSound( "NPC_Hound.AlertFar" );
	PrecacheScriptSound( "NPC_Hound.AlertNear" );
	PrecacheScriptSound( "NPC_Hound.GallopLeft" );
	PrecacheScriptSound( "NPC_Hound.GallopRight" );
	PrecacheScriptSound( "NPC_Hound.Scream" );
	PrecacheScriptSound( "NPC_Hound.RangeAttack" );
	PrecacheScriptSound( "NPC_Hound.Frenzy" );
	PrecacheScriptSound( "NPC_Hound.NoSound" );
	PrecacheScriptSound( "NPC_Hound.Die" );

	PrecacheScriptSound( "NPC_Hound.Gurgle" );

	PrecacheScriptSound( "NPC_Hound.Moan1" );

	BaseClass::Precache();
}

//---------------------------------------------------------
//---------------------------------------------------------
void CHound::OnScheduleChange( void )
{
	if ( m_flNextMeleeAttack > gpGlobals->curtime + 1 )
	{
		// Allow melee attacks again.
		m_flNextMeleeAttack = gpGlobals->curtime + 0.5;
	}

	BaseClass::OnScheduleChange();
}

//---------------------------------------------------------
//---------------------------------------------------------
int CHound::SelectSchedule ( void )
{
	if ( HasCondition( COND_ZOMBIE_RELEASECRAB ) )
	{
		// Death waits for no man. Or zombie. Or something.
		return SCHED_DIE_RAGDOLL; //SCHED_ZOMBIE_RELEASECRAB;
	}

	if ( HasCondition( COND_HOUND_CLIMB_TOUCH ) )
	{
		return SCHED_HOUND_UNSTICK_JUMP;
	}

	switch ( m_NPCState )
	{
	case NPC_STATE_COMBAT:
		if ( HasCondition( COND_LOST_ENEMY ) || ( HasCondition( COND_ENEMY_UNREACHABLE ) && MustCloseToAttack() ) )
		{
			// Set state to alert and recurse!
			SetState( NPC_STATE_ALERT );
			return SelectSchedule();
		}
		break;

	case NPC_STATE_ALERT:
		if ( HasCondition( COND_LOST_ENEMY ) || ( HasCondition( COND_ENEMY_UNREACHABLE ) && MustCloseToAttack() ) )
		{
			ClearCondition( COND_LOST_ENEMY );
			ClearCondition( COND_ENEMY_UNREACHABLE );
			SetEnemy( NULL );

#ifdef DEBUG_ZOMBIES
			DevMsg("Wandering\n");
#endif

			// Just lost track of our enemy. 
			// Wander around a bit so we don't look like a dingus.
			return SCHED_ZOMBIE_WANDER_MEDIUM;
		}
		break;
	}

	//Hear bug bait splattered?
	if (HasCondition(COND_HEAR_BUGBAIT))
	{
		//Play a special sound
		if (m_flNextAcknowledgeTime < gpGlobals->curtime)
		{
			EmitSound("NPC_Hound.Distracted");
			m_flNextAcknowledgeTime = gpGlobals->curtime + 1.0f;
		}

		m_flNextAngrySound = gpGlobals->curtime + 4.0f;

		//If the sound is valid, act upon it
		if (m_bHasHeardSound)
		{
			//Mark anything in the area as more interesting
			CBaseEntity *pTarget = NULL;
			CBaseEntity *pNewEnemy = NULL;
			Vector		soundOrg = m_vecHeardSound;

			//Find all entities within that sphere
			while ((pTarget = gEntList.FindEntityInSphere(pTarget, soundOrg, bugbait_radius.GetInt())) != NULL)
			{
				CAI_BaseNPC *pNPC = pTarget->MyNPCPointer();

				if (pNPC == NULL)
					continue;

				if (pNPC->CanBeAnEnemyOf(this) == false)
					continue;

				//Check to see if the default relationship is hatred, and if so intensify that
				if ((IRelationType(pNPC) == D_HT) && (pNPC->IsPlayer() == false))
				{
					AddEntityRelationship(pNPC, D_HT, 99);

					//Try to spread out the enemy distribution
					if ((pNewEnemy == NULL) || (random->RandomInt(0, 1)))
					{
						pNewEnemy = pNPC;
						continue;
					}
				}
			}

			// If we have a new enemy, take it
			if (pNewEnemy != NULL)
			{
				//Setup our ignore info
				SetEnemy(pNewEnemy);
			}

			ClearCondition(COND_HEAR_BUGBAIT);

			return SCHED_HOUND_CHASE_BUGBAIT;
		}
	}

	if (m_AssaultBehavior.CanSelectSchedule())
	{
		DeferSchedulingToBehavior(&m_AssaultBehavior);
		return BaseClass::SelectSchedule();
	}

	return BaseClass::SelectSchedule();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHound::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();

	if( GetGroundEntity() && GetGroundEntity()->Classify() == CLASS_HEADCRAB )
	{
		// Kill!
		CTakeDamageInfo info;
		info.SetDamage( GetGroundEntity()->GetHealth() );
		info.SetAttacker( this );
		info.SetInflictor( this );
		info.SetDamageType( DMG_GENERIC );
		GetGroundEntity()->TakeDamage( info );
	}

 	if( m_pMoanSound && gpGlobals->curtime > m_flTimeUpdateSound )
	{
		// Manage the snorting sound, pitch up for closer.
		float flDistNoBBox;

		if( GetEnemy() && m_NPCState == NPC_STATE_COMBAT )
		{
			flDistNoBBox = ( GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter() ).Length();
			flDistNoBBox -= WorldAlignSize().x;
		}
		else
		{
			// Calm down!
			flDistNoBBox = HOUND_EXCITE_DIST;
			m_flTimeUpdateSound += 1.0;
		}

		if( flDistNoBBox >= HOUND_EXCITE_DIST && m_flDistFactor != 1.0 )
		{
			// Go back to normal pitch.
			m_flDistFactor = 1.0;

			ENVELOPE_CONTROLLER.SoundChangePitch( m_pMoanSound, HOUND_IDLE_PITCH, HOUND_SOUND_UPDATE_FREQ );
		}
		else if( flDistNoBBox < HOUND_EXCITE_DIST )
		{
			// Zombie is close! Recalculate pitch.
			int iPitch;

			m_flDistFactor = MIN( 1.0, 1 - flDistNoBBox / HOUND_EXCITE_DIST ); 
			iPitch = HOUND_MIN_PITCH + ( ( HOUND_MAX_PITCH - HOUND_MIN_PITCH ) * m_flDistFactor); 
			ENVELOPE_CONTROLLER.SoundChangePitch( m_pMoanSound, iPitch, HOUND_SOUND_UPDATE_FREQ );
		}

		m_flTimeUpdateSound = gpGlobals->curtime + HOUND_SOUND_UPDATE_FREQ;
	}

	// Crudely detect the apex of our jump
	if( IsNavJumping() && !m_fHitApex && GetAbsVelocity().z <= 0.0 )
	{
		OnNavJumpHitApex();
	}

	if( IsCurSchedule(SCHED_HOUND_RANGE_ATTACK1, false) )
	{
		// Think more frequently when flying quickly through the 
		// air, to update the server's location more often.
		SetNextThink(gpGlobals->curtime);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Startup all of the sound patches that the fast zombie uses.
//
//
//-----------------------------------------------------------------------------
void CHound::SoundInit( void )
{
	if( !m_pMoanSound )
	{
		// !!!HACKHACK - kickstart the moan sound. (sjb)
		MoanSound( envHoundMoanVolume, ARRAYSIZE( envHoundMoanVolume ) );

		// Clear the commands that the base class gave the moaning sound channel.
		ENVELOPE_CONTROLLER.CommandClear( m_pMoanSound );
	}

	CPASAttenuationFilter filter( this );

	if( !m_pLayer2 )
	{
		// Set up layer2
		m_pLayer2 = ENVELOPE_CONTROLLER.SoundCreate( filter, entindex(), CHAN_VOICE, "NPC_Hound.Gurgle", ATTN_NORM );

		// Start silent.
		ENVELOPE_CONTROLLER.Play( m_pLayer2, 0.0, 100 );
	}

	SetIdleSoundState();
}

//-----------------------------------------------------------------------------
// Purpose: Make the zombie sound calm.
//-----------------------------------------------------------------------------
void CHound::SetIdleSoundState( void )
{
	// Main looping sound
	if ( m_pMoanSound )
	{
		ENVELOPE_CONTROLLER.SoundChangePitch( m_pMoanSound, HOUND_IDLE_PITCH, 1.0 );
		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pMoanSound, 0.01f, 0.75f ); //0.75, 1.0
	}

	// Second Layer
	if ( m_pLayer2 )
	{
		ENVELOPE_CONTROLLER.SoundChangePitch( m_pLayer2, 100, 1.0 );
		ENVELOPE_CONTROLLER.SoundChangeVolume( m_pLayer2, 0.0, 1.0 );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Make the zombie sound pizzled
//-----------------------------------------------------------------------------
void CHound::SetAngrySoundState( void )
{
	if (( !m_pMoanSound ) || ( !m_pLayer2 ))
	{
		return;
	}

	if (m_flNextAngrySound < gpGlobals->curtime) //sound spams out without this when we fail to navigate to our target
	{
		EmitSound("NPC_Hound.LeapAttack");
		m_flNextAngrySound = gpGlobals->curtime + 1.0f;
	}

	// Main looping sound
	ENVELOPE_CONTROLLER.SoundChangePitch( m_pMoanSound, HOUND_MIN_PITCH, 0.5 );
	ENVELOPE_CONTROLLER.SoundChangeVolume( m_pMoanSound, 1.0, 0.5 );

	// Second Layer
	ENVELOPE_CONTROLLER.SoundChangePitch( m_pLayer2, 100, 1.0 );
	ENVELOPE_CONTROLLER.SoundChangeVolume( m_pLayer2, 0.0, 1.0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CHound::Spawn( void )
{
	Precache();

	m_fJustJumped = false;

	m_fIsTorso = m_fIsHeadless = false;

	SetBloodColor( BLOOD_COLOR_RED );

	m_iHealth			= sk_hound_health.GetInt();
	m_flFieldOfView		= 0.2;

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_JUMP | bits_CAP_MOVE_GROUND | bits_CAP_INNATE_RANGE_ATTACK1 );


	m_flNextAttack = gpGlobals->curtime;

	m_pLayer2 = NULL;
	//m_iClimbCount = 0;

	EndNavJump();

	m_flDistFactor = 1.0;

	m_vecHeardSound = WorldSpaceCenter();
	m_bHasHeardSound = false;
	m_flIgnoreSoundTime = 0.0f;

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHound::PostNPCInit( void )
{
	SoundInit();

	m_flTimeUpdateSound = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the classname (ie "npc_headcrab") to spawn when our headcrab bails.
//-----------------------------------------------------------------------------
const char *CHound::GetHeadcrabClassname( void )
{
	return ""; //Don't spawn headcrabs when hound dies!
	//having nothing causes an error in console on our first load, but nothing really too concerning.
}

const char *CHound::GetHeadcrabModel( void )
{
	return "";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CHound::MaxYawSpeed( void )
{
	switch( GetActivity() )
	{
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		return 120;
		break;

	case ACT_RUN:
		return 160;
		break;

	case ACT_WALK:
	case ACT_IDLE:
		return 25;
		break;
		
	default:
		return 20;
		break;
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CHound::SetZombieModel( void )
{
	SetModel("models/repose/hound.mdl");
	SetHullType(HULL_MEDIUM); //HULL_TINY?

	SetHullSizeNormal( true );
	SetDefaultEyeOffset();
	SetActivity( ACT_IDLE );
}


//-----------------------------------------------------------------------------
// Purpose: Returns the model to use for our legs ragdoll when we are blown in twain.
//-----------------------------------------------------------------------------
const char *CHound::GetLegsModel( void )
{
	return "";	//s_pLegsModel;
}

const char *CHound::GetTorsoModel( void )
{
	return "";	//"models/gibs/fast_zombie_torso.mdl";
}


//-----------------------------------------------------------------------------
// Purpose: See if I can swat the player
//
//
//-----------------------------------------------------------------------------
int CHound::MeleeAttack1Conditions( float flDot, float flDist )
{
	if ( !GetEnemy() )
	{
		return COND_NONE;
	}

	if( !(GetFlags() & FL_ONGROUND) )
	{
		// Have to be on the ground!
		return COND_NONE;
	}

	if( gpGlobals->curtime < m_flNextMeleeAttack )
	{
		return COND_NONE;
	}
	
	int baseResult = BaseClass::MeleeAttack1Conditions( flDot, flDist );

	// @TODO (toml 07-21-04): follow up with Steve to find out why fz was explicitly not using these conditions
	if ( baseResult == COND_TOO_FAR_TO_ATTACK || baseResult == COND_NOT_FACING_ATTACK )
	{
		return COND_NONE;
	}

	return baseResult;
}

//-----------------------------------------------------------------------------
// Purpose: Returns a moan sound for this class of zombie.
//-----------------------------------------------------------------------------
const char *CHound::GetMoanSound( int nSound )
{
	return pMoanSounds[ nSound % ARRAYSIZE( pMoanSounds ) ];
}

//-----------------------------------------------------------------------------
// Purpose: Sound of a footstep
//-----------------------------------------------------------------------------
void CHound::FootstepSound( bool fRightFoot )
{
	if( fRightFoot )
	{
		EmitSound( "NPC_Hound.FootstepRight" );
	}
	else
	{
		EmitSound( "NPC_Hound.FootstepLeft" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack hit sound
//-----------------------------------------------------------------------------
void CHound::AttackHitSound( void )
{
	EmitSound( "NPC_Hound.AttackHit" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack miss sound
//-----------------------------------------------------------------------------
void CHound::AttackMissSound( void )
{
	// Play a random attack miss sound
	EmitSound( "NPC_Hound.AttackMiss" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack sound.
//-----------------------------------------------------------------------------
void CHound::LeapAttackSound( void )
{
	EmitSound( "NPC_Hound.LeapAttack" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack sound.
//-----------------------------------------------------------------------------
void CHound::AttackSound( void )
{
	EmitSound( "NPC_Hound.Attack" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random idle sound.
//-----------------------------------------------------------------------------
void CHound::IdleSound( void )
{
	EmitSound( "NPC_Hound.Idle" );
	//MakeAISpookySound( 360.0f );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random pain sound.
//-----------------------------------------------------------------------------
void CHound::PainSound( const CTakeDamageInfo &info )
{
	if ( m_pLayer2 )
		ENVELOPE_CONTROLLER.SoundPlayEnvelope( m_pLayer2, SOUNDCTRL_CHANGE_VOLUME, envHoundVolumePain, ARRAYSIZE(envHoundVolumePain) );
	if ( m_pMoanSound )
		ENVELOPE_CONTROLLER.SoundPlayEnvelope( m_pMoanSound, SOUNDCTRL_CHANGE_VOLUME, envHoundInverseVolumePain, ARRAYSIZE(envHoundInverseVolumePain) );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHound::DeathSound( const CTakeDamageInfo &info ) 
{
	EmitSound( "NPC_Hound.Die" );
}

//-----------------------------------------------------------------------------
// Purpose: Play a random alert sound.
//-----------------------------------------------------------------------------
void CHound::AlertSound( void )
{
	CBaseEntity *pPlayer = UTIL_GetNearestPlayer(this,true);
	if (!pPlayer)
		pPlayer = UTIL_GetNearestPlayer(this);
	if( pPlayer )
	{
		// Measure how far the nearest player is, and play the appropriate type of alert sound. 
		// Doesn't matter if I'm getting mad at a different character, the players are the
		// ones that hear the sound.
		float flDist;

		flDist = ( GetAbsOrigin() - pPlayer->GetAbsOrigin() ).Length();

		if( flDist > 512 )
		{
			EmitSound( "NPC_Hound.AlertFar" );
		}
		else
		{
			EmitSound( "NPC_Hound.AlertNear" );
		}
	}

}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define HOUND_MINLEAP			200
#define HOUND_MAXLEAP			300
float CHound::InnateRange1MaxRange( void ) 
{ 
	return HOUND_MAXLEAP; 
}


//-----------------------------------------------------------------------------
// Purpose: See if I can make my leaping attack!!
//
//
//-----------------------------------------------------------------------------
int CHound::RangeAttack1Conditions( float flDot, float flDist )
{

	if (GetEnemy() == NULL)
	{
		return( COND_NONE );
	}

	if( !(GetFlags() & FL_ONGROUND) )
	{
		return COND_NONE;
	}

	if( gpGlobals->curtime < m_flNextAttack )
	{
		return( COND_NONE );
	}

	// make sure the enemy isn't on a roof and I'm in the streets (Ravenholm)
	float flZDist;
	flZDist = fabs( GetEnemy()->GetLocalOrigin().z - GetLocalOrigin().z );
	if( flZDist > HOUND_MAXLEAP_Z )
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	if( flDist > InnateRange1MaxRange() )
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	if( flDist < HOUND_MINLEAP )
	{
		return COND_NONE;
	}

	if (flDot < 0.8) 
	{
		return COND_NONE;
	}

	if ( !IsMoving() )
	{
		// I Have to be running!!!
		return COND_NONE;
	}

	// Don't jump at the player unless he's facing me.
	// This allows the player to get away if he turns and sprints
	/*CBasePlayer *pPlayer = static_cast<CBasePlayer*>( GetEnemy() );

	if( pPlayer )
	{
		// If the enemy is a player, don't attack from behind!
		if( !pPlayer->FInViewCone( this ) )
		{
			return COND_NONE;
		}
	}*/

	// Drumroll please!
	// The final check! Is the path from my position to halfway between me
	// and the player clear?
	trace_t tr;
	Vector vecDirToEnemy;

	vecDirToEnemy = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter();
	Vector vecHullMin( -16, -16, -16 );
	Vector vecHullMax( 16, 16, 16 );

	// only check half the distance. (the first part of the jump)
	vecDirToEnemy = vecDirToEnemy * 0.5;

	AI_TraceHull( WorldSpaceCenter(), WorldSpaceCenter() + vecDirToEnemy, vecHullMin, vecHullMax, MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );

	if( tr.fraction != 1.0 )
	{
		// There's some sort of obstacle pretty much right in front of me.
		return COND_NONE;
	}

	return COND_CAN_RANGE_ATTACK1;
}

//-----------------------------------------------------------------------------
// Purpose:
//
//
//-----------------------------------------------------------------------------
void CHound::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_HOUND_LEAP )
	{
		LeapAttack();
		return;
	}
	
	if ( pEvent->event == AE_HOUND_GALLOP_LEFT )
	{
		EmitSound( "NPC_Hound.GallopLeft" );
		return;
	}

	if ( pEvent->event == AE_HOUND_GALLOP_RIGHT )
	{
		EmitSound( "NPC_Hound.GallopRight" );
		return;
	}
	
	if ( pEvent->event == AE_ZOMBIE_ATTACK_RIGHT )
	{
		Vector right;
		AngleVectors( GetLocalAngles(), NULL, &right, NULL );
		right = right * -50;

		QAngle angle( -3, -5, -3  );
		ClawAttack(GetClawAttackRange(), sk_hound_dmg_bite.GetInt(), angle, right, ZOMBIE_BLOOD_BITE); //ZOMBIE_BLOOD_RIGHT_HAND
		return;
	}

	if ( pEvent->event == AE_ZOMBIE_ATTACK_LEFT )
	{
		Vector right;
		AngleVectors( GetLocalAngles(), NULL, &right, NULL );
		right = right * 50;
		QAngle angle( -3, 5, -3 );
		ClawAttack(GetClawAttackRange(), sk_hound_dmg_bite.GetInt(), angle, right, ZOMBIE_BLOOD_BITE); //ZOMBIE_BLOOD_LEFT_HAND
		return;
	}

//=============================================================================
#ifdef HL2_EPISODIC

	// Do the leap attack
	if ( pEvent->event == AE_HOUND_VEHICLE_LEAP )
	{
		VehicleLeapAttack();
		return;
	}

	// Die while doing an SS in a vehicle
	if ( pEvent->event == AE_HOUND_VEHICLE_SS_DIE )
	{
		if ( IsInAVehicle() )
		{
			// Get the vehicle's present speed as a baseline
			Vector vecVelocity = vec3_origin;
			CBaseEntity *pVehicle = m_PassengerBehavior.GetTargetVehicle();
			if ( pVehicle )
			{
				pVehicle->GetVelocity( &vecVelocity, NULL );
			}

			// TODO: We need to make this content driven -- jdw
			Vector vecForward, vecRight, vecUp;
			GetVectors( &vecForward, &vecRight, &vecUp );

			vecVelocity += ( vecForward * -2500.0f ) + ( vecRight * 200.0f ) + ( vecUp * 300 );
			
			// Always kill
			float flDamage = GetMaxHealth() + 10;

			// Take the damage and die
			CTakeDamageInfo info( this, this, vecVelocity * 25.0f, WorldSpaceCenter(), flDamage, (DMG_CRUSH|DMG_VEHICLE) );
			TakeDamage( info );
		}
		return;
	}

#endif // HL2_EPISODIC
//=============================================================================

	BaseClass::HandleAnimEvent( pEvent );
}


//-----------------------------------------------------------------------------
// Purpose: Jump at the enemy!! (stole this from the headcrab)
//
//
//-----------------------------------------------------------------------------
void CHound::LeapAttack( void )
{
	SetGroundEntity( NULL );

	BeginAttackJump();

	//
	// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
	//
	UTIL_SetOrigin( this, GetLocalOrigin() + Vector( 0 , 0 , 1 ));

	Vector vecJumpDir;
	CBaseEntity *pEnemy = GetEnemy();

	if ( pEnemy )
	{
		Vector vecEnemyPos = pEnemy->WorldSpaceCenter();

		float gravity = GetCurrentGravity();
		if ( gravity <= 1 )
		{
			gravity = 1;
		}

		//
		// How fast does the zombie need to travel to reach my enemy's eyes given gravity?
		//
		float height = ( vecEnemyPos.z - GetAbsOrigin().z );

		if ( height < 16 )
		{
			height = 16;
		}
		else if ( height > 120 )
		{
			height = 120;
		}
		float speed = sqrt( 2 * gravity * height );
		float time = speed / gravity;

		//
		// Scale the sideways velocity to get there at the right time
		//
		vecJumpDir = vecEnemyPos - GetAbsOrigin();
		vecJumpDir = vecJumpDir / time;

		//
		// Speed to offset gravity at the desired height.
		//
		vecJumpDir.z = speed;

		//
		// Don't jump too far/fast.
		//
#define CLAMP 1000.0
		float distance = vecJumpDir.Length();
		if ( distance > CLAMP )
		{
			vecJumpDir = vecJumpDir * ( CLAMP / distance );
		}

		// try speeding up a bit.
		SetAbsVelocity( vecJumpDir );
		m_flNextAttack = gpGlobals->curtime + 2;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHound::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_HOUND_VERIFY_ATTACK:
		// Simply ensure that the zombie still has a valid melee attack
		if( HasCondition( COND_CAN_MELEE_ATTACK1 ) )
		{
			TaskComplete();
		}
		else
		{
			TaskFail("");
		}
		break;

	case TASK_HOUND_JUMP_BACK:
		{
			SetActivity( ACT_IDLE );

			SetGroundEntity( NULL );

			BeginAttackJump();

			Vector forward;
			AngleVectors( GetLocalAngles(), &forward );

			//
			// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
			//
			UTIL_SetOrigin( this, GetLocalOrigin() + Vector( 0 , 0 , 1 ));

			ApplyAbsVelocityImpulse( forward * -200 + Vector( 0, 0, 200 ) );
		}
		break;

	case TASK_HOUND_UNSTICK_JUMP:
		{
			SetGroundEntity( NULL );

			// Call begin attack jump. A little bit later if we fail to pathfind, we check
			// this value to see if we just jumped. If so, we assume we've jumped 
			// to someplace that's not pathing friendly, and so must jump again to get out.
			BeginAttackJump();

			//
			// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
			//
			UTIL_SetOrigin( this, GetLocalOrigin() + Vector( 0 , 0 , 1 ));

			CBaseEntity *pEnemy = GetEnemy();
			Vector vecJumpDir;

			if ( GetActivity() == ACT_CLIMB_UP || GetActivity() == ACT_CLIMB_DOWN )
			{
				// Jump off the pipe backwards!
				Vector forward;

				GetVectors( &forward, NULL, NULL );

				ApplyAbsVelocityImpulse( forward * -200 );
			}
			else if( pEnemy )
			{
				vecJumpDir = pEnemy->GetLocalOrigin() - GetLocalOrigin();
				VectorNormalize( vecJumpDir );
				vecJumpDir.z = 0;

				ApplyAbsVelocityImpulse( vecJumpDir * 300 + Vector( 0, 0, 200 ) );
			}
			else
			{
				DevMsg("UNHANDLED CASE! Stuck Hound with no enemy!\n");
			}
		}
		break;

	case TASK_WAIT_FOR_MOVEMENT:
		// If we're waiting for movement, that means that pathfinding succeeded, and
		// we're about to be moving. So we aren't stuck. So clear this flag. 
		m_fJustJumped = false;

		BaseClass::StartTask( pTask );
		break;

	case TASK_FACE_ENEMY:
		{
			// We don't use the base class implementation of this, because GetTurnActivity
			// stomps our landing scrabble animations (sjb)
			Vector flEnemyLKP = GetEnemyLKP();
			GetMotor()->SetIdealYawToTarget( flEnemyLKP );
		}
		break;

	case TASK_HOUND_LAND_RECOVER:
		{
			// Set the ideal yaw
			Vector flEnemyLKP = GetEnemyLKP();
			GetMotor()->SetIdealYawToTarget( flEnemyLKP );

			// figure out which way to turn.
			float flDeltaYaw = GetMotor()->DeltaIdealYaw();

			if( flDeltaYaw < 0 )
			{
				SetIdealActivity( (Activity)ACT_HOUND_LAND_RIGHT );
			}
			else
			{
				SetIdealActivity( (Activity)ACT_HOUND_LAND_LEFT );
			}


			TaskComplete();
		}
		break;

	case TASK_RANGE_ATTACK1:

		// Make melee attacks impossible until we land!
		m_flNextMeleeAttack = gpGlobals->curtime + 60;

		SetTouch( &CHound::LeapAttackTouch );
		break;

	case TASK_HOUND_DO_ATTACK:
		SetActivity( (Activity)ACT_HOUND_LEAP_SOAR );
		break;

	case TASK_HOUND_FACE_BUGBAIT:

		//Must have a saved sound
		//FIXME: This isn't assured to be still pointing to the right place, need to protect this
		if (!m_bHasHeardSound)
		{
			TaskFail("No remembered bug bait sound to run to!");
			return;
		}

		GetMotor()->SetIdealYawToTargetAndUpdate(m_vecHeardSound);
		SetTurnActivity();

		break;

	case TASK_HOUND_GET_PATH_TO_BUGBAIT:
	{
		//Must have a saved sound
		//FIXME: This isn't assured to be still pointing to the right place, need to protect this
		if (!m_bHasHeardSound)
		{
			TaskFail("No remembered bug bait sound to run to!");
			return;
		}

		Vector	goalPos;

		// Find the position to chase to
		if (FindChasePosition(m_vecHeardSound, goalPos))
		{
			AI_NavGoal_t goal(goalPos, (Activity)ACT_RUN/*ACT_HOUND_RUN_AGITATED*/, HOUND_BUGBAIT_NAV_TOLERANCE);
			//Try to run directly there
			if (GetNavigator()->SetGoal(goal, AIN_DISCARD_IF_FAIL) == false)
			{
				//Try and get as close as possible otherwise
				AI_NavGoal_t nearGoal(GOALTYPE_LOCATION_NEAREST_NODE, goalPos, (Activity)ACT_RUN/*ACT_HOUND_RUN_AGITATED*/, HOUND_BUGBAIT_NAV_TOLERANCE);

				if (GetNavigator()->SetGoal(nearGoal, AIN_CLEAR_PREVIOUS_STATE))
				{
					//FIXME: HACK! The internal pathfinding is setting this without our consent, so override it!
					ClearCondition(COND_TASK_FAILED);

					LockJumpNode();
					TaskComplete();
					return;
				}
				else
				{
					TaskFail("Hound failed to find path to bugbait position\n");
					return;
				}
			}
			else
			{
				LockJumpNode();
				TaskComplete();
				return;
			}
		}

		TaskFail("Hound failed to find path to bugbait position\n");
		break;
	}

	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHound::RunTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_HOUND_JUMP_BACK:
	case TASK_HOUND_UNSTICK_JUMP:
		if( GetFlags() & FL_ONGROUND )
		{
			TaskComplete();
		}
		break;

	case TASK_RANGE_ATTACK1:
		if( ( GetFlags() & FL_ONGROUND ) || ( m_pfnTouch == NULL ) )
		{
			// All done when you touch the ground, or if our touch function has somehow cleared.
			TaskComplete();

			// Allow melee attacks again.
			m_flNextMeleeAttack = gpGlobals->curtime + 0.5;
			return;
		}
		break;

	default:
		BaseClass::RunTask( pTask );
		break;
	}
}


//---------------------------------------------------------
//---------------------------------------------------------
int CHound::TranslateSchedule( int scheduleType )
{
	switch( scheduleType )
	{
	case SCHED_RANGE_ATTACK1:
		{
			// Scream right now, cause in half a second, we're gonna jump!!
	
			if( !m_fHasScreamed )
			{
				// Only play that over-the-top attack scream once per combat state.
				EmitSound( "NPC_Hound.Scream" );
				m_fHasScreamed = true;
			}
			else
			{
				EmitSound( "NPC_Hound.RangeAttack" );
			}

			return SCHED_HOUND_RANGE_ATTACK1;
		}
		break;

	case SCHED_MELEE_ATTACK1:
		return SCHED_HOUND_MELEE_ATTACK1;
		break;

	case SCHED_HOUND_UNSTICK_JUMP:
		return SCHED_HOUND_UNSTICK_JUMP;
		break;
	case SCHED_MOVE_TO_WEAPON_RANGE:
		{
			float flZDist = fabs( GetEnemy()->GetLocalOrigin().z - GetLocalOrigin().z );
			if ( flZDist > HOUND_MAXLEAP_Z )
				return SCHED_CHASE_ENEMY;
			else // fall through to default
				return BaseClass::TranslateSchedule( scheduleType );
			break;
		}

	default:
		return BaseClass::TranslateSchedule( scheduleType );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CHound::LeapAttackTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsSolid() )
	{
		// Touching a trigger or something.
		return;
	}

	// Stop the zombie and knock the player back
	Vector vecNewVelocity( 0, 0, GetAbsVelocity().z );
	SetAbsVelocity( vecNewVelocity );

	Vector forward;
	AngleVectors( GetLocalAngles(), &forward );
	forward *= 500;
	QAngle qaPunch( 15, random->RandomInt(-5,5), random->RandomInt(-5,5) );
	
	ClawAttack( GetClawAttackRange(), sk_hound_dmg_leap.GetInt(), qaPunch, forward, ZOMBIE_BLOOD_BITE ); //ZOMBIE_BLOOD_BOTH_HANDS

	SetTouch( NULL );
}

//-----------------------------------------------------------------------------
// Purpose: Shuts down our looping sounds.
//-----------------------------------------------------------------------------
void CHound::StopLoopingSounds( void )
{
	if ( m_pMoanSound )
	{
		ENVELOPE_CONTROLLER.SoundDestroy( m_pMoanSound );
		m_pMoanSound = NULL;
	}

	if ( m_pLayer2 )
	{
		ENVELOPE_CONTROLLER.SoundDestroy( m_pLayer2 );
		m_pLayer2 = NULL;
	}

	BaseClass::StopLoopingSounds();
}


//-----------------------------------------------------------------------------
// Purpose: Fast zombie cannot range attack when he's a torso!
//-----------------------------------------------------------------------------
void CHound::BecomeTorso( const Vector &vecTorsoForce, const Vector &vecLegsForce )
{ }

//-----------------------------------------------------------------------------
// Purpose: Returns true if a reasonable jumping distance
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CHound::IsJumpLegal(const Vector &startPos, const Vector &apex, const Vector &endPos) const
{
	const float MAX_JUMP_RISE		= 220.0f;
	const float MAX_JUMP_DISTANCE	= 512.0f;
	const float MAX_JUMP_DROP		= 384.0f;

	if ( BaseClass::IsJumpLegal( startPos, apex, endPos, MAX_JUMP_RISE, MAX_JUMP_DROP, MAX_JUMP_DISTANCE ) )
	{
		// Hang onto the jump distance. The AI is going to want it.
		m_flJumpDist = (startPos - endPos).Length();

		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool CHound::MovementCost( int moveType, const Vector &vecStart, const Vector &vecEnd, float *pCost )
{
	float delta = vecEnd.z - vecStart.z;

	float multiplier = 1;
	if ( moveType == bits_CAP_MOVE_JUMP )
	{
		multiplier = ( delta < 0 ) ? 0.5 : 1.5;
	}

	*pCost *= multiplier;

	return ( multiplier != 1 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool CHound::ShouldFailNav( bool bMovementFailed )
{
	if ( !BaseClass::ShouldFailNav( bMovementFailed ) )
	{
		DevMsg( 2, "Hound in scripted sequence probably hit bad node configuration at %s\n", VecToString( GetAbsOrigin() ) );
		
		if ( GetNavigator()->GetPath()->CurWaypointNavType() == NAV_JUMP && GetNavigator()->RefindPathToGoal( false ) )
		{
			return false;
		}
		DevMsg( 2, "Hound failed to get to scripted sequence\n" );
	}

	return true;
}


//---------------------------------------------------------
// Purpose: Notifier that lets us know when the hound
//			has hit the apex of a navigational jump.
//---------------------------------------------------------
void CHound::OnNavJumpHitApex( void )
{
	m_fHitApex = true;	// stop subsequent notifications
}

//---------------------------------------------------------
// Purpose: Overridden to detect when the zombie goes into
//			and out of his climb state and his navigation
//			jump state.
//---------------------------------------------------------
void CHound::OnChangeActivity( Activity NewActivity )
{
	if ( NewActivity == ACT_HOUND_FRENZY )
	{
		// Scream!!!!
		EmitSound( "NPC_Hound.Frenzy" );
		SetPlaybackRate( random->RandomFloat( .9, 1.1 ) );	
	}

	if( NewActivity == ACT_JUMP )
	{
		BeginNavJump();
	}
	else if( GetActivity() == ACT_JUMP )
	{
		EndNavJump();
	}

	if ( NewActivity == ACT_LAND )
	{
		m_flNextAttack = gpGlobals->curtime + 1.0;
	}

	if ( NewActivity == ACT_GLIDE )
	{
		// Started a jump.
		BeginNavJump();
	}
	else if ( GetActivity() == ACT_GLIDE )
	{
		// Landed a jump
		EndNavJump();

		if ( m_pMoanSound )
			ENVELOPE_CONTROLLER.SoundChangePitch( m_pMoanSound, HOUND_MIN_PITCH, 0.3 );
	}
	BaseClass::OnChangeActivity( NewActivity );
}


//=========================================================
// 
//=========================================================
int CHound::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{
	if ( m_fJustJumped )
	{
		// Assume we failed cause we jumped to a bad place.
		m_fJustJumped = false;
		return SCHED_HOUND_UNSTICK_JUMP;
	}

	return BaseClass::SelectFailSchedule( failedSchedule, failedTask, taskFailCode );
}

//=========================================================
// Purpose: Do some record keeping for jumps made for 
//			navigational purposes (i.e., not attack jumps)
//=========================================================
void CHound::BeginNavJump( void )
{
	m_fIsNavJumping = true;
	m_fHitApex = false;

	ENVELOPE_CONTROLLER.SoundPlayEnvelope( m_pLayer2, SOUNDCTRL_CHANGE_VOLUME, envHoundVolumeJump, ARRAYSIZE(envHoundVolumeJump) );
}

//=========================================================
// 
//=========================================================
void CHound::EndNavJump( void )
{
	m_fIsNavJumping = false;
	m_fHitApex = false;
}

//=========================================================
// 
//=========================================================
void CHound::BeginAttackJump( void )
{
	// Set this to true. A little bit later if we fail to pathfind, we check
	// this value to see if we just jumped. If so, we assume we've jumped 
	// to someplace that's not pathing friendly, and so must jump again to get out.
	m_fJustJumped = true;

	m_flJumpStartAltitude = GetLocalOrigin().z;
	LeapAttackSound();
}

//=========================================================
// 
//=========================================================
void CHound::EndAttackJump( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHound::BuildScheduleTestBits( void )
{
	// FIXME: This is probably the desired call to make, but it opts into an untested base class path, we'll need to
	//		  revisit this and figure out if we want that. -- jdw
	// BaseClass::BuildScheduleTestBits();
	//
	// For now, make sure our active behavior gets a chance to add its own bits
	if ( GetRunningBehavior() )
		GetRunningBehavior()->BridgeBuildScheduleTestBits(); 
}

//=========================================================
// 
//=========================================================
void CHound::OnStateChange( NPC_STATE OldState, NPC_STATE NewState )
{
	if( NewState == NPC_STATE_COMBAT )
	{
		SetAngrySoundState();
	}
	else if( (m_pMoanSound) && ( NewState == NPC_STATE_IDLE || NewState == NPC_STATE_ALERT ) ) ///!!!HACKHACK - sjb
	{
		// Don't make this sound while we're slumped
		if ( IsSlumped() == false )
		{
			// Set it up so that if the zombie goes into combat state sometime down the road
			// that he'll be able to scream.
			m_fHasScreamed = false;

			SetIdleSoundState();
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHound::Event_Killed( const CTakeDamageInfo &info )
{
	// Shut up my screaming sounds.
	CPASAttenuationFilter filter( this );
	EmitSound( filter, entindex(), "NPC_Hound.NoSound" );

	CTakeDamageInfo dInfo = info;

	if (info.GetDamageType() & DMG_VEHICLE)
	{
		Vector vecDamageDir = info.GetDamageForce();
		VectorNormalize(vecDamageDir);

		// Big blood splat
		UTIL_BloodSpray(WorldSpaceCenter(), vecDamageDir, BLOOD_COLOR_RED, 8, FX_BLOODSPRAY_CLOUD);
	}

	CAI_BaseNPC::Event_Killed(info);
}

//-----------------------------------------------------------------------------
// Never become a torso
//-----------------------------------------------------------------------------
bool CHound::ShouldBecomeTorso( const CTakeDamageInfo &info, float flDamageThreshold )
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Returns this monster's place in the relationship table.
//-----------------------------------------------------------------------------
Class_T	CHound::Classify( void )
{
	if ( IsSlumped() )
		return CLASS_NONE;

	return( CLASS_CITIZEN_REBEL ); //CLASS_ZOMBIE
}

//-----------------------------------------------------------------------------
// Never release a headcrab!
//-----------------------------------------------------------------------------
HeadcrabRelease_t CHound::ShouldReleaseHeadcrab(const CTakeDamageInfo &info, float flDamageThreshold)
{
	return RELEASE_NO;
}


//BUGBAIT
// Number of times the hounds will attempt to generate a random chase position
#define NUM_CHASE_POSITION_ATTEMPTS		3

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &targetPos - 
//			&result - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHound::FindChasePosition(const Vector &targetPos, Vector &result)
{
	/*if (HasSpawnFlags(SF_HOUND_USE_GROUNDCHECKS) == true)
	{
		result = targetPos;
		return true;
	}*/

	Vector runDir = (targetPos - GetAbsOrigin());
	VectorNormalize(runDir);

	Vector	vRight, vUp;
	VectorVectors(runDir, vRight, vUp);

	for (int i = 0; i < NUM_CHASE_POSITION_ATTEMPTS; i++)
	{
		result = targetPos;
		result += -runDir * random->RandomInt(64, 128);
		result += vRight * random->RandomInt(-128, 128);

		//FIXME: We need to do a more robust search here
		// Find a ground position and try to get there
		if (GetGroundPosition(result, result))
			return true;
	}
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &testPos - 
//-----------------------------------------------------------------------------
bool CHound::GetGroundPosition(const Vector &testPos, Vector &result)
{
	// Trace up to clear the ground
	trace_t	tr;
	AI_TraceHull(testPos, testPos + Vector(0, 0, 64), NAI_Hull::Mins(GetHullType()), NAI_Hull::Maxs(GetHullType()), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);

	// If we're stuck in solid, this can't be valid
	if (tr.allsolid)
		return false;

	// Trace down to find the ground
	AI_TraceHull(tr.endpos, tr.endpos - Vector(0, 0, 128), NAI_Hull::Mins(GetHullType()), NAI_Hull::Maxs(GetHullType()), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);

	// We must end up on the floor with this trace
	if (tr.fraction < 1.0f)
	{
		result = tr.endpos;
		return true;
	}

	// Ended up in open space
	return false;
}

void CHound::LockJumpNode(void)
{
	if (GetNavigator()->GetPath() == NULL)
		return;

	AI_Waypoint_t *pWaypoint = GetNavigator()->GetPath()->GetCurWaypoint();

	while (pWaypoint)
	{
		AI_Waypoint_t *pNextWaypoint = pWaypoint->GetNext();
		if (pNextWaypoint && pNextWaypoint->NavType() == NAV_JUMP && pWaypoint->iNodeID != NO_NODE)
		{
			CAI_Node *pNode = GetNavigator()->GetNetwork()->GetNode(pWaypoint->iNodeID);

			if (pNode)
			{
				//NDebugOverlay::Box( pNode->GetOrigin(), Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 255, 0, 0, 0, 2 );
				pNode->Lock(0.5f);
				break;
			}
		}
		else
		{
			pWaypoint = pWaypoint->GetNext();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEnemy - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHound::IsValidEnemy(CBaseEntity *pEnemy)
{
	if (pEnemy->IsWorld())
		return false;

	//If we're chasing bugbait, close to within a certain radius before picking up enemies
	if (IsCurSchedule(GetGlobalScheduleId(SCHED_HOUND_CHASE_BUGBAIT)) && (GetNavigator() != NULL))
	{
		//If the enemy is without the target radius, then don't allow them
		if ((GetNavigator()->IsGoalActive()) && (GetNavigator()->GetGoalPos() - pEnemy->GetAbsOrigin()).Length() > bugbait_radius.GetFloat())
			return false;
	}
	return BaseClass::IsValidEnemy(pEnemy);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSound - 
//-----------------------------------------------------------------------------
bool CHound::QueryHearSound(CSound *pSound)
{
	if (!BaseClass::QueryHearSound(pSound))
		return false;

	if (pSound->m_iType == SOUND_BUGBAIT)
	{
		//Must be more recent than the current
		if (pSound->SoundExpirationTime() <= m_flIgnoreSoundTime)
			return false;

		//If we can hear it, store it
		m_bHasHeardSound = (pSound != NULL);
		if (m_bHasHeardSound)
		{
			DevMsg("Hound heard bugbait splat!\n");
			m_vecHeardSound = pSound->GetSoundOrigin();
			m_flIgnoreSoundTime = pSound->SoundExpirationTime();
		}
	}

	//Do the normal behavior at this point
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHound::CreateBehaviors(void)
{
	//AddBehavior(&m_FollowBehavior);
	AddBehavior(&m_AssaultBehavior);

	return BaseClass::CreateBehaviors();
}

//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_hound, CHound )

	DECLARE_ACTIVITY( ACT_HOUND_LEAP_SOAR )
	DECLARE_ACTIVITY( ACT_HOUND_LEAP_STRIKE )
	DECLARE_ACTIVITY( ACT_HOUND_LAND_RIGHT )
	DECLARE_ACTIVITY( ACT_HOUND_LAND_LEFT )
	DECLARE_ACTIVITY( ACT_HOUND_FRENZY )
	DECLARE_ACTIVITY( ACT_HOUND_BIG_SLASH )
	
	DECLARE_TASK( TASK_HOUND_DO_ATTACK )
	DECLARE_TASK( TASK_HOUND_LAND_RECOVER )
	DECLARE_TASK( TASK_HOUND_UNSTICK_JUMP )
	DECLARE_TASK( TASK_HOUND_JUMP_BACK )
	DECLARE_TASK( TASK_HOUND_VERIFY_ATTACK )
	DECLARE_TASK(TASK_HOUND_GET_PATH_TO_BUGBAIT)
	DECLARE_TASK(TASK_HOUND_FACE_BUGBAIT)

	//Adrian: events go here
	DECLARE_ANIMEVENT( AE_HOUND_LEAP )
	DECLARE_ANIMEVENT( AE_HOUND_GALLOP_LEFT )
	DECLARE_ANIMEVENT( AE_HOUND_GALLOP_RIGHT )

	//=========================================================
	// 
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_HOUND_RANGE_ATTACK1,

		"	Tasks"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_RANGE_ATTACK1"
		"		TASK_SET_ACTIVITY				ACTIVITY:ACT_HOUND_LEAP_STRIKE"
		"		TASK_RANGE_ATTACK1				0"
		"		TASK_WAIT						0.1"
		"		TASK_HOUND_LAND_RECOVER	0" // essentially just figure out which way to turn.
		"		TASK_FACE_ENEMY					0"
		"	"
		"	Interrupts"
	)

	//=========================================================
	// I have landed somewhere that's pathfinding-unfriendly
	// just try to jump out.
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_HOUND_UNSTICK_JUMP,

		"	Tasks"
		"		TASK_HOUND_UNSTICK_JUMP	0"
		"	"
		"	Interrupts"
	)

	//=========================================================
	// > Melee_Attack1
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_HOUND_MELEE_ATTACK1,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_FACE_ENEMY					0"
		"		TASK_MELEE_ATTACK1				0"
		"		TASK_MELEE_ATTACK1				0"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_HOUND_FRENZY"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CHASE_ENEMY"
		"		TASK_HOUND_VERIFY_ATTACK	0"
		"		TASK_PLAY_SEQUENCE_FACE_ENEMY	ACTIVITY:ACT_HOUND_BIG_SLASH"

		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_OCCLUDED"
	)

	//==================================================
	// SCHED_HOUND_CHASE_BUGBAIT
	//==================================================
	DEFINE_SCHEDULE
		(
		SCHED_HOUND_CHASE_BUGBAIT,

		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_HOUND_GET_PATH_TO_BUGBAIT	0"
		"		TASK_RUN_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_STOP_MOVING					0"
		"		TASK_HOUND_FACE_BUGBAIT			0"
		""
		"	Interrupts"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_SEE_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		);


AI_END_CUSTOM_NPC()


