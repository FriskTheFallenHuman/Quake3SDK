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
#include "g_local.h"

gameImport_t * gameLocal;
gameExport_t * game;

void G_InitGame( int levelTime, int randomSeed, int restart );
void G_ShutdownGame( int restart );
void G_RunFrame( int levelTime );

gameExport_t * dllEntry( gameImport_t * gameAPI ) {
	static gameExport_t exports;

	exports.G_InitGame = G_InitGame;
	exports.G_ShutdownGame = G_ShutdownGame;
	exports.ClientConnect = ClientConnect;
	exports.ClientThink = ClientThink;
	exports.ClientUserinfoChanged = ClientUserinfoChanged;
	exports.ClientDisconnect = ClientDisconnect;
	exports.ClientBegin = ClientBegin;
	exports.ClientCommand = ClientCommand;
	exports.G_RunFrame = G_RunFrame;
	exports.ConsoleCommand = ConsoleCommand;

	gameLocal = gameAPI;
	game = &exports;

	return &exports;
}