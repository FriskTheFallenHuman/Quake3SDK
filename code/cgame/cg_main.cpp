/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"

#ifdef MISSIONPACK
#include "../ui/ui_shared.h"
// display context for new ui stuff
displayContextDef_t cgDC;
#endif

int forceModelModificationCount = -1;

void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown( void );

cg_t				cg;
cgs_t				cgs;
centity_t			cg_entities[MAX_GENTITIES];
weaponInfo_t		cg_weapons[MAX_WEAPONS];
itemInfo_t			cg_items[MAX_ITEMS];


vmCvar_t	cg_railTrailTime;
vmCvar_t	cg_centertime;
vmCvar_t	cg_runpitch;
vmCvar_t	cg_runroll;
vmCvar_t	cg_bobup;
vmCvar_t	cg_bobpitch;
vmCvar_t	cg_bobroll;
vmCvar_t	cg_swingSpeed;
vmCvar_t	cg_shadows;
vmCvar_t	cg_gibs;
vmCvar_t	cg_drawTimer;
vmCvar_t	cg_drawFPS;
vmCvar_t	cg_drawSnapshot;
vmCvar_t	cg_draw3dIcons;
vmCvar_t	cg_drawIcons;
vmCvar_t	cg_drawAmmoWarning;
vmCvar_t	cg_drawCrosshair;
vmCvar_t	cg_drawCrosshairNames;
vmCvar_t	cg_drawRewards;
vmCvar_t	cg_crosshairSize;
vmCvar_t	cg_crosshairX;
vmCvar_t	cg_crosshairY;
vmCvar_t	cg_crosshairHealth;
vmCvar_t	cg_draw2D;
vmCvar_t	cg_drawStatus;
vmCvar_t	cg_animSpeed;
vmCvar_t	cg_debugAnim;
vmCvar_t	cg_debugPosition;
vmCvar_t	cg_debugEvents;
vmCvar_t	cg_errorDecay;
vmCvar_t	cg_nopredict;
vmCvar_t	cg_noPlayerAnims;
vmCvar_t	cg_showmiss;
vmCvar_t	cg_footsteps;
vmCvar_t	cg_addMarks;
vmCvar_t	cg_brassTime;
vmCvar_t	cg_viewsize;
vmCvar_t	cg_drawGun;
vmCvar_t	cg_gun_frame;
vmCvar_t	cg_gun_x;
vmCvar_t	cg_gun_y;
vmCvar_t	cg_gun_z;
vmCvar_t	cg_tracerChance;
vmCvar_t	cg_tracerWidth;
vmCvar_t	cg_tracerLength;
vmCvar_t	cg_autoswitch;
vmCvar_t	cg_ignore;
vmCvar_t	cg_simpleItems;
vmCvar_t	cg_fov;
vmCvar_t	cg_zoomFov;
vmCvar_t	cg_thirdPerson;
vmCvar_t	cg_thirdPersonRange;
vmCvar_t	cg_thirdPersonAngle;
vmCvar_t	cg_stereoSeparation;
vmCvar_t	cg_lagometer;
vmCvar_t	cg_drawAttacker;
vmCvar_t	cg_synchronousClients;
vmCvar_t 	cg_teamChatTime;
vmCvar_t 	cg_teamChatHeight;
vmCvar_t 	cg_stats;
vmCvar_t 	cg_buildScript;
vmCvar_t 	cg_forceModel;
vmCvar_t	cg_paused;
vmCvar_t	cg_blood;
vmCvar_t	cg_predictItems;
vmCvar_t	cg_deferPlayers;
vmCvar_t	cg_drawTeamOverlay;
vmCvar_t	cg_teamOverlayUserinfo;
vmCvar_t	cg_drawFriend;
vmCvar_t	cg_teamChatsOnly;
vmCvar_t	cg_noVoiceChats;
vmCvar_t	cg_noVoiceText;
vmCvar_t	cg_hudFiles;
vmCvar_t 	cg_scorePlum;
vmCvar_t 	cg_smoothClients;
vmCvar_t	pmove_fixed;
//vmCvar_t	cg_pmove_fixed;
vmCvar_t	pmove_msec;
vmCvar_t	cg_pmove_msec;
vmCvar_t	cg_cameraMode;
vmCvar_t	cg_cameraOrbit;
vmCvar_t	cg_cameraOrbitDelay;
vmCvar_t	cg_timescaleFadeEnd;
vmCvar_t	cg_timescaleFadeSpeed;
vmCvar_t	cg_timescale;
vmCvar_t	cg_smallFont;
vmCvar_t	cg_bigFont;
vmCvar_t	cg_noTaunt;
vmCvar_t	cg_noProjectileTrail;
vmCvar_t	cg_oldRail;
vmCvar_t	cg_oldRocket;
vmCvar_t	cg_oldPlasma;
vmCvar_t	cg_trueLightning;

#ifdef MISSIONPACK
vmCvar_t 	cg_redTeamName;
vmCvar_t 	cg_blueTeamName;
vmCvar_t	cg_currentSelectedPlayer;
vmCvar_t	cg_currentSelectedPlayerName;
vmCvar_t	cg_enableDust;
vmCvar_t	cg_enableBreath;
vmCvar_t	cg_obeliskRespawnDelay;
#endif

typedef struct {
	vmCvar_t	* vmCvar;
	char	*	cvarName;
	char	*	defaultString;
	int			cvarFlags;
} cvarTable_t;

static cvarTable_t cvarTable[] = { // bk001129
	{ &cg_ignore, "cg_ignore", "0", 0 },	// used for debugging
	{ &cg_autoswitch, "cg_autoswitch", "1", CVAR_ARCHIVE },
	{ &cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE },
	{ &cg_zoomFov, "cg_zoomfov", "22.5", CVAR_ARCHIVE },
	{ &cg_fov, "cg_fov", "90", CVAR_ARCHIVE },
	{ &cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE },
	{ &cg_stereoSeparation, "cg_stereoSeparation", "0.4", CVAR_ARCHIVE  },
	{ &cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE  },
	{ &cg_gibs, "cg_gibs", "1", CVAR_ARCHIVE  },
	{ &cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE  },
	{ &cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE  },
	{ &cg_drawTimer, "cg_drawTimer", "0", CVAR_ARCHIVE  },
	{ &cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE  },
	{ &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE  },
	{ &cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawAmmoWarning, "cg_drawAmmoWarning", "1", CVAR_ARCHIVE  },
	{ &cg_drawAttacker, "cg_drawAttacker", "1", CVAR_ARCHIVE  },
	{ &cg_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE },
	{ &cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
	{ &cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE },
	{ &cg_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE },
	{ &cg_crosshairHealth, "cg_crosshairHealth", "1", CVAR_ARCHIVE },
	{ &cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE },
	{ &cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE },
	{ &cg_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE },
	{ &cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE },
	{ &cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE },
	{ &cg_lagometer, "cg_lagometer", "1", CVAR_ARCHIVE },
	{ &cg_railTrailTime, "cg_railTrailTime", "400", CVAR_ARCHIVE  },
	{ &cg_gun_x, "cg_gunX", "0", CVAR_CHEAT },
	{ &cg_gun_y, "cg_gunY", "0", CVAR_CHEAT },
	{ &cg_gun_z, "cg_gunZ", "0", CVAR_CHEAT },
	{ &cg_centertime, "cg_centertime", "3", CVAR_CHEAT },
	{ &cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE},
	{ &cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE },
	{ &cg_bobup, "cg_bobup", "0.005", CVAR_CHEAT },
	{ &cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE },
	{ &cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE },
	{ &cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT },
	{ &cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT },
	{ &cg_debugAnim, "cg_debuganim", "0", CVAR_CHEAT },
	{ &cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT },
	{ &cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT },
	{ &cg_errorDecay, "cg_errordecay", "100", 0 },
	{ &cg_nopredict, "cg_nopredict", "0", 0 },
	{ &cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT },
	{ &cg_showmiss, "cg_showmiss", "0", 0 },
	{ &cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT },
	{ &cg_tracerChance, "cg_tracerchance", "0.4", CVAR_CHEAT },
	{ &cg_tracerWidth, "cg_tracerwidth", "1", CVAR_CHEAT },
	{ &cg_tracerLength, "cg_tracerlength", "100", CVAR_CHEAT },
	{ &cg_thirdPersonRange, "cg_thirdPersonRange", "40", CVAR_CHEAT },
	{ &cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT },
	{ &cg_thirdPerson, "cg_thirdPerson", "0", 0 },
	{ &cg_teamChatTime, "cg_teamChatTime", "3000", CVAR_ARCHIVE  },
	{ &cg_teamChatHeight, "cg_teamChatHeight", "0", CVAR_ARCHIVE  },
	{ &cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE  },
	{ &cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE },
#ifdef MISSIONPACK
	{ &cg_deferPlayers, "cg_deferPlayers", "0", CVAR_ARCHIVE },
#else
	{ &cg_deferPlayers, "cg_deferPlayers", "1", CVAR_ARCHIVE },
#endif
	{ &cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE },
	{ &cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM | CVAR_USERINFO },
	{ &cg_stats, "cg_stats", "0", 0 },
	{ &cg_drawFriend, "cg_drawFriend", "1", CVAR_ARCHIVE },
	{ &cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE },
	{ &cg_noVoiceChats, "cg_noVoiceChats", "0", CVAR_ARCHIVE },
	{ &cg_noVoiceText, "cg_noVoiceText", "0", CVAR_ARCHIVE },
	// the following variables are created in other parts of the system,
	// but we also reference them here
	{ &cg_buildScript, "com_buildScript", "0", 0 },	// force loading of all possible data amd error on failures
	{ &cg_paused, "cl_paused", "0", CVAR_ROM },
	{ &cg_blood, "com_blood", "1", CVAR_ARCHIVE },
	{ &cg_synchronousClients, "g_synchronousClients", "0", 0 },	// communicated by systeminfo
#ifdef MISSIONPACK
	{ &cg_redTeamName, "g_redteam", DEFAULT_REDTEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO },
	{ &cg_blueTeamName, "g_blueteam", DEFAULT_BLUETEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO },
	{ &cg_currentSelectedPlayer, "cg_currentSelectedPlayer", "0", CVAR_ARCHIVE},
	{ &cg_currentSelectedPlayerName, "cg_currentSelectedPlayerName", "", CVAR_ARCHIVE},
	{ &cg_enableDust, "g_enableDust", "0", CVAR_SERVERINFO},
	{ &cg_enableBreath, "g_enableBreath", "0", CVAR_SERVERINFO},
	{ &cg_obeliskRespawnDelay, "g_obeliskRespawnDelay", "10", CVAR_SERVERINFO},
	{ &cg_hudFiles, "cg_hudFiles", "ui/hud.txt", CVAR_ARCHIVE},
#endif
	{ &cg_cameraOrbit, "cg_cameraOrbit", "0", CVAR_CHEAT},
	{ &cg_cameraOrbitDelay, "cg_cameraOrbitDelay", "50", CVAR_ARCHIVE},
	{ &cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0},
	{ &cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0},
	{ &cg_timescale, "timescale", "1", 0},
	{ &cg_scorePlum, "cg_scorePlums", "1", CVAR_USERINFO | CVAR_ARCHIVE},
	{ &cg_smoothClients, "cg_smoothClients", "0", CVAR_USERINFO | CVAR_ARCHIVE},
	{ &cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT},

	{ &pmove_fixed, "pmove_fixed", "0", 0},
	{ &pmove_msec, "pmove_msec", "8", 0},
	{ &cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE},
	{ &cg_noProjectileTrail, "cg_noProjectileTrail", "0", CVAR_ARCHIVE},
	{ &cg_smallFont, "ui_smallFont", "0.25", CVAR_ARCHIVE},
	{ &cg_bigFont, "ui_bigFont", "0.4", CVAR_ARCHIVE},
	{ &cg_oldRail, "cg_oldRail", "1", CVAR_ARCHIVE},
	{ &cg_oldRocket, "cg_oldRocket", "1", CVAR_ARCHIVE},
	{ &cg_oldPlasma, "cg_oldPlasma", "1", CVAR_ARCHIVE},
	{ &cg_trueLightning, "cg_trueLightning", "0.0", CVAR_ARCHIVE}
//	{ &cg_pmove_fixed, "cg_pmove_fixed", "0", CVAR_USERINFO | CVAR_ARCHIVE }
};

static int  cvarTableSize = sizeof( cvarTable ) / sizeof( cvarTable[0] );

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
	int			i;
	cvarTable_t	* cv;
	char		var[MAX_TOKEN_CHARS];

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		cgameLocal->Cvar_Register( cv->vmCvar, cv->cvarName,
							cv->defaultString, cv->cvarFlags );
	}

	// see if we are also running the server on this machine
	cgameLocal->Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
	cgs.localServer = atoi( var ) > 0;

	forceModelModificationCount = cg_forceModel.modificationCount;

	cgameLocal->Cvar_Register( NULL, "model", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
	cgameLocal->Cvar_Register( NULL, "headmodel", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
	cgameLocal->Cvar_Register( NULL, "team_model", DEFAULT_TEAM_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
	cgameLocal->Cvar_Register( NULL, "team_headmodel", DEFAULT_TEAM_HEAD, CVAR_USERINFO | CVAR_ARCHIVE );
}

/*
===================
CG_ForceModelChange
===================
*/
static void CG_ForceModelChange( void ) {
	int		i;

	for ( i = 0 ; i < MAX_CLIENTS ; i++ ) {
		const char	*	clientInfo;

		clientInfo = CG_ConfigString( CS_PLAYERS + i );
		if ( !clientInfo[0] ) {
			continue;
		}
		CG_NewClientInfo( i );
	}
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void ) {
	int			i;
	cvarTable_t	* cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		cgameLocal->Cvar_Update( cv->vmCvar );
	}

	// check for modications here

	// If team overlay is on, ask for updates from the server.  If its off,
	// let the server know so we don't receive it
	if ( drawTeamOverlayModificationCount != cg_drawTeamOverlay.modificationCount ) {
		drawTeamOverlayModificationCount = cg_drawTeamOverlay.modificationCount;

		if ( cg_drawTeamOverlay.integer > 0 ) {
			cgameLocal->Cvar_Set( "teamoverlay", "1" );
		} else {
			cgameLocal->Cvar_Set( "teamoverlay", "0" );
		}
		// FIXME E3 HACK
		cgameLocal->Cvar_Set( "teamoverlay", "1" );
	}

	// if force model changed
	if ( forceModelModificationCount != cg_forceModel.modificationCount ) {
		forceModelModificationCount = cg_forceModel.modificationCount;
		CG_ForceModelChange();
	}
}

int CG_CrosshairPlayer( void ) {
	if ( cg.time > ( cg.crosshairClientTime + 1000 ) ) {
		return -1;
	}
	return cg.crosshairClientNum;
}

int CG_LastAttacker( void ) {
	if ( !cg.attackerTime ) {
		return -1;
	}
	return cg.snap->ps.persistant[PERS_ATTACKER];
}

void QDECL CG_Printf( const char * msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start ( argptr, msg );
	Q_vsnprintf ( text, sizeof( text ), msg, argptr );
	va_end ( argptr );

	cgameLocal->Print( text );
}

void QDECL CG_Warning( const char * msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start ( argptr, msg );
	Q_vsnprintf ( text, sizeof( text ), msg, argptr );
	va_end ( argptr );

	cgameLocal->Warning( text );
}

void QDECL CG_Error( const char * msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start ( argptr, msg );
	Q_vsnprintf ( text, sizeof( text ), msg, argptr );
	va_end ( argptr );
	text[sizeof( text ) - 1] = 0;

	cgameLocal->Error( ERR_DROP, text );
}

#ifndef CGAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link (FIXME)

void QDECL Com_Error( int level, const char * error, ... ) {
	va_list		argptr;
	char		msg[1024];

	va_start ( argptr, error );
	Q_vsnprintf ( msg, sizeof( msg ), error, argptr );
	va_end ( argptr );
	msg[sizeof( msg ) - 1] = 0;

	CG_Error( S_COLOR_RED "ERROR: %s\n", msg );
}

void QDECL Com_Warning( const char * fmt, ... ) {
	va_list		argptr;
	char		msg[1024];

	va_start ( argptr, fmt );
	Q_vsnprintf ( msg, sizeof( msg ), fmt, argptr );
	va_end ( argptr );
	msg[sizeof( msg ) -1] = 0;

	CG_Warning( S_COLOR_YELLOW "WARNING: " S_COLOR_RED "%s\n", msg );
}

void QDECL Com_Printf( const char * fmt, ... ) {
	va_list		argptr;
	char		msg[1024];

	va_start ( argptr, fmt );
	Q_vsnprintf ( msg, sizeof( msg ), fmt, argptr );
	va_end ( argptr );
	msg[sizeof( msg ) -1] = 0;

	CG_Printf ( "%s", fmt );
}

#endif

/*
================
CG_Argv
================
*/
const char * CG_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	cgameLocal->Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


//========================================================================

/*
=================
CG_RegisterItemSounds

The server says this item is used on this level
=================
*/
static void CG_RegisterItemSounds( int itemNum ) {
	gitem_t		*	item;
	char			data[MAX_QPATH];
	char		*	s, * start;
	int				len;

	item = &bg_itemlist[ itemNum ];

	if ( item->pickup_sound ) {
		cgameLocal->S_RegisterSound( item->pickup_sound, false );
	}

	// parse the space seperated precache string for other media
	s = item->sounds;
	if ( !s || !s[0] ) {
		return;
	}

	while ( *s ) {
		start = s;
		while ( *s && *s != ' ' ) {
			s++;
		}

		len = s - start;
		if ( len >= MAX_QPATH || len < 5 ) {
			CG_Error( "PrecacheItem: %s has bad precache string",
					  item->classname );
			return;
		}
		memcpy ( data, start, len );
		data[len] = 0;
		if ( *s ) {
			s++;
		}

		if ( !strcmp( data + len - 3, "wav" ) ) {
			cgameLocal->S_RegisterSound( data, false );
		}
	}
}


/*
=================
CG_RegisterSounds

called during a precache command
=================
*/
static void CG_RegisterSounds( void ) {
	int		i;
	char	items[MAX_ITEMS + 1];
	char	name[MAX_QPATH];
	const char	* soundName;

	// voice commands
#ifdef MISSIONPACK
	CG_LoadVoiceChats();
#endif

	cgs.media.oneMinuteSound = cgameLocal->S_RegisterSound( "sound/feedback/1_minute.wav", true );
	cgs.media.fiveMinuteSound = cgameLocal->S_RegisterSound( "sound/feedback/5_minute.wav", true );
	cgs.media.suddenDeathSound = cgameLocal->S_RegisterSound( "sound/feedback/sudden_death.wav", true );
	cgs.media.oneFragSound = cgameLocal->S_RegisterSound( "sound/feedback/1_frag.wav", true );
	cgs.media.twoFragSound = cgameLocal->S_RegisterSound( "sound/feedback/2_frags.wav", true );
	cgs.media.threeFragSound = cgameLocal->S_RegisterSound( "sound/feedback/3_frags.wav", true );
	cgs.media.count3Sound = cgameLocal->S_RegisterSound( "sound/feedback/three.wav", true );
	cgs.media.count2Sound = cgameLocal->S_RegisterSound( "sound/feedback/two.wav", true );
	cgs.media.count1Sound = cgameLocal->S_RegisterSound( "sound/feedback/one.wav", true );
	cgs.media.countFightSound = cgameLocal->S_RegisterSound( "sound/feedback/fight.wav", true );
	cgs.media.countPrepareSound = cgameLocal->S_RegisterSound( "sound/feedback/prepare.wav", true );
#ifdef MISSIONPACK
	cgs.media.countPrepareTeamSound = cgameLocal->S_RegisterSound( "sound/feedback/prepare_team.wav", true );
#endif

	if ( cgs.gametype >= GT_TEAM || cg_buildScript.integer ) {

		cgs.media.captureAwardSound = cgameLocal->S_RegisterSound( "sound/teamplay/flagcapture_yourteam.wav", true );
		cgs.media.redLeadsSound = cgameLocal->S_RegisterSound( "sound/feedback/redleads.wav", true );
		cgs.media.blueLeadsSound = cgameLocal->S_RegisterSound( "sound/feedback/blueleads.wav", true );
		cgs.media.teamsTiedSound = cgameLocal->S_RegisterSound( "sound/feedback/teamstied.wav", true );
		cgs.media.hitTeamSound = cgameLocal->S_RegisterSound( "sound/feedback/hit_teammate.wav", true );

		cgs.media.redScoredSound = cgameLocal->S_RegisterSound( "sound/teamplay/voc_red_scores.wav", true );
		cgs.media.blueScoredSound = cgameLocal->S_RegisterSound( "sound/teamplay/voc_blue_scores.wav", true );

		cgs.media.captureYourTeamSound = cgameLocal->S_RegisterSound( "sound/teamplay/flagcapture_yourteam.wav", true );
		cgs.media.captureOpponentSound = cgameLocal->S_RegisterSound( "sound/teamplay/flagcapture_opponent.wav", true );

		cgs.media.returnYourTeamSound = cgameLocal->S_RegisterSound( "sound/teamplay/flagreturn_yourteam.wav", true );
		cgs.media.returnOpponentSound = cgameLocal->S_RegisterSound( "sound/teamplay/flagreturn_opponent.wav", true );

		cgs.media.takenYourTeamSound = cgameLocal->S_RegisterSound( "sound/teamplay/flagtaken_yourteam.wav", true );
		cgs.media.takenOpponentSound = cgameLocal->S_RegisterSound( "sound/teamplay/flagtaken_opponent.wav", true );

		if ( cgs.gametype == GT_CTF || cg_buildScript.integer ) {
			cgs.media.redFlagReturnedSound = cgameLocal->S_RegisterSound( "sound/teamplay/voc_red_returned.wav", true );
			cgs.media.blueFlagReturnedSound = cgameLocal->S_RegisterSound( "sound/teamplay/voc_blue_returned.wav", true );
			cgs.media.enemyTookYourFlagSound = cgameLocal->S_RegisterSound( "sound/teamplay/voc_enemy_flag.wav", true );
			cgs.media.yourTeamTookEnemyFlagSound = cgameLocal->S_RegisterSound( "sound/teamplay/voc_team_flag.wav", true );
		}

#ifdef MISSIONPACK
		if ( cgs.gametype == GT_1FCTF || cg_buildScript.integer ) {
			// FIXME: get a replacement for this sound ?
			cgs.media.neutralFlagReturnedSound = cgameLocal->S_RegisterSound( "sound/teamplay/flagreturn_opponent.wav", true );
			cgs.media.yourTeamTookTheFlagSound = cgameLocal->S_RegisterSound( "sound/teamplay/voc_team_1flag.wav", true );
			cgs.media.enemyTookTheFlagSound = cgameLocal->S_RegisterSound( "sound/teamplay/voc_enemy_1flag.wav", true );
		}

		if ( cgs.gametype == GT_1FCTF || cgs.gametype == GT_CTF || cg_buildScript.integer ) {
			cgs.media.youHaveFlagSound = cgameLocal->S_RegisterSound( "sound/teamplay/voc_you_flag.wav", true );
			cgs.media.holyShitSound = cgameLocal->S_RegisterSound( "sound/feedback/voc_holyshit.wav", true );
		}

		if ( cgs.gametype == GT_OBELISK || cg_buildScript.integer ) {
			cgs.media.yourBaseIsUnderAttackSound = cgameLocal->S_RegisterSound( "sound/teamplay/voc_base_attack.wav", true );
		}
#else
		cgs.media.youHaveFlagSound = cgameLocal->S_RegisterSound( "sound/teamplay/voc_you_flag.wav", true );
		cgs.media.holyShitSound = cgameLocal->S_RegisterSound( "sound/feedback/voc_holyshit.wav", true );
		cgs.media.neutralFlagReturnedSound = cgameLocal->S_RegisterSound( "sound/teamplay/flagreturn_opponent.wav", true );
		cgs.media.yourTeamTookTheFlagSound = cgameLocal->S_RegisterSound( "sound/teamplay/voc_team_1flag.wav", true );
		cgs.media.enemyTookTheFlagSound = cgameLocal->S_RegisterSound( "sound/teamplay/voc_enemy_1flag.wav", true );
#endif
	}

	cgs.media.tracerSound = cgameLocal->S_RegisterSound( "sound/weapons/machinegun/buletby1.wav", false );
	cgs.media.selectSound = cgameLocal->S_RegisterSound( "sound/weapons/change.wav", false );
	cgs.media.wearOffSound = cgameLocal->S_RegisterSound( "sound/items/wearoff.wav", false );
	cgs.media.useNothingSound = cgameLocal->S_RegisterSound( "sound/items/use_nothing.wav", false );
	cgs.media.gibSound = cgameLocal->S_RegisterSound( "sound/player/gibsplt1.wav", false );
	cgs.media.gibBounce1Sound = cgameLocal->S_RegisterSound( "sound/player/gibimp1.wav", false );
	cgs.media.gibBounce2Sound = cgameLocal->S_RegisterSound( "sound/player/gibimp2.wav", false );
	cgs.media.gibBounce3Sound = cgameLocal->S_RegisterSound( "sound/player/gibimp3.wav", false );

#ifdef MISSIONPACK
	cgs.media.useInvulnerabilitySound = cgameLocal->S_RegisterSound( "sound/items/invul_activate.wav", false );
	cgs.media.invulnerabilityImpactSound1 = cgameLocal->S_RegisterSound( "sound/items/invul_impact_01.wav", false );
	cgs.media.invulnerabilityImpactSound2 = cgameLocal->S_RegisterSound( "sound/items/invul_impact_02.wav", false );
	cgs.media.invulnerabilityImpactSound3 = cgameLocal->S_RegisterSound( "sound/items/invul_impact_03.wav", false );
	cgs.media.invulnerabilityJuicedSound = cgameLocal->S_RegisterSound( "sound/items/invul_juiced.wav", false );
	cgs.media.obeliskHitSound1 = cgameLocal->S_RegisterSound( "sound/items/obelisk_hit_01.wav", false );
	cgs.media.obeliskHitSound2 = cgameLocal->S_RegisterSound( "sound/items/obelisk_hit_02.wav", false );
	cgs.media.obeliskHitSound3 = cgameLocal->S_RegisterSound( "sound/items/obelisk_hit_03.wav", false );
	cgs.media.obeliskRespawnSound = cgameLocal->S_RegisterSound( "sound/items/obelisk_respawn.wav", false );

	cgs.media.ammoregenSound = cgameLocal->S_RegisterSound( "sound/items/cl_ammoregen.wav", false );
	cgs.media.doublerSound = cgameLocal->S_RegisterSound( "sound/items/cl_doubler.wav", false );
	cgs.media.guardSound = cgameLocal->S_RegisterSound( "sound/items/cl_guard.wav", false );
	cgs.media.scoutSound = cgameLocal->S_RegisterSound( "sound/items/cl_scout.wav", false );
#endif

	cgs.media.teleInSound = cgameLocal->S_RegisterSound( "sound/world/telein.wav", false );
	cgs.media.teleOutSound = cgameLocal->S_RegisterSound( "sound/world/teleout.wav", false );
	cgs.media.respawnSound = cgameLocal->S_RegisterSound( "sound/items/respawn1.wav", false );

	cgs.media.noAmmoSound = cgameLocal->S_RegisterSound( "sound/weapons/noammo.wav", false );

	cgs.media.talkSound = cgameLocal->S_RegisterSound( "sound/player/talk.wav", false );
	cgs.media.landSound = cgameLocal->S_RegisterSound( "sound/player/land1.wav", false );

	cgs.media.hitSound = cgameLocal->S_RegisterSound( "sound/feedback/hit.wav", false );
#ifdef MISSIONPACK
	cgs.media.hitSoundHighArmor = cgameLocal->S_RegisterSound( "sound/feedback/hithi.wav", false );
	cgs.media.hitSoundLowArmor = cgameLocal->S_RegisterSound( "sound/feedback/hitlo.wav", false );
#endif

	cgs.media.impressiveSound = cgameLocal->S_RegisterSound( "sound/feedback/impressive.wav", true );
	cgs.media.excellentSound = cgameLocal->S_RegisterSound( "sound/feedback/excellent.wav", true );
	cgs.media.deniedSound = cgameLocal->S_RegisterSound( "sound/feedback/denied.wav", true );
	cgs.media.humiliationSound = cgameLocal->S_RegisterSound( "sound/feedback/humiliation.wav", true );
	cgs.media.assistSound = cgameLocal->S_RegisterSound( "sound/feedback/assist.wav", true );
	cgs.media.defendSound = cgameLocal->S_RegisterSound( "sound/feedback/defense.wav", true );
#ifdef MISSIONPACK
	cgs.media.firstImpressiveSound = cgameLocal->S_RegisterSound( "sound/feedback/first_impressive.wav", true );
	cgs.media.firstExcellentSound = cgameLocal->S_RegisterSound( "sound/feedback/first_excellent.wav", true );
	cgs.media.firstHumiliationSound = cgameLocal->S_RegisterSound( "sound/feedback/first_gauntlet.wav", true );
#endif

	cgs.media.takenLeadSound = cgameLocal->S_RegisterSound( "sound/feedback/takenlead.wav", true );
	cgs.media.tiedLeadSound = cgameLocal->S_RegisterSound( "sound/feedback/tiedlead.wav", true );
	cgs.media.lostLeadSound = cgameLocal->S_RegisterSound( "sound/feedback/lostlead.wav", true );

#ifdef MISSIONPACK
	cgs.media.voteNow = cgameLocal->S_RegisterSound( "sound/feedback/vote_now.wav", true );
	cgs.media.votePassed = cgameLocal->S_RegisterSound( "sound/feedback/vote_passed.wav", true );
	cgs.media.voteFailed = cgameLocal->S_RegisterSound( "sound/feedback/vote_failed.wav", true );
#endif

	cgs.media.watrInSound = cgameLocal->S_RegisterSound( "sound/player/watr_in.wav", false );
	cgs.media.watrOutSound = cgameLocal->S_RegisterSound( "sound/player/watr_out.wav", false );
	cgs.media.watrUnSound = cgameLocal->S_RegisterSound( "sound/player/watr_un.wav", false );

	cgs.media.jumpPadSound = cgameLocal->S_RegisterSound ( "sound/world/jumppad.wav", false );

	for ( i = 0 ; i < 4 ; i++ ) {
		Com_sprintf ( name, sizeof( name ), "sound/player/footsteps/step%i.wav", i + 1 );
		cgs.media.footsteps[FOOTSTEP_NORMAL][i] = cgameLocal->S_RegisterSound ( name, false );

		Com_sprintf ( name, sizeof( name ), "sound/player/footsteps/boot%i.wav", i + 1 );
		cgs.media.footsteps[FOOTSTEP_BOOT][i] = cgameLocal->S_RegisterSound ( name, false );

		Com_sprintf ( name, sizeof( name ), "sound/player/footsteps/flesh%i.wav", i + 1 );
		cgs.media.footsteps[FOOTSTEP_FLESH][i] = cgameLocal->S_RegisterSound ( name, false );

		Com_sprintf ( name, sizeof( name ), "sound/player/footsteps/mech%i.wav", i + 1 );
		cgs.media.footsteps[FOOTSTEP_MECH][i] = cgameLocal->S_RegisterSound ( name, false );

		Com_sprintf ( name, sizeof( name ), "sound/player/footsteps/energy%i.wav", i + 1 );
		cgs.media.footsteps[FOOTSTEP_ENERGY][i] = cgameLocal->S_RegisterSound ( name, false );

		Com_sprintf ( name, sizeof( name ), "sound/player/footsteps/splash%i.wav", i + 1 );
		cgs.media.footsteps[FOOTSTEP_SPLASH][i] = cgameLocal->S_RegisterSound ( name, false );

		Com_sprintf ( name, sizeof( name ), "sound/player/footsteps/clank%i.wav", i + 1 );
		cgs.media.footsteps[FOOTSTEP_METAL][i] = cgameLocal->S_RegisterSound ( name, false );
	}

	// only register the items that the server says we need
	strcpy( items, CG_ConfigString( CS_ITEMS ) );

	for ( i = 1 ; i < bg_numItems ; i++ ) {
//		if ( items[ i ] == '1' || cg_buildScript.integer ) {
		CG_RegisterItemSounds( i );
//		}
	}

	for ( i = 1 ; i < MAX_SOUNDS ; i++ ) {
		soundName = CG_ConfigString( CS_SOUNDS + i );
		if ( !soundName[0] ) {
			break;
		}
		if ( soundName[0] == '*' ) {
			continue;	// custom sound
		}
		cgs.gameSounds[i] = cgameLocal->S_RegisterSound( soundName, false );
	}

	// FIXME: only needed with item
	cgs.media.flightSound = cgameLocal->S_RegisterSound( "sound/items/flight.wav", false );
	cgs.media.medkitSound = cgameLocal->S_RegisterSound ( "sound/items/use_medkit.wav", false );
	cgs.media.quadSound = cgameLocal->S_RegisterSound( "sound/items/damage3.wav", false );
	cgs.media.sfx_ric1 = cgameLocal->S_RegisterSound ( "sound/weapons/machinegun/ric1.wav", false );
	cgs.media.sfx_ric2 = cgameLocal->S_RegisterSound ( "sound/weapons/machinegun/ric2.wav", false );
	cgs.media.sfx_ric3 = cgameLocal->S_RegisterSound ( "sound/weapons/machinegun/ric3.wav", false );
	cgs.media.sfx_railg = cgameLocal->S_RegisterSound ( "sound/weapons/railgun/railgf1a.wav", false );
	cgs.media.sfx_rockexp = cgameLocal->S_RegisterSound ( "sound/weapons/rocket/rocklx1a.wav", false );
	cgs.media.sfx_plasmaexp = cgameLocal->S_RegisterSound ( "sound/weapons/plasma/plasmx1a.wav", false );
#ifdef MISSIONPACK
	cgs.media.sfx_proxexp = cgameLocal->S_RegisterSound( "sound/weapons/proxmine/wstbexpl.wav", false );
	cgs.media.sfx_nghit = cgameLocal->S_RegisterSound( "sound/weapons/nailgun/wnalimpd.wav", false );
	cgs.media.sfx_nghitflesh = cgameLocal->S_RegisterSound( "sound/weapons/nailgun/wnalimpl.wav", false );
	cgs.media.sfx_nghitmetal = cgameLocal->S_RegisterSound( "sound/weapons/nailgun/wnalimpm.wav", false );
	cgs.media.sfx_chghit = cgameLocal->S_RegisterSound( "sound/weapons/vulcan/wvulimpd.wav", false );
	cgs.media.sfx_chghitflesh = cgameLocal->S_RegisterSound( "sound/weapons/vulcan/wvulimpl.wav", false );
	cgs.media.sfx_chghitmetal = cgameLocal->S_RegisterSound( "sound/weapons/vulcan/wvulimpm.wav", false );
	cgs.media.weaponHoverSound = cgameLocal->S_RegisterSound( "sound/weapons/weapon_hover.wav", false );
	cgs.media.kamikazeExplodeSound = cgameLocal->S_RegisterSound( "sound/items/kam_explode.wav", false );
	cgs.media.kamikazeImplodeSound = cgameLocal->S_RegisterSound( "sound/items/kam_implode.wav", false );
	cgs.media.kamikazeFarSound = cgameLocal->S_RegisterSound( "sound/items/kam_explode_far.wav", false );
	cgs.media.winnerSound = cgameLocal->S_RegisterSound( "sound/feedback/voc_youwin.wav", false );
	cgs.media.loserSound = cgameLocal->S_RegisterSound( "sound/feedback/voc_youlose.wav", false );
	cgs.media.youSuckSound = cgameLocal->S_RegisterSound( "sound/misc/yousuck.wav", false );

	cgs.media.wstbimplSound = cgameLocal->S_RegisterSound( "sound/weapons/proxmine/wstbimpl.wav", false );
	cgs.media.wstbimpmSound = cgameLocal->S_RegisterSound( "sound/weapons/proxmine/wstbimpm.wav", false );
	cgs.media.wstbimpdSound = cgameLocal->S_RegisterSound( "sound/weapons/proxmine/wstbimpd.wav", false );
	cgs.media.wstbactvSound = cgameLocal->S_RegisterSound( "sound/weapons/proxmine/wstbactv.wav", false );
#endif

	cgs.media.regenSound = cgameLocal->S_RegisterSound( "sound/items/regen.wav", false );
	cgs.media.protectSound = cgameLocal->S_RegisterSound( "sound/items/protect3.wav", false );
	cgs.media.n_healthSound = cgameLocal->S_RegisterSound( "sound/items/n_health.wav", false );
	cgs.media.hgrenb1aSound = cgameLocal->S_RegisterSound( "sound/weapons/grenade/hgrenb1a.wav", false );
	cgs.media.hgrenb2aSound = cgameLocal->S_RegisterSound( "sound/weapons/grenade/hgrenb2a.wav", false );

#ifdef MISSIONPACK
	cgameLocal->S_RegisterSound( "sound/player/james/death1.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/james/death2.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/james/death3.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/james/jump1.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/james/pain25_1.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/james/pain75_1.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/james/pain100_1.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/james/falling1.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/james/gasp.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/james/drown.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/james/fall1.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/james/taunt.wav", false );

	cgameLocal->S_RegisterSound( "sound/player/janet/death1.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/janet/death2.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/janet/death3.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/janet/jump1.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/janet/pain25_1.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/janet/pain75_1.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/janet/pain100_1.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/janet/falling1.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/janet/gasp.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/janet/drown.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/janet/fall1.wav", false );
	cgameLocal->S_RegisterSound( "sound/player/janet/taunt.wav", false );
#endif

}


//===================================================================================


/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterGraphics( void ) {
	int			i;
	char		items[MAX_ITEMS + 1];
	static char	*	sb_nums[11] = {
		"gfx/2d/numbers/zero_32b",
		"gfx/2d/numbers/one_32b",
		"gfx/2d/numbers/two_32b",
		"gfx/2d/numbers/three_32b",
		"gfx/2d/numbers/four_32b",
		"gfx/2d/numbers/five_32b",
		"gfx/2d/numbers/six_32b",
		"gfx/2d/numbers/seven_32b",
		"gfx/2d/numbers/eight_32b",
		"gfx/2d/numbers/nine_32b",
		"gfx/2d/numbers/minus_32b",
	};

	// clear any references to old media
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	cgameLocal->R_ClearScene();

	CG_LoadingString( cgs.mapname );

	cgameLocal->R_LoadWorldMap( cgs.mapname );

	// precache status bar pics
	CG_LoadingString( "game media" );

	for ( i = 0 ; i < 11 ; i++ ) {
		cgs.media.numberShaders[i] = cgameLocal->R_RegisterShader( sb_nums[i] );
	}

	cgs.media.viewBloodShader = cgameLocal->R_RegisterShader( "viewBloodBlend" );

	cgs.media.deferShader = cgameLocal->R_RegisterShaderNoMip( "gfx/2d/defer.tga" );

	cgs.media.scoreboardName = cgameLocal->R_RegisterShaderNoMip( "menu/tab/name.tga" );
	cgs.media.scoreboardPing = cgameLocal->R_RegisterShaderNoMip( "menu/tab/ping.tga" );
	cgs.media.scoreboardScore = cgameLocal->R_RegisterShaderNoMip( "menu/tab/score.tga" );
	cgs.media.scoreboardTime = cgameLocal->R_RegisterShaderNoMip( "menu/tab/time.tga" );

	cgs.media.smokePuffShader = cgameLocal->R_RegisterShader( "smokePuff" );
	cgs.media.smokePuffRageProShader = cgameLocal->R_RegisterShader( "smokePuffRagePro" );
	cgs.media.shotgunSmokePuffShader = cgameLocal->R_RegisterShader( "shotgunSmokePuff" );
#ifdef MISSIONPACK
	cgs.media.nailPuffShader = cgameLocal->R_RegisterShader( "nailtrail" );
	cgs.media.blueProxMine = cgameLocal->R_RegisterModel( "models/weaphits/proxmineb.md3" );
#endif
	cgs.media.plasmaBallShader = cgameLocal->R_RegisterShader( "sprites/plasma1" );
	cgs.media.bloodTrailShader = cgameLocal->R_RegisterShader( "bloodTrail" );
	cgs.media.lagometerShader = cgameLocal->R_RegisterShader( "lagometer" );
	cgs.media.connectionShader = cgameLocal->R_RegisterShader( "disconnected" );

	cgs.media.waterBubbleShader = cgameLocal->R_RegisterShader( "waterBubble" );

	cgs.media.tracerShader = cgameLocal->R_RegisterShader( "gfx/misc/tracer" );
	cgs.media.selectShader = cgameLocal->R_RegisterShader( "gfx/2d/select" );

	for ( i = 0 ; i < NUM_CROSSHAIRS ; i++ ) {
		cgs.media.crosshairShader[i] = cgameLocal->R_RegisterShader( va( "gfx/2d/crosshair%c", 'a' + i ) );
	}

	cgs.media.backTileShader = cgameLocal->R_RegisterShader( "gfx/2d/backtile" );
	cgs.media.noammoShader = cgameLocal->R_RegisterShader( "icons/noammo" );

	// powerup shaders
	cgs.media.quadShader = cgameLocal->R_RegisterShader( "powerups/quad" );
	cgs.media.quadWeaponShader = cgameLocal->R_RegisterShader( "powerups/quadWeapon" );
	cgs.media.battleSuitShader = cgameLocal->R_RegisterShader( "powerups/battleSuit" );
	cgs.media.battleWeaponShader = cgameLocal->R_RegisterShader( "powerups/battleWeapon" );
	cgs.media.invisShader = cgameLocal->R_RegisterShader( "powerups/invisibility" );
	cgs.media.regenShader = cgameLocal->R_RegisterShader( "powerups/regen" );
	cgs.media.hastePuffShader = cgameLocal->R_RegisterShader( "hasteSmokePuff" );

#ifdef MISSIONPACK
	if ( cgs.gametype == GT_CTF || cgs.gametype == GT_1FCTF || cgs.gametype == GT_HARVESTER || cg_buildScript.integer ) {
#else
	if ( cgs.gametype == GT_CTF || cg_buildScript.integer ) {
#endif
		cgs.media.redCubeModel = cgameLocal->R_RegisterModel( "models/powerups/orb/r_orb.md3" );
		cgs.media.blueCubeModel = cgameLocal->R_RegisterModel( "models/powerups/orb/b_orb.md3" );
		cgs.media.redCubeIcon = cgameLocal->R_RegisterShader( "icons/skull_red" );
		cgs.media.blueCubeIcon = cgameLocal->R_RegisterShader( "icons/skull_blue" );
	}

#ifdef MISSIONPACK
	if ( cgs.gametype == GT_CTF || cgs.gametype == GT_1FCTF || cgs.gametype == GT_HARVESTER || cg_buildScript.integer ) {
#else
	if ( cgs.gametype == GT_CTF || cg_buildScript.integer ) {
#endif
		cgs.media.redFlagModel = cgameLocal->R_RegisterModel( "models/flags/r_flag.md3" );
		cgs.media.blueFlagModel = cgameLocal->R_RegisterModel( "models/flags/b_flag.md3" );
		cgs.media.redFlagShader[0] = cgameLocal->R_RegisterShaderNoMip( "icons/iconf_red1" );
		cgs.media.redFlagShader[1] = cgameLocal->R_RegisterShaderNoMip( "icons/iconf_red2" );
		cgs.media.redFlagShader[2] = cgameLocal->R_RegisterShaderNoMip( "icons/iconf_red3" );
		cgs.media.blueFlagShader[0] = cgameLocal->R_RegisterShaderNoMip( "icons/iconf_blu1" );
		cgs.media.blueFlagShader[1] = cgameLocal->R_RegisterShaderNoMip( "icons/iconf_blu2" );
		cgs.media.blueFlagShader[2] = cgameLocal->R_RegisterShaderNoMip( "icons/iconf_blu3" );
#ifdef MISSIONPACK
		cgs.media.flagPoleModel = cgameLocal->R_RegisterModel( "models/flag2/flagpole.md3" );
		cgs.media.flagFlapModel = cgameLocal->R_RegisterModel( "models/flag2/flagflap3.md3" );

		cgs.media.redFlagFlapSkin = cgameLocal->R_RegisterSkin( "models/flag2/red.skin" );
		cgs.media.blueFlagFlapSkin = cgameLocal->R_RegisterSkin( "models/flag2/blue.skin" );
		cgs.media.neutralFlagFlapSkin = cgameLocal->R_RegisterSkin( "models/flag2/white.skin" );

		cgs.media.redFlagBaseModel = cgameLocal->R_RegisterModel( "models/mapobjects/flagbase/red_base.md3" );
		cgs.media.blueFlagBaseModel = cgameLocal->R_RegisterModel( "models/mapobjects/flagbase/blue_base.md3" );
		cgs.media.neutralFlagBaseModel = cgameLocal->R_RegisterModel( "models/mapobjects/flagbase/ntrl_base.md3" );
#endif
	}

#ifdef MISSIONPACK
	if ( cgs.gametype == GT_1FCTF || cg_buildScript.integer ) {
		cgs.media.neutralFlagModel = cgameLocal->R_RegisterModel( "models/flags/n_flag.md3" );
		cgs.media.flagShader[0] = cgameLocal->R_RegisterShaderNoMip( "icons/iconf_neutral1" );
		cgs.media.flagShader[1] = cgameLocal->R_RegisterShaderNoMip( "icons/iconf_red2" );
		cgs.media.flagShader[2] = cgameLocal->R_RegisterShaderNoMip( "icons/iconf_blu2" );
		cgs.media.flagShader[3] = cgameLocal->R_RegisterShaderNoMip( "icons/iconf_neutral3" );
	}

	if ( cgs.gametype == GT_OBELISK || cg_buildScript.integer ) {
		cgs.media.overloadBaseModel = cgameLocal->R_RegisterModel( "models/powerups/overload_base.md3" );
		cgs.media.overloadTargetModel = cgameLocal->R_RegisterModel( "models/powerups/overload_target.md3" );
		cgs.media.overloadLightsModel = cgameLocal->R_RegisterModel( "models/powerups/overload_lights.md3" );
		cgs.media.overloadEnergyModel = cgameLocal->R_RegisterModel( "models/powerups/overload_energy.md3" );
	}

	if ( cgs.gametype == GT_HARVESTER || cg_buildScript.integer ) {
		cgs.media.harvesterModel = cgameLocal->R_RegisterModel( "models/powerups/harvester/harvester.md3" );
		cgs.media.harvesterRedSkin = cgameLocal->R_RegisterSkin( "models/powerups/harvester/red.skin" );
		cgs.media.harvesterBlueSkin = cgameLocal->R_RegisterSkin( "models/powerups/harvester/blue.skin" );
		cgs.media.harvesterNeutralModel = cgameLocal->R_RegisterModel( "models/powerups/obelisk/obelisk.md3" );
	}

	cgs.media.redKamikazeShader = cgameLocal->R_RegisterShader( "models/weaphits/kamikred" );
	cgs.media.dustPuffShader = cgameLocal->R_RegisterShader( "hasteSmokePuff" );
#endif

	if ( cgs.gametype >= GT_TEAM || cg_buildScript.integer ) {
		cgs.media.friendShader = cgameLocal->R_RegisterShader( "sprites/foe" );
		cgs.media.redQuadShader = cgameLocal->R_RegisterShader( "powerups/blueflag" );
		cgs.media.teamStatusBar = cgameLocal->R_RegisterShader( "gfx/2d/colorbar.tga" );
#ifdef MISSIONPACK
		cgs.media.blueKamikazeShader = cgameLocal->R_RegisterShader( "models/weaphits/kamikblu" );
#endif
	}

	cgs.media.armorModel = cgameLocal->R_RegisterModel( "models/powerups/armor/armor_yel.md3" );
	cgs.media.armorIcon  = cgameLocal->R_RegisterShaderNoMip( "icons/iconr_yellow" );

	cgs.media.machinegunBrassModel = cgameLocal->R_RegisterModel( "models/weapons2/shells/m_shell.md3" );
	cgs.media.shotgunBrassModel = cgameLocal->R_RegisterModel( "models/weapons2/shells/s_shell.md3" );

	cgs.media.gibAbdomen = cgameLocal->R_RegisterModel( "models/gibs/abdomen.md3" );
	cgs.media.gibArm = cgameLocal->R_RegisterModel( "models/gibs/arm.md3" );
	cgs.media.gibChest = cgameLocal->R_RegisterModel( "models/gibs/chest.md3" );
	cgs.media.gibFist = cgameLocal->R_RegisterModel( "models/gibs/fist.md3" );
	cgs.media.gibFoot = cgameLocal->R_RegisterModel( "models/gibs/foot.md3" );
	cgs.media.gibForearm = cgameLocal->R_RegisterModel( "models/gibs/forearm.md3" );
	cgs.media.gibIntestine = cgameLocal->R_RegisterModel( "models/gibs/intestine.md3" );
	cgs.media.gibLeg = cgameLocal->R_RegisterModel( "models/gibs/leg.md3" );
	cgs.media.gibSkull = cgameLocal->R_RegisterModel( "models/gibs/skull.md3" );
	cgs.media.gibBrain = cgameLocal->R_RegisterModel( "models/gibs/brain.md3" );

	cgs.media.smoke2 = cgameLocal->R_RegisterModel( "models/weapons2/shells/s_shell.md3" );

	cgs.media.balloonShader = cgameLocal->R_RegisterShader( "sprites/balloon3" );

	cgs.media.bloodExplosionShader = cgameLocal->R_RegisterShader( "bloodExplosion" );

	cgs.media.bulletFlashModel = cgameLocal->R_RegisterModel( "models/weaphits/bullet.md3" );
	cgs.media.ringFlashModel = cgameLocal->R_RegisterModel( "models/weaphits/ring02.md3" );
	cgs.media.dishFlashModel = cgameLocal->R_RegisterModel( "models/weaphits/boom01.md3" );
#ifdef MISSIONPACK
	cgs.media.teleportEffectModel = cgameLocal->R_RegisterModel( "models/powerups/pop.md3" );
#else
	cgs.media.teleportEffectModel = cgameLocal->R_RegisterModel( "models/misc/telep.md3" );
	cgs.media.teleportEffectShader = cgameLocal->R_RegisterShader( "teleportEffect" );
#endif
#ifdef MISSIONPACK
	cgs.media.kamikazeEffectModel = cgameLocal->R_RegisterModel( "models/weaphits/kamboom2.md3" );
	cgs.media.kamikazeShockWave = cgameLocal->R_RegisterModel( "models/weaphits/kamwave.md3" );
	cgs.media.kamikazeHeadModel = cgameLocal->R_RegisterModel( "models/powerups/kamikazi.md3" );
	cgs.media.kamikazeHeadTrail = cgameLocal->R_RegisterModel( "models/powerups/trailtest.md3" );
	cgs.media.guardPowerupModel = cgameLocal->R_RegisterModel( "models/powerups/guard_player.md3" );
	cgs.media.scoutPowerupModel = cgameLocal->R_RegisterModel( "models/powerups/scout_player.md3" );
	cgs.media.doublerPowerupModel = cgameLocal->R_RegisterModel( "models/powerups/doubler_player.md3" );
	cgs.media.ammoRegenPowerupModel = cgameLocal->R_RegisterModel( "models/powerups/ammo_player.md3" );
	cgs.media.invulnerabilityImpactModel = cgameLocal->R_RegisterModel( "models/powerups/shield/impact.md3" );
	cgs.media.invulnerabilityJuicedModel = cgameLocal->R_RegisterModel( "models/powerups/shield/juicer.md3" );
	cgs.media.medkitUsageModel = cgameLocal->R_RegisterModel( "models/powerups/regen.md3" );
	cgs.media.heartShader = cgameLocal->R_RegisterShaderNoMip( "ui/assets/statusbar/selectedhealth.tga" );

#endif

	cgs.media.invulnerabilityPowerupModel = cgameLocal->R_RegisterModel( "models/powerups/shield/shield.md3" );
	cgs.media.medalImpressive = cgameLocal->R_RegisterShaderNoMip( "medal_impressive" );
	cgs.media.medalExcellent = cgameLocal->R_RegisterShaderNoMip( "medal_excellent" );
	cgs.media.medalGauntlet = cgameLocal->R_RegisterShaderNoMip( "medal_gauntlet" );
	cgs.media.medalDefend = cgameLocal->R_RegisterShaderNoMip( "medal_defend" );
	cgs.media.medalAssist = cgameLocal->R_RegisterShaderNoMip( "medal_assist" );
	cgs.media.medalCapture = cgameLocal->R_RegisterShaderNoMip( "medal_capture" );


	memset( cg_items, 0, sizeof( cg_items ) );
	memset( cg_weapons, 0, sizeof( cg_weapons ) );

	// only register the items that the server says we need
	strcpy( items, CG_ConfigString( CS_ITEMS ) );

	for ( i = 1 ; i < bg_numItems ; i++ ) {
		if ( items[ i ] == '1' || cg_buildScript.integer ) {
			CG_LoadingItem( i );
			CG_RegisterItemVisuals( i );
		}
	}

	// wall marks
	cgs.media.bulletMarkShader = cgameLocal->R_RegisterShader( "gfx/damage/bullet_mrk" );
	cgs.media.burnMarkShader = cgameLocal->R_RegisterShader( "gfx/damage/burn_med_mrk" );
	cgs.media.holeMarkShader = cgameLocal->R_RegisterShader( "gfx/damage/hole_lg_mrk" );
	cgs.media.energyMarkShader = cgameLocal->R_RegisterShader( "gfx/damage/plasma_mrk" );
	cgs.media.shadowMarkShader = cgameLocal->R_RegisterShader( "markShadow" );
	cgs.media.wakeMarkShader = cgameLocal->R_RegisterShader( "wake" );
	cgs.media.bloodMarkShader = cgameLocal->R_RegisterShader( "bloodMark" );

	// register the inline models
	cgs.numInlineModels = cgameLocal->CM_NumInlineModels();
	for ( i = 1 ; i < cgs.numInlineModels ; i++ ) {
		char	name[10];
		vec3_t			mins, maxs;
		int				j;

		Com_sprintf( name, sizeof( name ), "*%i", i );
		cgs.inlineDrawModel[i] = cgameLocal->R_RegisterModel( name );
		cgameLocal->R_ModelBounds( cgs.inlineDrawModel[i], mins, maxs );
		for ( j = 0 ; j < 3 ; j++ ) {
			cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
		}
	}

	// register all the server specified models
	for ( i = 1 ; i < MAX_MODELS ; i++ ) {
		const char	*	modelName;

		modelName = CG_ConfigString( CS_MODELS + i );
		if ( !modelName[0] ) {
			break;
		}
		cgs.gameModels[i] = cgameLocal->R_RegisterModel( modelName );
	}

#ifdef MISSIONPACK
	// new stuff
	cgs.media.patrolShader = cgameLocal->R_RegisterShaderNoMip( "ui/assets/statusbar/patrol.tga" );
	cgs.media.assaultShader = cgameLocal->R_RegisterShaderNoMip( "ui/assets/statusbar/assault.tga" );
	cgs.media.campShader = cgameLocal->R_RegisterShaderNoMip( "ui/assets/statusbar/camp.tga" );
	cgs.media.followShader = cgameLocal->R_RegisterShaderNoMip( "ui/assets/statusbar/follow.tga" );
	cgs.media.defendShader = cgameLocal->R_RegisterShaderNoMip( "ui/assets/statusbar/defend.tga" );
	cgs.media.teamLeaderShader = cgameLocal->R_RegisterShaderNoMip( "ui/assets/statusbar/team_leader.tga" );
	cgs.media.retrieveShader = cgameLocal->R_RegisterShaderNoMip( "ui/assets/statusbar/retrieve.tga" );
	cgs.media.escortShader = cgameLocal->R_RegisterShaderNoMip( "ui/assets/statusbar/escort.tga" );
	cgs.media.cursor = cgameLocal->R_RegisterShaderNoMip( "menu/art/3_cursor2" );
	cgs.media.sizeCursor = cgameLocal->R_RegisterShaderNoMip( "ui/assets/sizecursor.tga" );
	cgs.media.selectCursor = cgameLocal->R_RegisterShaderNoMip( "ui/assets/selectcursor.tga" );
	cgs.media.flagShaders[0] = cgameLocal->R_RegisterShaderNoMip( "ui/assets/statusbar/flag_in_base.tga" );
	cgs.media.flagShaders[1] = cgameLocal->R_RegisterShaderNoMip( "ui/assets/statusbar/flag_capture.tga" );
	cgs.media.flagShaders[2] = cgameLocal->R_RegisterShaderNoMip( "ui/assets/statusbar/flag_missing.tga" );

	cgameLocal->R_RegisterModel( "models/players/james/lower.md3" );
	cgameLocal->R_RegisterModel( "models/players/james/upper.md3" );
	cgameLocal->R_RegisterModel( "models/players/heads/james/james.md3" );

	cgameLocal->R_RegisterModel( "models/players/janet/lower.md3" );
	cgameLocal->R_RegisterModel( "models/players/janet/upper.md3" );
	cgameLocal->R_RegisterModel( "models/players/heads/janet/janet.md3" );

#endif
	CG_ClearParticles ();
	/*
		for (i=1; i<MAX_PARTICLES_AREAS; i++)
		{
			{
				int rval;

				rval = CG_NewParticleArea ( CS_PARTICLES + i);
				if (!rval)
					break;
			}
		}
	*/
}


/*
=======================
CG_BuildSpectatorString

=======================
*/
void CG_BuildSpectatorString() {
	int i;
	cg.spectatorList[0] = 0;
	for ( i = 0; i < MAX_CLIENTS; i++ ) {
		if ( cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_SPECTATOR ) {
			Q_strcat( cg.spectatorList, sizeof( cg.spectatorList ), va( "%s     ", cgs.clientinfo[i].name ) );
		}
	}
	i = strlen( cg.spectatorList );
	if ( i != cg.spectatorLen ) {
		cg.spectatorLen = i;
		cg.spectatorWidth = -1;
	}
}


/*
===================
CG_RegisterClients
===================
*/
static void CG_RegisterClients( void ) {
	int		i;

	CG_LoadingClient( cg.clientNum );
	CG_NewClientInfo( cg.clientNum );

	for ( i = 0 ; i < MAX_CLIENTS ; i++ ) {
		const char	*	clientInfo;

		if ( cg.clientNum == i ) {
			continue;
		}

		clientInfo = CG_ConfigString( CS_PLAYERS + i );
		if ( !clientInfo[0] ) {
			continue;
		}
		CG_LoadingClient( i );
		CG_NewClientInfo( i );
	}
	CG_BuildSpectatorString();
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char * CG_ConfigString( int index ) {
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		CG_Error( "CG_ConfigString: bad index: %i", index );
	}
	return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

/*
======================
CG_StartMusic

======================
*/
void CG_StartMusic( void ) {
	char	* s;
	char	parm1[MAX_QPATH], parm2[MAX_QPATH];

	// start the background music
	s = ( char * )CG_ConfigString( CS_MUSIC );
	Q_strncpyz( parm1, COM_Parse( &s ), sizeof( parm1 ) );
	Q_strncpyz( parm2, COM_Parse( &s ), sizeof( parm2 ) );

	cgameLocal->S_StartBackgroundTrack( parm1, parm2 );
}
#ifdef MISSIONPACK
char * CG_GetMenuBuffer( const char * filename ) {
	int	len;
	fileHandle_t	f;
	static char buf[MAX_MENUFILE];

	len = cgameLocal->FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		cgameLocal->Print( va( S_COLOR_RED "menu file not found: %s, using default\n", filename ) );
		return NULL;
	}
	if ( len >= MAX_MENUFILE ) {
		cgameLocal->Print( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", filename, len, MAX_MENUFILE ) );
		cgameLocal->FS_FCloseFile( f );
		return NULL;
	}

	cgameLocal->FS_Read( buf, len, f );
	buf[len] = 0;
	cgameLocal->FS_FCloseFile( f );

	return buf;
}

//
// ==============================
// new hud stuff ( mission pack )
// ==============================
//
bool CG_Asset_Parse( int handle ) {
	pc_token_t token;
	const char * tempStr;

	if ( !cgameLocal->PC_ReadToken( handle, &token ) ) {
		return false;
	}
	if ( Q_stricmp( token.string, "{" ) != 0 ) {
		return false;
	}

	while ( 1 ) {
		if ( !cgameLocal->PC_ReadToken( handle, &token ) ) {
			return false;
		}

		if ( Q_stricmp( token.string, "}" ) == 0 ) {
			return true;
		}

		// font
		if ( Q_stricmp( token.string, "font" ) == 0 ) {
			int pointSize;
			if ( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle, &pointSize ) ) {
				return false;
			}
			cgDC.registerFont( tempStr, pointSize, &cgDC.Assets.textFont );
			continue;
		}

		// smallFont
		if ( Q_stricmp( token.string, "smallFont" ) == 0 ) {
			int pointSize;
			if ( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle, &pointSize ) ) {
				return false;
			}
			cgDC.registerFont( tempStr, pointSize, &cgDC.Assets.smallFont );
			continue;
		}

		// font
		if ( Q_stricmp( token.string, "bigfont" ) == 0 ) {
			int pointSize;
			if ( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle, &pointSize ) ) {
				return false;
			}
			cgDC.registerFont( tempStr, pointSize, &cgDC.Assets.bigFont );
			continue;
		}

		// gradientbar
		if ( Q_stricmp( token.string, "gradientbar" ) == 0 ) {
			if ( !PC_String_Parse( handle, &tempStr ) ) {
				return false;
			}
			cgDC.Assets.gradientBar = cgameLocal->R_RegisterShaderNoMip( tempStr );
			continue;
		}

		// enterMenuSound
		if ( Q_stricmp( token.string, "menuEnterSound" ) == 0 ) {
			if ( !PC_String_Parse( handle, &tempStr ) ) {
				return false;
			}
			cgDC.Assets.menuEnterSound = cgameLocal->S_RegisterSound( tempStr, false );
			continue;
		}

		// exitMenuSound
		if ( Q_stricmp( token.string, "menuExitSound" ) == 0 ) {
			if ( !PC_String_Parse( handle, &tempStr ) ) {
				return false;
			}
			cgDC.Assets.menuExitSound = cgameLocal->S_RegisterSound( tempStr, false );
			continue;
		}

		// itemFocusSound
		if ( Q_stricmp( token.string, "itemFocusSound" ) == 0 ) {
			if ( !PC_String_Parse( handle, &tempStr ) ) {
				return false;
			}
			cgDC.Assets.itemFocusSound = cgameLocal->S_RegisterSound( tempStr, false );
			continue;
		}

		// menuBuzzSound
		if ( Q_stricmp( token.string, "menuBuzzSound" ) == 0 ) {
			if ( !PC_String_Parse( handle, &tempStr ) ) {
				return false;
			}
			cgDC.Assets.menuBuzzSound = cgameLocal->S_RegisterSound( tempStr, false );
			continue;
		}

		if ( Q_stricmp( token.string, "cursor" ) == 0 ) {
			if ( !PC_String_Parse( handle, &cgDC.Assets.cursorStr ) ) {
				return false;
			}
			cgDC.Assets.cursor = cgameLocal->R_RegisterShaderNoMip( cgDC.Assets.cursorStr );
			continue;
		}

		if ( Q_stricmp( token.string, "fadeClamp" ) == 0 ) {
			if ( !PC_Float_Parse( handle, &cgDC.Assets.fadeClamp ) ) {
				return false;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "fadeCycle" ) == 0 ) {
			if ( !PC_Int_Parse( handle, &cgDC.Assets.fadeCycle ) ) {
				return false;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "fadeAmount" ) == 0 ) {
			if ( !PC_Float_Parse( handle, &cgDC.Assets.fadeAmount ) ) {
				return false;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "shadowX" ) == 0 ) {
			if ( !PC_Float_Parse( handle, &cgDC.Assets.shadowX ) ) {
				return false;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "shadowY" ) == 0 ) {
			if ( !PC_Float_Parse( handle, &cgDC.Assets.shadowY ) ) {
				return false;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "shadowColor" ) == 0 ) {
			if ( !PC_Color_Parse( handle, &cgDC.Assets.shadowColor ) ) {
				return false;
			}
			cgDC.Assets.shadowFadeClamp = cgDC.Assets.shadowColor[3];
			continue;
		}
	}
	return false; // bk001204 - why not?
}

void CG_ParseMenu( const char * menuFile ) {
	pc_token_t token;
	int handle;

	handle = cgameLocal->PC_LoadSource( menuFile );
	if ( !handle ) {
		handle = cgameLocal->PC_LoadSource( "ui/testhud.menu" );
	}
	if ( !handle ) {
		return;
	}

	while ( 1 ) {
		if ( !cgameLocal->PC_ReadToken( handle, &token ) ) {
			break;
		}

		//if ( Q_stricmp( token, "{" ) ) {
		//	cgameLocal->Print( "Missing { in menu file\n" );
		//	break;
		//}

		//if ( menuCount == MAX_MENUS ) {
		//	cgameLocal->Print( "Too many menus!\n" );
		//	break;
		//}

		if ( token.string[0] == '}' ) {
			break;
		}

		if ( Q_stricmp( token.string, "assetGlobalDef" ) == 0 ) {
			if ( CG_Asset_Parse( handle ) ) {
				continue;
			} else {
				break;
			}
		}


		if ( Q_stricmp( token.string, "menudef" ) == 0 ) {
			// start a new menu
			Menu_New( handle );
		}
	}
	cgameLocal->PC_FreeSource( handle );
}

bool CG_Load_Menu( char ** p ) {
	char * token;

	token = COM_ParseExt( p, true );

	if ( token[0] != '{' ) {
		return false;
	}

	while ( 1 ) {

		token = COM_ParseExt( p, true );

		if ( Q_stricmp( token, "}" ) == 0 ) {
			return true;
		}

		if ( !token || token[0] == 0 ) {
			return false;
		}

		CG_ParseMenu( token );
	}
	return false;
}


void CG_LoadMenus( const char * menuFile ) {
	char	* token;
	char * p;
	int	len, start;
	fileHandle_t	f;
	static char buf[MAX_MENUDEFFILE];

	start = cgameLocal->Milliseconds();

	len = cgameLocal->FS_FOpenFile( menuFile, &f, FS_READ );
	if ( !f ) {
		cgameLocal->Error( va( S_COLOR_YELLOW "menu file not found: %s, using default\n", menuFile ) );
		len = cgameLocal->FS_FOpenFile( "ui/hud.txt", &f, FS_READ );
		if ( !f ) {
			cgameLocal->Error( va( S_COLOR_RED "default menu file not found: ui/hud.txt, unable to continue!\n", menuFile ) );
		}
	}

	if ( len >= MAX_MENUDEFFILE ) {
		cgameLocal->Error( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", menuFile, len, MAX_MENUDEFFILE ) );
		cgameLocal->FS_FCloseFile( f );
		return;
	}

	cgameLocal->FS_Read( buf, len, f );
	buf[len] = 0;
	cgameLocal->FS_FCloseFile( f );

	COM_Compress( buf );

	Menu_Reset();

	p = buf;

	while ( 1 ) {
		token = COM_ParseExt( &p, true );
		if ( !token || token[0] == 0 || token[0] == '}' ) {
			break;
		}

		//if ( Q_stricmp( token, "{" ) ) {
		//	cgameLocal->Print( "Missing { in menu file\n" );
		//	break;
		//}

		//if ( menuCount == MAX_MENUS ) {
		//	cgameLocal->Print( "Too many menus!\n" );
		//	break;
		//}

		if ( Q_stricmp( token, "}" ) == 0 ) {
			break;
		}

		if ( Q_stricmp( token, "loadmenu" ) == 0 ) {
			if ( CG_Load_Menu( &p ) ) {
				continue;
			} else {
				break;
			}
		}
	}

	cgameLocal->Print( "UI menu load time = %d milli seconds\n", cgameLocal->Milliseconds() - start );

}


static bool CG_OwnerDrawHandleKey( int ownerDraw, int flags, float * special, int key ) {
	return false;
}


static int CG_FeederCount( float feederID ) {
	int i, count;
	count = 0;
	if ( feederID == FEEDER_REDTEAM_LIST ) {
		for ( i = 0; i < cg.numScores; i++ ) {
			if ( cg.scores[i].team == TEAM_RED ) {
				count++;
			}
		}
	} else if ( feederID == FEEDER_BLUETEAM_LIST ) {
		for ( i = 0; i < cg.numScores; i++ ) {
			if ( cg.scores[i].team == TEAM_BLUE ) {
				count++;
			}
		}
	} else if ( feederID == FEEDER_SCOREBOARD ) {
		return cg.numScores;
	}
	return count;
}


void CG_SetScoreSelection( void * p ) {
	menuDef_t * menu = ( menuDef_t * )p;
	playerState_t * ps = &cg.snap->ps;
	int i, red, blue;
	red = blue = 0;
	for ( i = 0; i < cg.numScores; i++ ) {
		if ( cg.scores[i].team == TEAM_RED ) {
			red++;
		} else if ( cg.scores[i].team == TEAM_BLUE ) {
			blue++;
		}
		if ( ps->clientNum == cg.scores[i].client ) {
			cg.selectedScore = i;
		}
	}

	if ( menu == NULL ) {
		// just interested in setting the selected score
		return;
	}

	if ( cgs.gametype >= GT_TEAM ) {
		int feeder = FEEDER_REDTEAM_LIST;
		i = red;
		if ( cg.scores[cg.selectedScore].team == TEAM_BLUE ) {
			feeder = FEEDER_BLUETEAM_LIST;
			i = blue;
		}
		Menu_SetFeederSelection( menu, feeder, i, NULL );
	} else {
		Menu_SetFeederSelection( menu, FEEDER_SCOREBOARD, cg.selectedScore, NULL );
	}
}

// FIXME: might need to cache this info
static clientInfo_t * CG_InfoFromScoreIndex( int index, int team, int * scoreIndex ) {
	int i, count;
	if ( cgs.gametype >= GT_TEAM ) {
		count = 0;
		for ( i = 0; i < cg.numScores; i++ ) {
			if ( cg.scores[i].team == team ) {
				if ( count == index ) {
					*scoreIndex = i;
					return &cgs.clientinfo[cg.scores[i].client];
				}
				count++;
			}
		}
	}
	*scoreIndex = index;
	return &cgs.clientinfo[ cg.scores[index].client ];
}

static const char * CG_FeederItemText( float feederID, int index, int column, qhandle_t * handle ) {
	gitem_t * item;
	int scoreIndex = 0;
	clientInfo_t * info = NULL;
	int team = -1;
	score_t * sp = NULL;

	*handle = -1;

	if ( feederID == FEEDER_REDTEAM_LIST ) {
		team = TEAM_RED;
	} else if ( feederID == FEEDER_BLUETEAM_LIST ) {
		team = TEAM_BLUE;
	}

	info = CG_InfoFromScoreIndex( index, team, &scoreIndex );
	sp = &cg.scores[scoreIndex];

	if ( info && info->infoValid ) {
		switch ( column ) {
			case 0:
				if ( info->powerups & ( 1 << PW_NEUTRALFLAG ) ) {
					item = BG_FindItemForPowerup( PW_NEUTRALFLAG );
					*handle = cg_items[ ITEM_INDEX( item ) ].icon;
				} else if ( info->powerups & ( 1 << PW_REDFLAG ) ) {
					item = BG_FindItemForPowerup( PW_REDFLAG );
					*handle = cg_items[ ITEM_INDEX( item ) ].icon;
				} else if ( info->powerups & ( 1 << PW_BLUEFLAG ) ) {
					item = BG_FindItemForPowerup( PW_BLUEFLAG );
					*handle = cg_items[ ITEM_INDEX( item ) ].icon;
				} else {
					if ( info->handicap < 100 ) {
						return va( "%i", info->handicap );
					}
				}
				break;
			case 1:
				if ( team == -1 ) {
					return "";
				} else {
					*handle = CG_StatusHandle( info->teamTask );
				}
				break;
			case 2:
				if ( cg.snap->ps.stats[ STAT_CLIENTS_READY ] & ( 1 << sp->client ) ) {
					return "Ready";
				}
				if ( team == -1 ) {
					if ( cgs.gametype == GT_TOURNAMENT ) {
						return va( "%i/%i", info->wins, info->losses );
					} else if ( info->infoValid && info->team == TEAM_SPECTATOR ) {
						return "Spectator";
					} else {
						return "";
					}
				} else {
					if ( info->teamLeader ) {
						return "Leader";
					}
				}
				break;
			case 3:
				return info->name;
				break;
			case 4:
				return va( "%i", info->score );
				break;
			case 5:
				return va( "%4i", sp->time );
				break;
			case 6:
				if ( sp->ping == -1 ) {
					return "connecting";
				}
				return va( "%4i", sp->ping );
				break;
		}
	}

	return "";
}

static qhandle_t CG_FeederItemImage( float feederID, int index ) {
	return 0;
}

static void CG_FeederSelection( float feederID, int index ) {
	if ( cgs.gametype >= GT_TEAM ) {
		int i, count;
		int team = ( feederID == FEEDER_REDTEAM_LIST ) ? TEAM_RED : TEAM_BLUE;
		count = 0;
		for ( i = 0; i < cg.numScores; i++ ) {
			if ( cg.scores[i].team == team ) {
				if ( index == count ) {
					cg.selectedScore = i;
				}
				count++;
			}
		}
	} else {
		cg.selectedScore = index;
	}
}
#endif

#ifdef MISSIONPACK // bk001204 - only needed there
static float CG_Cvar_Get( const char * cvar ) {
	char buff[128];
	memset( buff, 0, sizeof( buff ) );
	cgameLocal->Cvar_VariableStringBuffer( cvar, buff, sizeof( buff ) );
	return atof( buff );
}
#endif

#ifdef MISSIONPACK
void CG_Text_PaintWithCursor( float x, float y, float scale, vec4_t color, const char * text, int cursorPos, char cursor, int limit, int style ) {
	CG_Text_Paint( x, y, scale, color, text, 0, limit, style );
}

static int CG_OwnerDrawWidth( int ownerDraw, float scale ) {
	switch ( ownerDraw ) {
		case CG_GAME_TYPE:
			return CG_Text_Width( CG_GameTypeString(), scale, 0 );
		case CG_GAME_STATUS:
			return CG_Text_Width( CG_GetGameStatusText(), scale, 0 );
			break;
		case CG_KILLER:
			return CG_Text_Width( CG_GetKillerText(), scale, 0 );
			break;
		case CG_RED_NAME:
			return CG_Text_Width( cg_redTeamName.string, scale, 0 );
			break;
		case CG_BLUE_NAME:
			return CG_Text_Width( cg_blueTeamName.string, scale, 0 );
			break;


	}
	return 0;
}

static int CG_PlayCinematic( const char * name, float x, float y, float w, float h ) {
	return cgameLocal->CIN_PlayCinematic( name, x, y, w, h, CIN_loop );
}

static void CG_StopCinematic( int handle ) {
	cgameLocal->CIN_StopCinematic( handle );
}

static void CG_DrawCinematic( int handle, float x, float y, float w, float h ) {
	cgameLocal->CIN_SetExtents( handle, x, y, w, h );
	cgameLocal->CIN_DrawCinematic( handle );
}

static void CG_RunCinematicFrame( int handle ) {
	cgameLocal->CIN_RunCinematic( handle );
}

/*
=================
CG_LoadHudMenu();

=================
*/
void CG_LoadHudMenu() {
	char buff[1024];
	const char * hudSet;

	cgDC.registerShaderNoMip = &cgameLocal->R_RegisterShaderNoMip;
	cgDC.setColor = &cgameLocal->R_SetColor;
	cgDC.drawHandlePic = &CG_DrawPic;
	cgDC.drawStretchPic = &cgameLocal->R_DrawStretchPic;
	cgDC.drawText = &CG_Text_Paint;
	cgDC.textWidth = &CG_Text_Width;
	cgDC.textHeight = &CG_Text_Height;
	cgDC.registerModel = &cgameLocal->R_RegisterModel;
	cgDC.modelBounds = &cgameLocal->R_ModelBounds;
	cgDC.fillRect = &CG_FillRect;
	cgDC.drawRect = &CG_DrawRect;
	cgDC.drawSides = &CG_DrawSides;
	cgDC.drawTopBottom = &CG_DrawTopBottom;
	cgDC.clearScene = &cgameLocal->R_ClearScene;
	cgDC.addRefEntityToScene = &cgameLocal->R_AddRefEntityToScene;
	cgDC.renderScene = &cgameLocal->R_RenderScene;
	cgDC.registerFont = &cgameLocal->R_RegisterFont;
	cgDC.ownerDrawItem = &CG_OwnerDraw;
	cgDC.getValue = &CG_GetValue;
	cgDC.ownerDrawVisible = &CG_OwnerDrawVisible;
	cgDC.runScript = &CG_RunMenuScript;
	cgDC.getTeamColor = &CG_GetTeamColor;
	cgDC.setCVar = cgameLocal->Cvar_Set;
	cgDC.getCVarString = cgameLocal->Cvar_VariableStringBuffer;
	cgDC.getCVarValue = CG_Cvar_Get;
	cgDC.drawTextWithCursor = &CG_Text_PaintWithCursor;
	//cgDC.setOverstrikeMode = &cgameLocal->Key_SetOverstrikeMode;
	//cgDC.getOverstrikeMode = &cgameLocal->Key_GetOverstrikeMode;
	cgDC.startLocalSound = &cgameLocal->S_StartLocalSound;
	cgDC.ownerDrawHandleKey = &CG_OwnerDrawHandleKey;
	cgDC.feederCount = &CG_FeederCount;
	cgDC.feederItemImage = &CG_FeederItemImage;
	cgDC.feederItemText = &CG_FeederItemText;
	cgDC.feederSelection = &CG_FeederSelection;
	//cgDC.setBinding = &cgameLocal->Key_SetBinding;
	//cgDC.getBindingBuf = &cgameLocal->Key_GetBindingBuf;
	//cgDC.keynumToStringBuf = &cgameLocal->Key_KeynumToStringBuf;
	//cgDC.executeText = &cgameLocal->Cmd_ExecuteText;
	cgDC.Error = &Com_Error;
	cgDC.Print = &cgameLocal->Print;
	cgDC.ownerDrawWidth = &CG_OwnerDrawWidth;
	//cgDC.Pause = &CG_Pause;
	cgDC.registerSound = &cgameLocal->S_RegisterSound;
	cgDC.startBackgroundTrack = &cgameLocal->S_StartBackgroundTrack;
	cgDC.stopBackgroundTrack = &cgameLocal->S_StopBackgroundTrack;
	cgDC.playCinematic = &CG_PlayCinematic;
	cgDC.stopCinematic = &CG_StopCinematic;
	cgDC.drawCinematic = &CG_DrawCinematic;
	cgDC.runCinematicFrame = &CG_RunCinematicFrame;

	Init_Display( &cgDC );

	Menu_Reset();

	cgameLocal->Cvar_VariableStringBuffer( "cg_hudFiles", buff, sizeof( buff ) );
	hudSet = buff;
	if ( hudSet[0] == '\0' ) {
		hudSet = "ui/hud.txt";
	}

	CG_LoadMenus( hudSet );
}

void CG_AssetCache() {
	//if (Assets.textFont == NULL) {
	//  cgameLocal->R_RegisterFont("fonts/arial.ttf", 72, &Assets.textFont);
	//}
	//Assets.background = cgameLocal->R_RegisterShaderNoMip( ASSET_BACKGROUND );
	//cgameLocal->Print("Menu Size: %i bytes\n", sizeof(Menus));
	cgDC.Assets.gradientBar = cgameLocal->R_RegisterShaderNoMip( ASSET_GRADIENTBAR );
	cgDC.Assets.fxBasePic = cgameLocal->R_RegisterShaderNoMip( ART_FX_BASE );
	cgDC.Assets.fxPic[0] = cgameLocal->R_RegisterShaderNoMip( ART_FX_RED );
	cgDC.Assets.fxPic[1] = cgameLocal->R_RegisterShaderNoMip( ART_FX_YELLOW );
	cgDC.Assets.fxPic[2] = cgameLocal->R_RegisterShaderNoMip( ART_FX_GREEN );
	cgDC.Assets.fxPic[3] = cgameLocal->R_RegisterShaderNoMip( ART_FX_TEAL );
	cgDC.Assets.fxPic[4] = cgameLocal->R_RegisterShaderNoMip( ART_FX_BLUE );
	cgDC.Assets.fxPic[5] = cgameLocal->R_RegisterShaderNoMip( ART_FX_CYAN );
	cgDC.Assets.fxPic[6] = cgameLocal->R_RegisterShaderNoMip( ART_FX_WHITE );
	cgDC.Assets.scrollBar = cgameLocal->R_RegisterShaderNoMip( ASSET_SCROLLBAR );
	cgDC.Assets.scrollBarArrowDown = cgameLocal->R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
	cgDC.Assets.scrollBarArrowUp = cgameLocal->R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
	cgDC.Assets.scrollBarArrowLeft = cgameLocal->R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
	cgDC.Assets.scrollBarArrowRight = cgameLocal->R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
	cgDC.Assets.scrollBarThumb = cgameLocal->R_RegisterShaderNoMip( ASSET_SCROLL_THUMB );
	cgDC.Assets.sliderBar = cgameLocal->R_RegisterShaderNoMip( ASSET_SLIDER_BAR );
	cgDC.Assets.sliderThumb = cgameLocal->R_RegisterShaderNoMip( ASSET_SLIDER_THUMB );
}
#endif
/*
=================
CG_Init

Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum ) {
	const char	* s;

	// clear everything
	memset( &cgs, 0, sizeof( cgs ) );
	memset( &cg, 0, sizeof( cg ) );
	memset( cg_entities, 0, sizeof( cg_entities ) );
	memset( cg_weapons, 0, sizeof( cg_weapons ) );
	memset( cg_items, 0, sizeof( cg_items ) );

	cg.clientNum = clientNum;

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	// load a few needed things before we do any screen updates
	cgs.media.charsetShader		= cgameLocal->R_RegisterShader( "gfx/2d/bigchars" );
	cgs.media.whiteShader		= cgameLocal->R_RegisterShader( "white" );
	cgs.media.charsetProp		= cgameLocal->R_RegisterShaderNoMip( "menu/art/font1_prop.tga" );
	cgs.media.charsetPropGlow	= cgameLocal->R_RegisterShaderNoMip( "menu/art/font1_prop_glo.tga" );
	cgs.media.charsetPropB		= cgameLocal->R_RegisterShaderNoMip( "menu/art/font2_prop.tga" );

	CG_RegisterCvars();

	CG_InitConsoleCommands();

	cg.weaponSelect = WP_MACHINEGUN;

	cgs.redflag = cgs.blueflag = -1; // For compatibily, default to unset for
	cgs.flagStatus = -1;
	// old servers

	// get the rendering configuration from the client system
	cgameLocal->GetGlconfig( &cgs.glconfig );
	cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
	cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;

	// get the gamestate from the client system
	cgameLocal->GetGameState( &cgs.gameState );

	// check version
	s = CG_ConfigString( CS_GAME_VERSION );
	if ( strcmp( s, GAME_VERSION ) ) {
		CG_Error( "Client/Server game mismatch: %s/%s", GAME_VERSION, s );
	}

	s = CG_ConfigString( CS_LEVEL_START_TIME );
	cgs.levelStartTime = atoi( s );

	CG_ParseServerinfo();

	// load the new map
	CG_LoadingString( "collision map" );

	cgameLocal->CM_LoadMap( cgs.mapname );

#ifdef MISSIONPACK
	String_Init();
#endif

	cg.loading = true;		// force players to load instead of defer

	CG_LoadingString( "sounds" );

	CG_RegisterSounds();

	CG_LoadingString( "graphics" );

	CG_RegisterGraphics();

	CG_LoadingString( "clients" );

	CG_RegisterClients();		// if low on memory, some clients will be deferred

#ifdef MISSIONPACK
	CG_AssetCache();
	CG_LoadHudMenu();      // load new hud stuff
#endif

	cg.loading = false;	// future players will be deferred

	CG_InitLocalEntities();

	CG_InitMarkPolys();

	// remove the last loading update
	cg.infoScreenText[0] = 0;

	// Make sure we have update values (scores)
	CG_SetConfigValues();

	CG_StartMusic();

	CG_LoadingString( "" );

#ifdef MISSIONPACK
	CG_InitTeamChat();
#endif

	CG_ShaderStateChanged();

	cgameLocal->S_ClearLoopingSounds( true );
}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void ) {
	// some mods may need to do cleanup work here,
	// like closing files or archiving session data
}


/*
==================
CG_EventHandling
==================
 type 0 - no event handling
	  1 - team menu
	  2 - hud editor

*/
#ifndef MISSIONPACK
void CG_EventHandling( int type ) {
}


void CG_KeyEvent( int key, bool down ) {
}

void CG_MouseEvent( int x, int y ) {
}
#endif

