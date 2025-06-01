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
#include "ui_local.h"

uiImport_t	* uiLocal;
uiExport_t * ui;

uiExport_t * dllEntry( uiImport_t * imports ) {
	static uiExport_t exports;

	exports.UI_Init = UI_Init;
	exports.UI_Shutdown = UI_Shutdown;
	exports.UI_KeyEvent = UI_KeyEvent;
	exports.UI_MouseEvent = UI_MouseEvent;
	exports.UI_Refresh = UI_Refresh;
	exports.UI_IsFullscreen = UI_IsFullscreen;
	exports.UI_SetActiveMenu = UI_SetActiveMenu;
	exports.UI_ConsoleCommand = UI_ConsoleCommand;
	exports.UI_DrawConnectScreen = UI_DrawConnectScreen;

	uiLocal = imports;
	ui = &exports;

	return &exports;
}