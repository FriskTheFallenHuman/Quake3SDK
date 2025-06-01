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
// cg_syscalls.c
//

#include "cg_local.h"

cgameImport_t * cgameLocal;
cgameExport_t * cgame;

void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown( void );

cgameExport_t * dllEntry( cgameImport_t * imports ) {
	static cgameExport_t exports;

	exports.CG_Init = CG_Init;
	exports.CG_Shutdown = CG_Shutdown;
	exports.CG_ConsoleCommand = CG_ConsoleCommand;
	exports.CG_DrawActiveFrame = CG_DrawActiveFrame;
	exports.CG_CrosshairPlayer = CG_CrosshairPlayer;
	exports.CG_LastAttacker = CG_LastAttacker;
	exports.CG_KeyEvent = CG_KeyEvent;
	exports.CG_MouseEvent = CG_MouseEvent;
	exports.CG_EventHandling = CG_EventHandling;

	cgameLocal = imports;
	cgame = &exports;

	return &exports;
}