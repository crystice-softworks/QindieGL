/***************************************************************************
* Copyright (C) 2011-2016, Crystice Softworks.
* 
* This file is part of QindieGL source code.
* Please note that QindieGL is not driver, it's emulator.
* 
* QindieGL source code is free software; you can redistribute it and/or 
* modify it under the terms of the GNU General Public License as 
* published by the Free Software Foundation; either version 2 of 
* the License, or (at your option) any later version.
* 
* QindieGL source code is distributed in the hope that it will be 
* useful, but WITHOUT ANY WARRANTY; without even the implied 
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software 
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
***************************************************************************/
#include "d3d_wrapper.hpp"
#include "d3d_global.hpp"
#include "d3d_state.hpp"
#include "d3d_utils.hpp"
#include "d3d_matrix_stack.hpp"

//==================================================================================
// CPU features detect
//----------------------------------------------------------------------------------
// Used to detect SSE support
//==================================================================================

static bool cpuid( unsigned long function, unsigned long& out_eax, unsigned long& out_ebx, unsigned long& out_ecx, unsigned long& out_edx )
{
	bool retval = true;
	unsigned long local_eax, local_ebx, local_ecx, local_edx;
	_asm pushad;

	__try {
        _asm {
			xor edx, edx
            mov eax, function
            cpuid
            mov local_eax, eax
            mov local_ebx, ebx
            mov local_ecx, ecx
            mov local_edx, edx
		}
    } __except(EXCEPTION_EXECUTE_HANDLER) { 
		retval = false; 
	}

	out_eax = local_eax;
	out_ebx = local_ebx;
	out_ecx = local_ecx;
	out_edx = local_edx;
	_asm popad
	return retval;
}

void D3DGlobal_CPU_Detect()
{
	unsigned long p1, p2, p3, p4;
	char szVendorID[13];
	memset( szVendorID, 0, sizeof(szVendorID) );

	if( !cpuid(0, p1, p2, p3, p4 ) ) {
		strcpy_s( szVendorID, "Generic_x86" ); 
	} else {
		memcpy( szVendorID+0, &(p2), sizeof( p2 ) );
		memcpy( szVendorID+4, &(p4), sizeof( p4 ) );
		memcpy( szVendorID+8, &(p3), sizeof( p3 ) );
	}

	logPrintf("CPU: %s\n", szVendorID );

    if( !cpuid( 1, p1, p2, p3, p4 ) ) {
		logPrintf("Features: none\n" );
		D3DGlobal.settings.useSSE = false;
		return;
	}

	logPrintf("Features:" );

	if ( p4 & 0x00000010L )
		logPrintf(" RDTSC" );
	if ( p4 & 0x00800000L )
		logPrintf(" MMX" );
	if ( p4 & 0x02000000L )
		logPrintf(" SSE" );
	if ( p4 & 0x04000000L )
		logPrintf(" SSE2" );
	if ( p3 & 0x00000001L )
		logPrintf(" SSE3" );
	
	logPrintf("\n" );

	if (!( p4 & 0x02000000L )) {
		if (D3DGlobal.settings.useSSE) {
			logPrintf("WARNING: SSE is not supported, all SSE optimizations disabled\n" );
			D3DGlobal.settings.useSSE = false;
		}
	}

	if (D3DGlobal.settings.useSSE) {
		logPrintf("Using SSE optimizations\n" );
	}
}