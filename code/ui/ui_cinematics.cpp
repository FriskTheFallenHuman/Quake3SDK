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


#define ART_BACK0		"menu/art/back_0"
#define ART_BACK1		"menu/art/back_1"
#define ART_FRAMEL		"menu/art/frame2_l"
#define ART_FRAMER		"menu/art/frame1_r"

#define VERTICAL_SPACING	30

#define ID_BACK			10
#define ID_CIN_IDLOGO	11

typedef struct {
	menuframework_s	menu;
	menutext_s		banner;
	menubitmap_s	framel;
	menubitmap_s	framer;
	menutext_s		cin_idlogo;
	menubitmap_s	back;
} cinematicsMenuInfo_t;

static cinematicsMenuInfo_t	cinematicsMenuInfo;

static char * cinematics[] = {
	"idlogo",
};

/*
===============
UI_CinematicsMenu_BackEvent
===============
*/
static void UI_CinematicsMenu_BackEvent( void * ptr, int event ) {
	if ( event != QM_ACTIVATED ) {
		return;
	}
	UI_PopMenu();
}


/*
===============
UI_CinematicsMenu_Event
===============
*/
static void UI_CinematicsMenu_Event( void * ptr, int event ) {
	int		n;

	if ( event != QM_ACTIVATED ) {
		return;
	}

	n = ( ( menucommon_s * )ptr )->id - ID_CIN_IDLOGO;
	trap_Cvar_Set( "nextmap", va( "ui_cinematics %i", n ) );
	trap_Cmd_ExecuteText( EXEC_APPEND, va( "disconnect; cinematic %s.RoQ\n", cinematics[n] ) );
}


/*
===============
UI_CinematicsMenu_Init
===============
*/
static void UI_CinematicsMenu_Init( void ) {
	int		y;

	UI_CinematicsMenu_Cache();

	memset( &cinematicsMenuInfo, 0, sizeof( cinematicsMenuInfo ) );
	cinematicsMenuInfo.menu.fullscreen = qtrue;

	cinematicsMenuInfo.banner.generic.type		= MTYPE_BTEXT;
	cinematicsMenuInfo.banner.generic.x			= 320;
	cinematicsMenuInfo.banner.generic.y			= 16;
	cinematicsMenuInfo.banner.string			= "CINEMATICS";
	cinematicsMenuInfo.banner.color				= color_white;
	cinematicsMenuInfo.banner.style				= UI_CENTER;

	cinematicsMenuInfo.framel.generic.type		= MTYPE_BITMAP;
	cinematicsMenuInfo.framel.generic.name		= ART_FRAMEL;
	cinematicsMenuInfo.framel.generic.flags		= QMF_INACTIVE;
	cinematicsMenuInfo.framel.generic.x			= 0;
	cinematicsMenuInfo.framel.generic.y			= 78;
	cinematicsMenuInfo.framel.width  			= 256;
	cinematicsMenuInfo.framel.height  			= 329;

	cinematicsMenuInfo.framer.generic.type		= MTYPE_BITMAP;
	cinematicsMenuInfo.framer.generic.name		= ART_FRAMER;
	cinematicsMenuInfo.framer.generic.flags		= QMF_INACTIVE;
	cinematicsMenuInfo.framer.generic.x			= 376;
	cinematicsMenuInfo.framer.generic.y			= 76;
	cinematicsMenuInfo.framer.width  			= 256;
	cinematicsMenuInfo.framer.height  			= 334;

	y = 100;
	cinematicsMenuInfo.cin_idlogo.generic.type		= MTYPE_PTEXT;
	cinematicsMenuInfo.cin_idlogo.generic.flags		= QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
	cinematicsMenuInfo.cin_idlogo.generic.x			= 320;
	cinematicsMenuInfo.cin_idlogo.generic.y			= y;
	cinematicsMenuInfo.cin_idlogo.generic.id		= ID_CIN_IDLOGO;
	cinematicsMenuInfo.cin_idlogo.generic.callback	= UI_CinematicsMenu_Event;
	cinematicsMenuInfo.cin_idlogo.string			= "ID LOGO";
	cinematicsMenuInfo.cin_idlogo.color				= color_red;
	cinematicsMenuInfo.cin_idlogo.style				= UI_CENTER;

	cinematicsMenuInfo.back.generic.type		= MTYPE_BITMAP;
	cinematicsMenuInfo.back.generic.name		= ART_BACK0;
	cinematicsMenuInfo.back.generic.flags		= QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
	cinematicsMenuInfo.back.generic.id			= ID_BACK;
	cinematicsMenuInfo.back.generic.callback	= UI_CinematicsMenu_BackEvent;
	cinematicsMenuInfo.back.generic.x			= 0;
	cinematicsMenuInfo.back.generic.y			= 480 - 64;
	cinematicsMenuInfo.back.width				= 128;
	cinematicsMenuInfo.back.height				= 64;
	cinematicsMenuInfo.back.focuspic			= ART_BACK1;

	Menu_AddItem( &cinematicsMenuInfo.menu, &cinematicsMenuInfo.banner );
	Menu_AddItem( &cinematicsMenuInfo.menu, &cinematicsMenuInfo.framel );
	Menu_AddItem( &cinematicsMenuInfo.menu, &cinematicsMenuInfo.framer );
	Menu_AddItem( &cinematicsMenuInfo.menu, &cinematicsMenuInfo.cin_idlogo );
	Menu_AddItem( &cinematicsMenuInfo.menu, &cinematicsMenuInfo.back );
}


/*
=================
UI_CinematicsMenu_Cache
=================
*/
void UI_CinematicsMenu_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
	trap_R_RegisterShaderNoMip( ART_FRAMEL );
	trap_R_RegisterShaderNoMip( ART_FRAMER );
}


/*
===============
UI_CinematicsMenu
===============
*/
void UI_CinematicsMenu( void ) {
	UI_CinematicsMenu_Init();
	UI_PushMenu( &cinematicsMenuInfo.menu );
}


/*
===============
UI_CinematicsMenu_f
===============
*/
void UI_CinematicsMenu_f( void ) {
	int		n;

	n = atoi( UI_Argv( 1 ) );
	UI_CinematicsMenu();
	Menu_SetCursorToItem( &cinematicsMenuInfo.menu, cinematicsMenuInfo.menu.items[n + 3] );
}
