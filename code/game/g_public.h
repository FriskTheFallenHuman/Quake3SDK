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

#ifndef __G_PUBLIC_H__
#define __G_PUBLIC_H__

// g_public.h -- game module information visible to server

#define	GAME_API_VERSION	8

// entity->svFlags
// the server does not know how to interpret most of the values
// in entityStates (level eType), so the game must explicitly flag
// special server behaviors
#define	SVF_NOCLIENT			0x00000001	// don't send entity to clients, even if it has effects

// TTimo
// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=551
#define SVF_CLIENTMASK 0x00000002

#define SVF_UNUSED				0x00000008	// unused server flag
#define	SVF_BROADCAST			0x00000020	// send to all connected clients
#define	SVF_PORTAL				0x00000040	// merge a second pvs at origin2 into snapshots
#define	SVF_USE_CURRENT_ORIGIN	0x00000080	// entity->r.currentOrigin instead of entity->s.origin
// for link position (missiles and movers)
#define SVF_SINGLECLIENT		0x00000100	// only send to a single client (entityShared_t->singleClient)
#define SVF_NOSERVERINFO		0x00000200	// don't send CS_SERVERINFO updates to this client
// so that it can be updated for ping tools without
// lagging clients
#define SVF_CAPSULE				0x00000400	// use capsule for collision detection instead of bbox
#define SVF_NOTSINGLECLIENT		0x00000800	// send entity to everyone but one client
// (entityShared_t->singleClient)



//===============================================================


typedef struct {
	entityState_t	s;				// communicated by server to clients

	qboolean	linked;				// qfalse if not in any good cluster
	int			linkcount;

	int			svFlags;			// SVF_NOCLIENT, SVF_BROADCAST, etc

	// only send to this client when SVF_SINGLECLIENT is set
	// if SVF_CLIENTMASK is set, use bitmask for clients to send to (maxclients must be <= 32, up to the mod to enforce this)
	int			singleClient;

	qboolean	bmodel;				// if false, assume an explicit mins / maxs bounding box
	// only set by trap_SetBrushModel
	vec3_t		mins, maxs;
	int			contents;			// CONTENTS_TRIGGER, CONTENTS_SOLID, CONTENTS_BODY, etc
	// a non-solid entity should set to 0

	vec3_t		absmin, absmax;		// derived from mins/maxs and origin + rotation

	// currentOrigin will be used for all collision detection and world linking.
	// it will not necessarily be the same as the trajectory evaluation for the current
	// time, because each entity must be moved one at a time after time is advanced
	// to avoid simultanious collision issues
	vec3_t		currentOrigin;
	vec3_t		currentAngles;

	// when a trace call is made and passEntityNum != ENTITYNUM_NONE,
	// an ent will be excluded from testing if:
	// ent->s.number == passEntityNum	(don't interact with self)
	// ent->s.ownerNum = passEntityNum	(don't interact with your own missiles)
	// entity[ent->s.ownerNum].ownerNum = passEntityNum	(don't interact with other missiles from owner)
	int			ownerNum;
} entityShared_t;



// the server looks at a sharedEntity, which is the start of the game's gentity_t structure
typedef struct {
	entityState_t	s;				// communicated by server to clients
	entityShared_t	r;				// shared by both the server system and game
} sharedEntity_t;



//===============================================================

//
// system traps provided by the main engine
//
typedef struct {
	void ( *Print )( const char *, ... );
	void ( *Error )( int, const char *, ... );
	int ( *Milliseconds )( void );
	void ( *Cvar_Register )( vmCvar_t *, const char *, const char *, int );
	void ( *Cvar_Update )( vmCvar_t * );
	void ( *Cvar_Set )( const char *, const char * );
	int ( *Cvar_VariableIntegerValue )( const char * var_name );
	void ( *Cvar_VariableStringBuffer )( const char * var_name, char * buffer, int bufsize );
	int ( *Argc )( void );
	void ( *Argv )( int n, char * buffer, int bufferLength );
	void ( *SendConsoleCommand )( int exec_when, const char * text );
	int ( *FS_FOpenFile )( const char * qpath, fileHandle_t * f, fsMode_t mode );
	int ( *FS_Read )( void *, int, int );
	int ( *FS_Write )( const void *, int, int );
	void ( *FS_FCloseFile )( int f );
	int ( *FS_GetFileList )( const char * path, const char * extension, char * listbuf, int bufsize );
	int ( *FS_Seek )( int f, long offset, int origin );
	void ( *LocateGameData )( sharedEntity_t *, int, int sizeofGEntity_t, playerState_t *, int );
	void ( *DropClient )( int clientNum, const char * reason );
	void ( *SendServerCommand )( int clientNum, const char * text );
	void ( *SetConfigstring )( int num, const char * string );
	void ( *GetConfigstring )( int num, char * buffer, int bufferSize );
	void ( *SetUserinfo )( int num, const char * buffer );
	void ( *GetUserinfo )( int num, char * buffer, int bufferSize );
	void ( *GetServerinfo )( char * buffer, int bufferSize );
	void ( *SetBrushModel )( sharedEntity_t *, const char * );
	void ( *Trace )( trace_t *, const vec3_t, vec3_t, vec3_t, const vec3_t, int, int, int );
	int ( *PointContents )( const vec3_t, int );
	qboolean ( *inPVS )( const vec3_t, const vec3_t );
	qboolean ( *inPVSIgnorePortals )( const vec3_t, const vec3_t );
	void ( *AdjustAreaPortalState )( sharedEntity_t *, qboolean );
	qboolean ( *AreasConnected )( int, int );
	void ( *LinkEntity )( sharedEntity_t * );
	void ( *UnlinkEntity )( sharedEntity_t * );
	int ( *EntitiesInBox )( const vec3_t, const vec3_t, int *, int );
	qboolean ( *EntityContact )( vec3_t, vec3_t, const sharedEntity_t *, int );
	void ( *GetUsercmd )( int, usercmd_t * );
	int ( *GetEntityToken )( char *, int );
	int ( *RealTime )( qtime_t * );
} gameImport_t;


//
// functions exported by the game subsystem
//
typedef struct {
	void ( *G_InitGame )( int, int, int );
	void ( *G_ShutdownGame )( int );
	char * ( *ClientConnect )( int, qboolean );
	void ( *ClientThink )( int );
	void ( *ClientUserinfoChanged )( int );
	void ( *ClientDisconnect )( int );
	void ( *ClientBegin )( int );
	void ( *ClientCommand )( int );
	void ( *G_RunFrame )( int );
	qboolean ( *ConsoleCommand )( void );
} gameExport_t;

extern gameExport_t * game;

#endif /*!__G_PUBLIC_H__ */