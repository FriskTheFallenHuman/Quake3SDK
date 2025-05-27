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

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

//#define MEMDEBUG

#define MEM_ID		0x12345678l
#define HUNK_ID		0x87654321l

int allocatedmemory;
int totalmemorysize;
int numblocks;

/*
==================
Mem_GetMemory
==================
*/
void * Mem_GetMemory( int size ) {
	void * ptr;

	ptr = Z_TagMalloc( size, TAG_UTILS );
	return ptr;
}

/*
=================
Mem_HunkAlloc
=================
*/
void * Mem_HunkAlloc( int size ) {
	if ( Hunk_CheckMark() ) {
		Com_Error( ERR_DROP, "SV_Bot_HunkAlloc: Alloc with marks already set\n" );
	}
	return Hunk_Alloc( size, h_high );
}

/*
=================
GetMemory/GetMemoryDebug
=================
*/
#ifdef MEMDEBUG
void * GetMemoryDebug( unsigned long size, char * label, char * file, int line )
#else
void * GetMemory( unsigned long size )
#endif //MEMDEBUG
{
	void * ptr;
	unsigned long int * memid;

	ptr = Mem_GetMemory( size + sizeof( unsigned long int ) );
	if ( !ptr ) {
		return NULL;
	}
	memid = ( unsigned long int * ) ptr;
	*memid = MEM_ID;
	return ( unsigned long int * )( ( char * ) ptr + sizeof( unsigned long int ) );
}

/*
=================
GetClearedMemory/GetClearedMemoryDebug
=================
*/
#ifdef MEMDEBUG
void * GetClearedMemoryDebug( unsigned long size, char * label, char * file, int line )
#else
void * GetClearedMemory( unsigned long size )
#endif //MEMDEBUG
{
	void * ptr;
#ifdef MEMDEBUG
	ptr = GetMemoryDebug( size, label, file, line );
#else
	ptr = GetMemory( size );
#endif //MEMDEBUG
	Com_Memset( ptr, 0, size );
	return ptr;
}

/*
=================
GetHunkMemoryDebug/GetHunkMemory
=================
*/
#ifdef MEMDEBUG
void * GetHunkMemoryDebug( unsigned long size, char * label, char * file, int line )
#else
void * GetHunkMemory( unsigned long size )
#endif //MEMDEBUG
{
	void * ptr;
	unsigned long int * memid;

	ptr = Mem_HunkAlloc( size + sizeof( unsigned long int ) );
	if ( !ptr ) {
		return NULL;
	}
	memid = ( unsigned long int * ) ptr;
	*memid = HUNK_ID;
	return ( unsigned long int * )( ( char * ) ptr + sizeof( unsigned long int ) );
}

/*
=================
GetClearedHunkMemoryDebug/GetClearedHunkMemory
=================
*/
#ifdef MEMDEBUG
void * GetClearedHunkMemoryDebug( unsigned long size, char * label, char * file, int line )
#else
void * GetClearedHunkMemory( unsigned long size )
#endif //MEMDEBUG
{
	void * ptr;
#ifdef MEMDEBUG
	ptr = GetHunkMemoryDebug( size, label, file, line );
#else
	ptr = GetHunkMemory( size );
#endif //MEMDEBUG
	Com_Memset( ptr, 0, size );
	return ptr;
}

/*
=================
FreeMemory
=================
*/
void FreeMemory( void * ptr ) {
	unsigned long int * memid;

	memid = ( unsigned long int * )( ( char * ) ptr - sizeof( unsigned long int ) );

	if ( *memid == MEM_ID ) {
		Z_Free( memid );
	}
}

/*
=================
AvailableMemory
=================
*/
int AvailableMemory( void ) {
	return Z_AvailableMemory();
}

/*
=================
PrintUsedMemorySize
=================
*/
void PrintUsedMemorySize( void ) {
}

/*
=================
PrintMemoryLabels
=================
*/
void PrintMemoryLabels( void ) {
}