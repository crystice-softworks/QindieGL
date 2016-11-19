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
#include "d3d_immediate.hpp"
#include "d3d_array.hpp"
#include "d3d_object.hpp"
#include "d3d_extension.hpp"
#include "d3d_texture.hpp"
#include "d3d_matrix_stack.hpp"

//==================================================================================
// D3D Global
//----------------------------------------------------------------------------------
// Initialize and set up global D3D vars
//==================================================================================

D3DGlobal_t D3DGlobal;

void D3DGlobal_Init( bool clearGlobals )
{
	logPrintf("--- Init( %s ) ---\n", clearGlobals ? "clear globals" : "normal" );

	if (clearGlobals) {
		memset( &D3DGlobal, 0, sizeof(D3DGlobal) );
	} else {
		D3DGlobal.initialized = false;
		D3DGlobal.sceneBegan = false;
		D3DGlobal.deviceLost = false;
		D3DGlobal.skipCopyImage = 5;
		D3DGlobal.lastError = S_OK;
	}

	if (!D3DGlobal.pIMBuffer)
		D3DGlobal.pIMBuffer = new D3DIMBuffer;
	if (!D3DGlobal.pVABuffer)
		D3DGlobal.pVABuffer = new D3DVABuffer;

	if (!D3DGlobal.pObjectBuffer)
		D3DGlobal.pObjectBuffer = new D3DObjectBuffer;

	for (int i = 0; i < D3D_TEXTARGET_MAX; ++i) {
		if (!D3DGlobal.defaultTexture[i])
			D3DGlobal.defaultTexture[i] = new D3DTextureObject(0);
	}
}

void D3DGlobal_Reset()
{
	D3DGlobal.skipCopyImage = 5;

	if (D3DGlobal.pIMBuffer) {
		delete D3DGlobal.pIMBuffer;
		D3DGlobal.pIMBuffer = nullptr;
	}
	if (D3DGlobal.pVABuffer) {
		delete D3DGlobal.pVABuffer;
		D3DGlobal.pVABuffer = nullptr;
	}
	if (D3DGlobal.pSystemMemRT) {
		D3DGlobal.pSystemMemRT->Release();
		D3DGlobal.pSystemMemRT = nullptr;
	}
	if (D3DGlobal.pSystemMemFB) {
		D3DGlobal.pSystemMemFB->Release();
		D3DGlobal.pSystemMemFB = nullptr;
	}

	Sleep( 20 );
	D3DGlobal.pDevice->Reset(&D3DGlobal.hPresentParams);
	Sleep( 20 );

	D3DGlobal.pIMBuffer = new D3DIMBuffer;
	D3DGlobal.pVABuffer = new D3DVABuffer;
}

void D3DGlobal_Cleanup( bool cleanupAll )
{
	logPrintf("--- Cleanup( %s ) ---\n", cleanupAll ? "all" : "partial" );

	UTIL_FreeString(D3DGlobal.szWExtensions);
	D3DGlobal.szWExtensions = nullptr;
	UTIL_FreeString(D3DGlobal.szExtensions);
	D3DGlobal.szExtensions = nullptr;
	UTIL_FreeString(D3DGlobal.szRendererName);
	D3DGlobal.szRendererName = nullptr;

	for (int i = 0; i < D3D_TEXTARGET_MAX; ++i) {
		if (D3DGlobal.defaultTexture[i]) {
			delete D3DGlobal.defaultTexture[i];
			D3DGlobal.defaultTexture[i] = nullptr;
		}
	}

	if (D3DGlobal.compiledVertexArray.compiledVertexData) {
		UTIL_Free(D3DGlobal.compiledVertexArray.compiledVertexData);
		D3DGlobal.compiledVertexArray.compiledVertexData = nullptr;
		D3DGlobal.compiledVertexArray.compiledVertexDataSize = 0;
	}
	if (D3DGlobal.compiledVertexArray.compiledNormalData) {
		UTIL_Free(D3DGlobal.compiledVertexArray.compiledNormalData);
		D3DGlobal.compiledVertexArray.compiledNormalData = nullptr;
		D3DGlobal.compiledVertexArray.compiledNormalDataSize = 0;
	}
	if (D3DGlobal.compiledVertexArray.compiledColorData) {
		UTIL_Free(D3DGlobal.compiledVertexArray.compiledColorData);
		D3DGlobal.compiledVertexArray.compiledColorData = nullptr;
		D3DGlobal.compiledVertexArray.compiledColorDataSize = 0;
	}
	for (int i = 0; i < MAX_D3D_TMU; ++i) {
		if (D3DGlobal.compiledVertexArray.compiledTexCoordData[i]) {
			UTIL_Free(D3DGlobal.compiledVertexArray.compiledTexCoordData[i]);
			D3DGlobal.compiledVertexArray.compiledTexCoordData[i] = nullptr;
			D3DGlobal.compiledVertexArray.compiledTexCoordDataSize[i] = 0;
		}
	}

	if (D3DGlobal.modelviewMatrixStack) {
		delete D3DGlobal.modelviewMatrixStack;
		D3DGlobal.modelviewMatrixStack = nullptr;
	}
	if (D3DGlobal.projectionMatrixStack) {
		delete D3DGlobal.projectionMatrixStack;
		D3DGlobal.projectionMatrixStack = nullptr;
	}
	for (int i = 0; i < MAX_D3D_TMU; ++i) {
		if (D3DGlobal.textureMatrixStack[i]) {
			delete D3DGlobal.textureMatrixStack[i];
			D3DGlobal.textureMatrixStack[i] = nullptr;
		}
	}
	if (D3DGlobal.pObjectBuffer) {
		delete D3DGlobal.pObjectBuffer;
		D3DGlobal.pObjectBuffer = nullptr;
	}
	if (D3DGlobal.pIMBuffer) {
		delete D3DGlobal.pIMBuffer;
		D3DGlobal.pIMBuffer = nullptr;
	}
	if (D3DGlobal.pVABuffer) {
		delete D3DGlobal.pVABuffer;
		D3DGlobal.pVABuffer = nullptr;
	}

	if (D3DGlobal.pSystemMemRT) {
		D3DGlobal.pSystemMemRT->Release();
		D3DGlobal.pSystemMemRT = nullptr;
	}
	if (D3DGlobal.pSystemMemFB) {
		D3DGlobal.pSystemMemFB->Release();
		D3DGlobal.pSystemMemFB = nullptr;
	}

	if (D3DGlobal.pSwapChain) {
		D3DGlobal.pSwapChain->Release();
		D3DGlobal.pSwapChain = nullptr;
	}
	if (D3DGlobal.pDevice) {
		D3DGlobal.pDevice->Release();
		D3DGlobal.pDevice = nullptr;
	}

	if (cleanupAll) {
		// some invalid parms may cause D3D to be corrupted
		// and crash upon release; so don't release.
		/*
		if (D3DGlobal.pD3D) {
			D3DGlobal.pD3D->Release();
			D3DGlobal.pD3D = nullptr;
		}
		if (D3DGlobal.hD3DDll) {
			FreeLibrary(D3DGlobal.hD3DDll);
			D3DGlobal.hD3DDll = nullptr;
		}
		*/
	}
}

/*
==================
D3DGlobal_GetRegistryValue

Returns a dword value for registry variable
==================
*/
DWORD D3DGlobal_GetRegistryValue( const char *key, const char *section, DWORD defaultValue )
{
	DWORD dwValue;
    HKEY hKey;
    LONG returnStatus;
	DWORD dwType = REG_DWORD;
	DWORD dwSize = sizeof(DWORD);
	char szKeyPath[MAX_PATH];

	strncpy_s(szKeyPath, WRAPPER_REGISTRY_PATH "\\", MAX_PATH-1);
	strncat_s(szKeyPath, section, MAX_PATH-1);

    returnStatus = RegOpenKeyEx(HKEY_CURRENT_USER, szKeyPath, 0L, KEY_READ, &hKey);

	if (returnStatus == ERROR_SUCCESS) {
		returnStatus = RegQueryValueEx(hKey, key, nullptr, &dwType, (LPBYTE)&dwValue, &dwSize);
		if (returnStatus == ERROR_SUCCESS) {
			RegCloseKey(hKey);
			return dwValue;
		} else {
			RegCloseKey(hKey);
			logPrintf("D3DGlobal_GetRegistryValue: RegQueryValueEx(\"%s\\%s\") failed (error %u)\n", szKeyPath, key, returnStatus);
		}
	} else {
		logPrintf("D3DGlobal_GetRegistryValue: RegOpenKeyEx(\"%s\") failed (error %u)\n", szKeyPath, returnStatus);
	}

	return defaultValue;
}

/*
==================
D3DGlobal_FormatToString

Returns a string name for given format
==================
*/
const char* D3DGlobal_FormatToString( D3DFORMAT format )
{
    const char* pstr = nullptr;
    switch( format )
    {
    case D3DFMT_UNKNOWN:         pstr = ("D3DFMT_UNKNOWN"); break;
    case D3DFMT_R8G8B8:          pstr = ("D3DFMT_R8G8B8"); break;
    case D3DFMT_A8R8G8B8:        pstr = ("D3DFMT_A8R8G8B8"); break;
    case D3DFMT_X8R8G8B8:        pstr = ("D3DFMT_X8R8G8B8"); break;
    case D3DFMT_R5G6B5:          pstr = ("D3DFMT_R5G6B5"); break;
    case D3DFMT_X1R5G5B5:        pstr = ("D3DFMT_X1R5G5B5"); break;
    case D3DFMT_A1R5G5B5:        pstr = ("D3DFMT_A1R5G5B5"); break;
    case D3DFMT_A4R4G4B4:        pstr = ("D3DFMT_A4R4G4B4"); break;
    case D3DFMT_R3G3B2:          pstr = ("D3DFMT_R3G3B2"); break;
    case D3DFMT_A8:              pstr = ("D3DFMT_A8"); break;
    case D3DFMT_A8R3G3B2:        pstr = ("D3DFMT_A8R3G3B2"); break;
    case D3DFMT_X4R4G4B4:        pstr = ("D3DFMT_X4R4G4B4"); break;
    case D3DFMT_A2B10G10R10:     pstr = ("D3DFMT_A2B10G10R10"); break;
    case D3DFMT_A8B8G8R8:        pstr = ("D3DFMT_A8B8G8R8"); break;
    case D3DFMT_X8B8G8R8:        pstr = ("D3DFMT_X8B8G8R8"); break;
    case D3DFMT_G16R16:          pstr = ("D3DFMT_G16R16"); break;
    case D3DFMT_A2R10G10B10:     pstr = ("D3DFMT_A2R10G10B10"); break;
    case D3DFMT_A16B16G16R16:    pstr = ("D3DFMT_A16B16G16R16"); break;
    case D3DFMT_A8P8:            pstr = ("D3DFMT_A8P8"); break;
    case D3DFMT_P8:              pstr = ("D3DFMT_P8"); break;
    case D3DFMT_L8:              pstr = ("D3DFMT_L8"); break;
    case D3DFMT_A8L8:            pstr = ("D3DFMT_A8L8"); break;
    case D3DFMT_A4L4:            pstr = ("D3DFMT_A4L4"); break;
    case D3DFMT_V8U8:            pstr = ("D3DFMT_V8U8"); break;
    case D3DFMT_L6V5U5:          pstr = ("D3DFMT_L6V5U5"); break;
    case D3DFMT_X8L8V8U8:        pstr = ("D3DFMT_X8L8V8U8"); break;
    case D3DFMT_Q8W8V8U8:        pstr = ("D3DFMT_Q8W8V8U8"); break;
    case D3DFMT_V16U16:          pstr = ("D3DFMT_V16U16"); break;
    case D3DFMT_A2W10V10U10:     pstr = ("D3DFMT_A2W10V10U10"); break;
    case D3DFMT_UYVY:            pstr = ("D3DFMT_UYVY"); break;
    case D3DFMT_YUY2:            pstr = ("D3DFMT_YUY2"); break;
    case D3DFMT_DXT1:            pstr = ("D3DFMT_DXT1"); break;
    case D3DFMT_DXT2:            pstr = ("D3DFMT_DXT2"); break;
    case D3DFMT_DXT3:            pstr = ("D3DFMT_DXT3"); break;
    case D3DFMT_DXT4:            pstr = ("D3DFMT_DXT4"); break;
    case D3DFMT_DXT5:            pstr = ("D3DFMT_DXT5"); break;
    case D3DFMT_D16_LOCKABLE:    pstr = ("D3DFMT_D16_LOCKABLE"); break;
	case D3DFMT_D32_LOCKABLE:    pstr = ("D3DFMT_D32_LOCKABLE"); break;
    case D3DFMT_D32:             pstr = ("D3DFMT_D32"); break;
    case D3DFMT_D15S1:           pstr = ("D3DFMT_D15S1"); break;
    case D3DFMT_D24S8:           pstr = ("D3DFMT_D24S8"); break;
    case D3DFMT_D24X8:           pstr = ("D3DFMT_D24X8"); break;
    case D3DFMT_D24X4S4:         pstr = ("D3DFMT_D24X4S4"); break;
    case D3DFMT_D16:             pstr = ("D3DFMT_D16"); break;
    case D3DFMT_L16:             pstr = ("D3DFMT_L16"); break;
    case D3DFMT_VERTEXDATA:      pstr = ("D3DFMT_VERTEXDATA"); break;
    case D3DFMT_INDEX16:         pstr = ("D3DFMT_INDEX16"); break;
    case D3DFMT_INDEX32:         pstr = ("D3DFMT_INDEX32"); break;
    case D3DFMT_Q16W16V16U16:    pstr = ("D3DFMT_Q16W16V16U16"); break;
    case D3DFMT_MULTI2_ARGB8:    pstr = ("D3DFMT_MULTI2_ARGB8"); break;
    case D3DFMT_R16F:            pstr = ("D3DFMT_R16F"); break;
    case D3DFMT_G16R16F:         pstr = ("D3DFMT_G16R16F"); break;
    case D3DFMT_A16B16G16R16F:   pstr = ("D3DFMT_A16B16G16R16F"); break;
    case D3DFMT_R32F:            pstr = ("D3DFMT_R32F"); break;
    case D3DFMT_G32R32F:         pstr = ("D3DFMT_G32R32F"); break;
    case D3DFMT_A32B32G32R32F:   pstr = ("D3DFMT_A32B32G32R32F"); break;
    case D3DFMT_CxV8U8:          pstr = ("D3DFMT_CxV8U8"); break;
    default:                     pstr = ("Unknown format"); break;
    }
 
	if( !strstr( pstr, "D3DFMT_" ) )
        return pstr;
    else
		return pstr + strlen( "D3DFMT_" );
}

/*
========================
D3DGlobal_SetupPixelFormatBits

Assigns context format values
They may be queried with glGet
========================
*/
static void D3DGlobal_SetupPixelFormatBits( D3DFORMAT fmtColor, D3DFORMAT fmtDepth )
{
	switch( fmtColor )
    {
    case D3DFMT_R8G8B8:			D3DGlobal.rgbaBits[0] = 8; D3DGlobal.rgbaBits[1] = 8; D3DGlobal.rgbaBits[2] = 8; D3DGlobal.rgbaBits[3] = 0; break;
	case D3DFMT_A8R8G8B8:		D3DGlobal.rgbaBits[0] = 8; D3DGlobal.rgbaBits[1] = 8; D3DGlobal.rgbaBits[2] = 8; D3DGlobal.rgbaBits[3] = 8; break;
	case D3DFMT_X8R8G8B8:		D3DGlobal.rgbaBits[0] = 8; D3DGlobal.rgbaBits[1] = 8; D3DGlobal.rgbaBits[2] = 8; D3DGlobal.rgbaBits[3] = 0; break;
	case D3DFMT_R5G6B5:			D3DGlobal.rgbaBits[0] = 5; D3DGlobal.rgbaBits[1] = 6; D3DGlobal.rgbaBits[2] = 5; D3DGlobal.rgbaBits[3] = 0; break;
	case D3DFMT_X1R5G5B5:		D3DGlobal.rgbaBits[0] = 5; D3DGlobal.rgbaBits[1] = 5; D3DGlobal.rgbaBits[2] = 5; D3DGlobal.rgbaBits[3] = 0; break;
	case D3DFMT_A1R5G5B5:		D3DGlobal.rgbaBits[0] = 5; D3DGlobal.rgbaBits[1] = 5; D3DGlobal.rgbaBits[2] = 5; D3DGlobal.rgbaBits[3] = 1; break;
	case D3DFMT_X4R4G4B4:		D3DGlobal.rgbaBits[0] = 4; D3DGlobal.rgbaBits[1] = 4; D3DGlobal.rgbaBits[2] = 4; D3DGlobal.rgbaBits[3] = 0; break;
	case D3DFMT_A4R4G4B4:		D3DGlobal.rgbaBits[0] = 4; D3DGlobal.rgbaBits[1] = 4; D3DGlobal.rgbaBits[2] = 4; D3DGlobal.rgbaBits[3] = 4; break;
	case D3DFMT_R3G3B2:			D3DGlobal.rgbaBits[0] = 3; D3DGlobal.rgbaBits[1] = 3; D3DGlobal.rgbaBits[2] = 2; D3DGlobal.rgbaBits[3] = 0; break;
	case D3DFMT_A8R3G3B2:		D3DGlobal.rgbaBits[0] = 3; D3DGlobal.rgbaBits[1] = 3; D3DGlobal.rgbaBits[2] = 2; D3DGlobal.rgbaBits[3] = 8; break;
	case D3DFMT_A2B10G10R10:	D3DGlobal.rgbaBits[0] = 10; D3DGlobal.rgbaBits[1] = 10; D3DGlobal.rgbaBits[2] = 10; D3DGlobal.rgbaBits[3] = 2; break;
	case D3DFMT_A2R10G10B10:	D3DGlobal.rgbaBits[0] = 10; D3DGlobal.rgbaBits[1] = 10; D3DGlobal.rgbaBits[2] = 10; D3DGlobal.rgbaBits[3] = 2; break;
	case D3DFMT_A8B8G8R8:		D3DGlobal.rgbaBits[0] = 8; D3DGlobal.rgbaBits[1] = 8; D3DGlobal.rgbaBits[2] = 8; D3DGlobal.rgbaBits[3] = 8; break;
	case D3DFMT_X8B8G8R8:		D3DGlobal.rgbaBits[0] = 8; D3DGlobal.rgbaBits[1] = 8; D3DGlobal.rgbaBits[2] = 8; D3DGlobal.rgbaBits[3] = 0; break;
	case D3DFMT_A16B16G16R16:	D3DGlobal.rgbaBits[0] = 16; D3DGlobal.rgbaBits[1] = 16; D3DGlobal.rgbaBits[2] = 16; D3DGlobal.rgbaBits[3] = 16; break;
	default: 
		logPrintf("WARNING: funny backbuffer color format 0x%x\n", fmtColor);
		D3DGlobal.rgbaBits[0] = 8; D3DGlobal.rgbaBits[1] = 8; D3DGlobal.rgbaBits[2] = 8; D3DGlobal.rgbaBits[3] = 0; break;
	}

	switch( fmtDepth )
    {
    case D3DFMT_D16_LOCKABLE:	D3DGlobal.depthBits = 16; D3DGlobal.stencilBits = 0; break;
	case D3DFMT_D32_LOCKABLE:	D3DGlobal.depthBits = 32; D3DGlobal.stencilBits = 0; break;
	case D3DFMT_D32:			D3DGlobal.depthBits = 32; D3DGlobal.stencilBits = 0; break;
	case D3DFMT_D15S1:			D3DGlobal.depthBits = 15; D3DGlobal.stencilBits = 1; break;
	case D3DFMT_D24S8:			D3DGlobal.depthBits = 24; D3DGlobal.stencilBits = 8; break;
	case D3DFMT_D24X8:			D3DGlobal.depthBits = 24; D3DGlobal.stencilBits = 0; break;
	case D3DFMT_D24X4S4:		D3DGlobal.depthBits = 24; D3DGlobal.stencilBits = 4; break;
	case D3DFMT_D16:			D3DGlobal.depthBits = 16; D3DGlobal.stencilBits = 0; break;
	default: 
		logPrintf("WARNING: funny depthstencil format 0x%x\n", fmtDepth);
		D3DGlobal.depthBits = 16; D3DGlobal.stencilBits = 0; break;
	}

	logPrintf("Color bits: %i %i %i %i\n", D3DGlobal.rgbaBits[0], D3DGlobal.rgbaBits[1], D3DGlobal.rgbaBits[2], D3DGlobal.rgbaBits[3]);
	logPrintf("Depth bits: %i\n", D3DGlobal.depthBits);
	logPrintf("Stencil bits: %i\n", D3DGlobal.stencilBits);
}

/*
==================
D3DGlobal_GetDepthFormat

Gets a valid depth format for a given adapter format
==================
*/
static D3DFORMAT D3DGlobal_GetDepthFormat( D3DFORMAT AdapterFormat )
{
	// valid depth formats
	// first try to create formats with depthstencil buffer, then depth-only
	const D3DFORMAT d3d_DepthFormats[] = {D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_D32, D3DFMT_D24X8, D3DFMT_D16, D3DFMT_UNKNOWN};

	for (int i = 0; ; ++i)
	{
		if (d3d_DepthFormats[i] == D3DFMT_UNKNOWN)
			break;

		HRESULT hr = D3DGlobal.pD3D->CheckDeviceFormat(	D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, AdapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, d3d_DepthFormats[i] );

		logPrintf("GetDepthFormat: format %s is %ssupported\n", D3DGlobal_FormatToString(d3d_DepthFormats[i]), FAILED(hr) ? "not " : "");

		// return the first format that succeeds
		if (SUCCEEDED(hr)) 
			return d3d_DepthFormats[i];
	}

	// didn't get one
	logPrintf("GetDepthFormat: no valid depth format found\n");
	return D3DFMT_UNKNOWN;
}

/*
==================
D3DGlobal_GetAlphaFormat

Gets a valid alpha format for a given adapter format
==================
*/
static D3DFORMAT D3DGlobal_GetAlphaFormat( D3DFORMAT AdapterFormat )
{
	// valid depth formats
	// first try to create formats with depthstencil buffer, then depth-only
	const D3DFORMAT d3d_ColorFormats[] = {D3DFMT_A8R8G8B8, D3DFMT_A4R4G4B4, D3DFMT_UNKNOWN};

	for (int i = 0; ; ++i)
	{
		if (d3d_ColorFormats[i] == D3DFMT_UNKNOWN)
			break;

		HRESULT hr = D3DGlobal.pD3D->CheckDeviceFormat(	D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, AdapterFormat, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, d3d_ColorFormats[i] );

		logPrintf("GetAlphaFormat: format %s is %ssupported\n", D3DGlobal_FormatToString(d3d_ColorFormats[i]), FAILED(hr) ? "not " : "");

		// return the first format that succeeds
		if (SUCCEEDED(hr)) 
			return d3d_ColorFormats[i];
	}

	// didn't get one
	logPrintf("GetDepthFormat: no valid alpha format found\n");
	return D3DFMT_UNKNOWN;
}


/*
======================
D3DGlobal_CheckTextureFormat

Ensures that a given texture format will be available
======================
*/
BOOL D3DGlobal_CheckTextureFormat( D3DFORMAT TextureFormat, D3DFORMAT AdapterFormat )
{
	HRESULT hr = D3DGlobal.pD3D->CheckDeviceFormat(	D3DADAPTER_DEFAULT,	D3DDEVTYPE_HAL,	AdapterFormat, 0, D3DRTYPE_TEXTURE,	TextureFormat );
	logPrintf("CheckTextureFormat: format %s is %ssupported\n", D3DGlobal_FormatToString(TextureFormat), FAILED(hr) ? "not " : "");
	return SUCCEEDED(hr);
}

/*
======================
D3DGlobal_CheckMultisampleType

Ensures that multisample type is available
======================
*/
BOOL D3DGlobal_CheckMultisampleType( D3DFORMAT BackBufferFormat, D3DFORMAT DepthBufferFormat, int numSamples, DWORD *pQuality )
{
	if( SUCCEEDED(D3DGlobal.pD3D->CheckDeviceMultiSampleType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, BackBufferFormat, FALSE, (D3DMULTISAMPLE_TYPE)numSamples, pQuality ) ) &&
		SUCCEEDED(D3DGlobal.pD3D->CheckDeviceMultiSampleType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, DepthBufferFormat, FALSE, (D3DMULTISAMPLE_TYPE)numSamples, pQuality ) ) )
	{
		return TRUE;
	} else {
		return FALSE;
	}
}


/*
========================
D3DGlobal_GetAdapterModeFormat

returns a usable adapter mode for the given width, height and bpp
========================
*/
static D3DFORMAT D3DGlobal_GetAdapterModeFormat( int width, int height, int bpp )
{
	// fill these in depending on bpp
	D3DFORMAT d3d_Formats[3];

	// these are the orders in which we prefer our formats
	if (bpp == 0)
	{
		// unspecified bpp uses the desktop mode format
		d3d_Formats[0] = D3DGlobal.hDesktopMode.Format;
		d3d_Formats[1] = D3DFMT_UNKNOWN;
	}
	else if (bpp == 16)
	{
		d3d_Formats[0] = D3DFMT_R5G6B5;
		d3d_Formats[1] = D3DFMT_X1R5G5B5;
		d3d_Formats[2] = D3DFMT_UNKNOWN;
	}
	else
	{
		d3d_Formats[0] = D3DFMT_X8R8G8B8;
		d3d_Formats[1] = D3DFMT_A8R8G8B8;
		d3d_Formats[2] = D3DFMT_UNKNOWN;
	}

	for (int i = 0; ; ++i)
	{
		// no more modes
		if (d3d_Formats[i] == D3DFMT_UNKNOWN) 
			break;

		// get and validate the number of modes for this format; we expect that this will succeed first time
		UINT modecount = D3DGlobal.pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT, d3d_Formats[i]);
		logPrintf("GetAdapterModeFormat: format %s supports %i modes\n", D3DGlobal_FormatToString(d3d_Formats[i]), modecount);
		if (!modecount) 
			continue;

		// check each mode in turn to find a match
		for (UINT m = 0; m < modecount; ++m)
		{
			// get this mode
			D3DDISPLAYMODE mode;
			logPrintf("GetAdapterModeFormat: format %s, checking mode %i...\n", D3DGlobal_FormatToString(d3d_Formats[i]), m);

			HRESULT hr = D3DGlobal.pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, d3d_Formats[i], m, &mode);

			// should never happen
			if (FAILED (hr)) 
				continue;

			// check it against the requested mode
			if (mode.Width != (UINT)width || mode.Height != (UINT)height)
				continue;

			// ensure that we can get a depth buffer
			if (D3DGlobal_GetDepthFormat(d3d_Formats[i]) == D3DFMT_UNKNOWN) 
				continue;

			// ensure that we can get backbuffer with alpha
			if (bpp == 32) {
				if (D3DGlobal_GetAlphaFormat(d3d_Formats[i]) == D3DFMT_UNKNOWN) 
					continue;
			}

			// ensure that the texture formats we want to create exist
			if (!D3DGlobal_CheckTextureFormat (D3DFMT_L8, d3d_Formats[i])) 
				continue;
			if (!D3DGlobal_CheckTextureFormat (D3DFMT_X8R8G8B8, d3d_Formats[i])) 
				continue;
			if (!D3DGlobal_CheckTextureFormat (D3DFMT_A8R8G8B8, d3d_Formats[i])) 
				continue;

			// copy it out and return the mode we got
			memcpy (&D3DGlobal.hCurrentMode, &mode, sizeof (D3DDISPLAYMODE));
			logPrintf("GetAdapterModeFormat: mode %i accepted (new format %s)\n", m, D3DGlobal_FormatToString(mode.Format));
			return mode.Format;
		}
	}

	// didn't find a format
	return D3DFMT_UNKNOWN;
}

/*
========================
D3DGlobal_SetupPresentParams

Initialize present parameters with a valid adapter mode
========================
*/
static bool D3DGlobal_SetupPresentParams( int width, int height, int bpp, BOOL windowed )
{
	// clear present params to NULL
	memset (&D3DGlobal.hPresentParams, 0, sizeof (D3DPRESENT_PARAMETERS));

	logPrintf("SetupPresentParams: %i x %i x %i (%s)\n", width, height, bpp, windowed ? "windowed" : "fullscreen" );

	// popup windows are fullscreen always
	if (windowed)
	{
		// defaults for windowed mode - also need to store out clientrect.right and clientrect.bottom
		// (iBPP is only used for fullscreen modes)
		D3DGlobal.hCurrentMode.Format = D3DGlobal.hDesktopMode.Format;
		D3DGlobal.hCurrentMode.Width = width;
		D3DGlobal.hCurrentMode.Height = height;
		D3DGlobal.hCurrentMode.RefreshRate = 0;
	}
	else
	{
		// also fills in d3d_CurrentMode
		D3DFORMAT fmt = D3DGlobal_GetAdapterModeFormat(width, height, D3DGlobal.iBPP);

		// ensure that we got a good format
		if (fmt == D3DFMT_UNKNOWN) {
			logPrintf("SetupPresentParams: failed to get fullscreen mode\n" );
			return false;
		}
	}

	// fill in mode-dependent stuff
	if (bpp == 32)
		D3DGlobal.hPresentParams.BackBufferFormat = D3DGlobal_GetAlphaFormat(D3DGlobal.hCurrentMode.Format);
	else
		D3DGlobal.hPresentParams.BackBufferFormat = D3DGlobal.hCurrentMode.Format;

	D3DGlobal.hPresentParams.FullScreen_RefreshRateInHz = D3DGlobal.hCurrentMode.RefreshRate;
	D3DGlobal.hPresentParams.Windowed = windowed;

	// request 1 backbuffer
	D3DGlobal.hPresentParams.BackBufferCount = 1;
	D3DGlobal.hPresentParams.BackBufferWidth = width;
	D3DGlobal.hPresentParams.BackBufferHeight = height;

	D3DGlobal.hPresentParams.EnableAutoDepthStencil = TRUE;
	D3DGlobal.hPresentParams.AutoDepthStencilFormat = D3DGlobal_GetDepthFormat(D3DGlobal.hCurrentMode.Format);
	D3DGlobal.hPresentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
	D3DGlobal.hPresentParams.MultiSampleQuality = 0;
	D3DGlobal.hPresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	D3DGlobal.hPresentParams.hDeviceWindow = D3DGlobal.hWnd;

	D3DGlobal.multiSamples = 0;
	if (D3DGlobal.settings.multisample) {
		int currentSamples = D3DGlobal.settings.multisample;
		DWORD quality;
		while (currentSamples > 1) {
			if (!D3DGlobal_CheckMultisampleType(D3DGlobal.hPresentParams.BackBufferFormat, D3DGlobal.hPresentParams.AutoDepthStencilFormat, currentSamples, &quality)) {
				logPrintf("%ix FSAA is not supported\n", currentSamples);
				currentSamples--;
				continue;
			}
			D3DGlobal.hPresentParams.MultiSampleType = (D3DMULTISAMPLE_TYPE)currentSamples;
			D3DGlobal.hPresentParams.MultiSampleQuality = quality-1;
			logPrintf("Using %ix FSAA (quality = %i)\n", currentSamples, quality-1);
			break;
		}
		D3DGlobal.multiSamples = currentSamples;
	}

	// check for S3TC
	D3DGlobal.supportsS3TC = 0;
	if ( D3DGlobal_CheckTextureFormat( D3DFMT_DXT1, D3DGlobal.hCurrentMode.Format ) &&
		 D3DGlobal_CheckTextureFormat( D3DFMT_DXT3, D3DGlobal.hCurrentMode.Format ) &&
		 D3DGlobal_CheckTextureFormat( D3DFMT_DXT5, D3DGlobal.hCurrentMode.Format ) ) {
		logPrintf("S3TC texture compression is supported\n");
		D3DGlobal.supportsS3TC = 1;
	}

	return true;
}

#define D3D_CONTEXT_MAGIC	0xBEEF

OPENGL_API HGLRC WINAPI wrap_wglCreateContext( HDC hdc )
{
	//logPrintf("wrap_wglCreateContext( %x )\n", hdc);

	if (D3DGlobal.hGLRC)	//don't create multiple contexts
	{
		logPrintf("wglCreateContext: attempt to create additional context, ignored\n");
		return 0;
	}

	D3DGlobal.hDC = hdc;
	D3DGlobal.hWnd = WindowFromDC(hdc);
	if (!D3DGlobal.hWnd) {
		logPrintf("wglCreateContext: WindowFromDC failed (hdc = 0x%x)\n", hdc);
		return 0;
	}

	// initialize direct3d
	if (!D3DGlobal.pD3D) {
		if (nullptr == (D3DGlobal.hD3DDll = LoadLibrary("d3d9.dll"))) {
			logPrintf("wglCreateContext: failed to load d3d9.dll\n");
			return 0;
		}
		pfnDirect3DCreate9 d3dCreateFn = (pfnDirect3DCreate9)GetProcAddress(D3DGlobal.hD3DDll, "Direct3DCreate9");
		if (!d3dCreateFn) {
			logPrintf("wglCreateContext: failed to get address of \"Direct3DCreate9\" from d3d9.dll\n");
			return 0;
		}

		if (nullptr == (D3DGlobal.pD3D = d3dCreateFn(D3D_SDK_VERSION)) ) {
			logPrintf("wglCreateContext: failed to initialize Direct3D\n");
			return 0;
		}
		if (nullptr == D3DGlobal.pD3D) {
			logPrintf("wglCreateContext: Direct3DCreate9 returned NULL\n");
			return 0;
		}
		logPrintf("Direct3D initialized\n");
	}

	// if a device was previously set up, return TRUE
	if (D3DGlobal.pDevice) 
		return D3DGlobal.hGLRC;

	D3DGlobal_Cleanup( false );
	D3DGlobal_Init( false );

	D3DGlobal.settings.multisample = D3DGlobal_GetRegistryValue( "MultiSample", "Settings", 0 );
	D3DGlobal.settings.projectionFix = D3DGlobal_GetRegistryValue( "ProjectionFix", "Settings", 0 );
	D3DGlobal.settings.texcoordFix = D3DGlobal_GetRegistryValue( "TexCoordFix", "Settings", 0 );
	D3DGlobal.settings.useSSE = D3DGlobal_GetRegistryValue( "UseSSE", "Settings", 0 );

	D3DGlobal_CPU_Detect();

	D3DGlobal.hGLRC = (HGLRC)D3D_CONTEXT_MAGIC;

	RECT clientrect;
	LONG winstyle;
	HRESULT hr;
	
	// get the dimensions of the window
	if (!GetClientRect(D3DGlobal.hWnd, &clientrect)) {
		logPrintf("wglCreateContext: GetClientRect failed with error %u\n", GetLastError());
		return 0;
	}

	// get window style
	HWND hwndParent;
	if (nullptr != (hwndParent = GetParent(D3DGlobal.hWnd))) {
		winstyle = GetWindowLong(hwndParent, GWL_STYLE);
		logPrintf("Parent window Style:");
	} else {
		winstyle = GetWindowLong(D3DGlobal.hWnd, GWL_STYLE);
		logPrintf("Window Style:");
	}
	if (winstyle & WS_BORDER) logPrintf(" WS_BORDER");
	if (winstyle & WS_CAPTION) logPrintf(" WS_CAPTION");
	if (winstyle & WS_CHILD) logPrintf(" WS_CHILD");
	if (winstyle & WS_CLIPCHILDREN) logPrintf(" WS_CLIPCHILDREN");
	if (winstyle & WS_CLIPSIBLINGS) logPrintf(" WS_CLIPSIBLINGS");
	if (winstyle & WS_DISABLED) logPrintf(" WS_DISABLED");
	if (winstyle & WS_DLGFRAME) logPrintf(" WS_DLGFRAME");
	if (winstyle & WS_GROUP) logPrintf(" WS_GROUP");
	if (winstyle & WS_MAXIMIZEBOX) logPrintf(" WS_MAXIMIZEBOX");
	if (winstyle & WS_MINIMIZEBOX) logPrintf(" WS_MINIMIZEBOX");
	if (winstyle & WS_POPUP) logPrintf(" WS_POPUP");
	if (winstyle & WS_SIZEBOX) logPrintf(" WS_SIZEBOX");
	if (winstyle & WS_SYSMENU) logPrintf(" WS_SYSMENU");
	if (winstyle & WS_THICKFRAME) logPrintf(" WS_THICKFRAME");
	logPrintf("\n");

	BOOL isWindowed = winstyle & (WS_BORDER|WS_CAPTION|WS_CHILD);

	if ((clientrect.right - clientrect.left <= 0) ||
		(clientrect.bottom - clientrect.top <= 0)) {
		clientrect.right = clientrect.left + 640;
		clientrect.bottom = clientrect.top + 480;
		isWindowed = 1;
	}

	if (FAILED(hr = D3DGlobal.pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &D3DGlobal.hD3DCaps))) {
		D3DGlobal.lastError = hr;
		logPrintf("wglCreateContext: GetDeviceCaps failed with error '%s'\n", DXGetErrorString9(hr));
		return 0;
	}

	// get the format for the desktop mode
	if (FAILED(hr = D3DGlobal.pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &D3DGlobal.hDesktopMode))) {
		D3DGlobal.lastError = hr;
		logPrintf("wglCreateContext: GetAdapterDisplayMode failed with error '%s'\n", DXGetErrorString9(hr));
		return 0;
	}

	// setup our present parameters
	if (!D3DGlobal_SetupPresentParams(clientrect.right, clientrect.bottom, D3DGlobal.iBPP, isWindowed)) {
		if (!isWindowed) {
			//try to start windowed
			isWindowed = 1;
			if (!D3DGlobal_SetupPresentParams(clientrect.right, clientrect.bottom, D3DGlobal.iBPP, isWindowed)) {
				D3DGlobal.lastError = E_INVALIDARG;
				return 0;
			}
		} else {
			D3DGlobal.lastError = E_INVALIDARG;
			return 0;
		}
	}

	// here we use D3DCREATE_FPU_PRESERVE to maintain the resolution of Quake's timers (this is a serious problem)
	// and D3DCREATE_DISABLE_DRIVER_MANAGEMENT to protect us from rogue drivers (call it honest paranoia).  first
	// we attempt to create a hardware vp device.
	// --------------------------------------------------------------------------------------------------------
	// NOTE re pure devices: we intentionally DON'T request a pure device, EVEN if one is available, as we need
	// to support certain glGet functions that would be broken if we used one.
	// --------------------------------------------------------------------------------------------------------
	// NOTE re D3DCREATE_FPU_PRESERVE - this can be avoided if we use a timer that's not subject to FPU drift,
	// such as timeGetTime (with timeBeginTime (1)); by default Quake's times *ARE* prone to FPU drift as they
	// use doubles for storing the last time, which gradually creeps up to be nearer to the current time each
	// frame.  Not using doubles for the stored times (i.e. switching them all to floats) would also help here.
	logPrintf("wglCreateContext: creating pure device with hardware vertex processing\n");
	hr = D3DGlobal.pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DGlobal.hWnd, 
									   D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_DISABLE_DRIVER_MANAGEMENT | D3DCREATE_PUREDEVICE,
									   &D3DGlobal.hPresentParams, &D3DGlobal.pDevice );

	if (FAILED(hr)) {
		logPrintf("wglCreateContext: CreateDevice failed with error '%s'\n", DXGetErrorString9(hr));
		logPrintf("wglCreateContext: creating device with hardware vertex processing\n");
	
		hr = D3DGlobal.pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DGlobal.hWnd, 
										   D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
										   &D3DGlobal.hPresentParams, &D3DGlobal.pDevice );

		if (FAILED(hr)) {
			// it's OK, we may not have hardware vp available, so create a software vp device
			logPrintf("wglCreateContext: CreateDevice failed with error '%s'\n", DXGetErrorString9(hr));
			logPrintf("wglCreateContext: creating device with software vertex processing\n");

			hr = D3DGlobal.pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DGlobal.hWnd, 
											   D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
											   &D3DGlobal.hPresentParams, &D3DGlobal.pDevice );
			if (FAILED(hr)) {
				logPrintf("wglCreateContext: CreateDevice failed with error '%s'\n", DXGetErrorString9(hr));
				return 0;
			}
		}
	}

	if (nullptr == D3DGlobal.pDevice) {
		logPrintf("wglCreateContext: CreateDevice returned NULL\n");
		return 0;
	}

	hr = D3DGlobal.pDevice->GetSwapChain( 0, &D3DGlobal.pSwapChain );
	if (FAILED(hr)) {
		logPrintf("wglCreateContext: GetSwapChain failed with error '%s'\n", DXGetErrorString9(hr));
		return 0;
	}
	if (nullptr == D3DGlobal.pSwapChain) {
		logPrintf("wglCreateContext: GetSwapChain returned NULL\n");
		return 0;
	}

	//setup color bits
	LPDIRECT3DSURFACE9 lpBackBuffer;
	LPDIRECT3DSURFACE9 lpDepthStencil;
	D3DSURFACE_DESC hBackBufferDesc;
	D3DSURFACE_DESC hDepthStencilDesc;
	hr = D3DGlobal.pSwapChain->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &lpBackBuffer );
	if (FAILED(hr)) {
		logPrintf("wglCreateContext: GetBackBuffer failed with error '%s'\n", DXGetErrorString9(hr));
		return 0;
	}
	hr = lpBackBuffer->GetDesc( &hBackBufferDesc );
	if (FAILED(hr)) {
		logPrintf("wglCreateContext: lpBackBuffer->GetDesc failed with error '%s'\n", DXGetErrorString9(hr));
		return 0;
	}
	lpBackBuffer->Release();
	hr = D3DGlobal.pDevice->GetDepthStencilSurface( &lpDepthStencil );
	if (FAILED(hr)) {
		logPrintf("wglCreateContext: GetDepthStencilSurface failed with error '%s'\n", DXGetErrorString9(hr));
		return 0;
	}
	hr = lpDepthStencil->GetDesc( &hDepthStencilDesc );
	if (FAILED(hr)) {
		logPrintf("wglCreateContext: lpDepthStencil->GetDesc failed with error '%s'\n", DXGetErrorString9(hr));
		return 0;
	}
	lpDepthStencil->Release();

	D3DGlobal_SetupPixelFormatBits( hBackBufferDesc.Format, hDepthStencilDesc.Format );	
	
	//setup clear ignore mask
	D3DGlobal.ignoreClearMask = D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL;
	if (D3DGlobal.stencilBits) {
		D3DGlobal.ignoreClearMask &= ~D3DCLEAR_STENCIL;
	}
	if (D3DGlobal.depthBits) {
		D3DGlobal.ignoreClearMask &= ~D3DCLEAR_ZBUFFER;
	}

	//setup adapter info strings
	D3DADAPTER_IDENTIFIER9 adapterInfo;
	if( SUCCEEDED( D3DGlobal.pD3D->GetAdapterIdentifier( D3DADAPTER_DEFAULT, 0, &adapterInfo ) ) ) {
		D3DGlobal.szRendererName = UTIL_AllocString( adapterInfo.Description );
	}

	//init some caps
	D3DGlobal.maxActiveTMU = QINDIEGL_MIN( MAX_D3D_TMU, D3DGlobal.hD3DCaps.MaxSimultaneousTextures );
	D3DGlobal.maxActiveLights = QINDIEGL_MIN( IMPL_MAX_LIGHTS, D3DGlobal.hD3DCaps.MaxActiveLights );

	//build extensions string
	D3DExtension_BuildExtensionsString();

	//init matrix stacks
	D3DGlobal.modelviewMatrixStack = new D3DMatrixStack;
	D3DGlobal.projectionMatrixStack = new D3DMatrixStack;
	for (int i = 0; i < MAX_D3D_TMU; ++i)
		D3DGlobal.textureMatrixStack[i] = new D3DMatrixStack;

	//set default state
	D3DState_SetDefaults();

	//first clear
	D3DGlobal.pDevice->Clear( 0, nullptr, (D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL) & ~D3DGlobal.ignoreClearMask, 
							  D3DState.ColorBufferState.clearColor, D3DState.DepthBufferState.clearDepth, D3DState.StencilBufferState.clearStencil );
	D3DGlobal.initialized = true;
	D3DGlobal.deviceLost = false;
	D3DGlobal.sceneBegan = false;

	return D3DGlobal.hGLRC;
}

OPENGL_API BOOL WINAPI wrap_wglDeleteContext( HGLRC hglrc )
{
	if (!D3DGlobal.hGLRC && ((HGLRC)D3D_CONTEXT_MAGIC == hglrc)) {
		logPrintf("wglDeleteContext: called twice, skipping second call\n");
		return FALSE;
	}

	if ( hglrc != D3DGlobal.hGLRC ) {
		logPrintf("wglDeleteContext: attempt to delete additional context (0x%x), ignored\n", hglrc);
		return FALSE;
	}

	logPrintf("wglDeleteContext: deleting rendering context\n");

	D3DGlobal.hGLRC = (HGLRC)0;
	D3DGlobal_Cleanup( false );

	// success
	return TRUE;
}

OPENGL_API HGLRC WINAPI wrap_wglGetCurrentContext()
{
	return D3DGlobal.hGLRC;
}

OPENGL_API HDC WINAPI wrap_wglGetCurrentDC()
{
	return D3DGlobal.hDC;
}

OPENGL_API BOOL WINAPI wrap_wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
	//logPrintf("wrap_wglMakeCurrent( %x, %x )\n", hdc, hglrc);

	if (hglrc != nullptr && hdc != nullptr) {
		if (!D3DGlobal.pDevice)
			return FALSE;

		//check if mode was switched
		if (D3DGlobal.hDC != hdc) {
			logPrintf("--- Resetting mode ---\n");
			D3DGlobal.hWnd = WindowFromDC(hdc);
			if (!D3DGlobal.hWnd) {
				logPrintf("wglMakeCurrent: WindowFromDC failed (hdc = 0x%x)\n", hdc);
				return FALSE;
			}
			RECT clientrect;
			LONG winstyle;
	
			GetClientRect(D3DGlobal.hWnd, &clientrect);
			winstyle = GetWindowLong(D3DGlobal.hWnd, GWL_STYLE);
			BOOL isWindowed = winstyle & (WS_BORDER|WS_CAPTION|WS_CHILD);

			if (!D3DGlobal_SetupPresentParams (clientrect.right, clientrect.bottom, D3DGlobal.iBPP, isWindowed)) {
				logPrintf("wglMakeCurrent: SetupPresentParams failed\n");
				return FALSE;
			}

			D3DGlobal.pDevice->Reset(&D3DGlobal.hPresentParams);

			if (D3DGlobal.pSystemMemRT) {
				D3DGlobal.pSystemMemRT->Release();
				D3DGlobal.pSystemMemRT = nullptr;
			}
			if (D3DGlobal.pSystemMemFB) {
				D3DGlobal.pSystemMemFB->Release();
				D3DGlobal.pSystemMemFB = nullptr;
			}
	
			D3DState_Apply( GL_ALL_ATTRIB_BITS );
		}

		if (!D3DGlobal.pDevice) {
			logPrintf("WARNING: wrap_wglMakeCurrent: pDevice is NULL\n");
		} else {
			//check cooperative level
			HRESULT hr = D3DGlobal.pDevice->TestCooperativeLevel();
			if (hr == D3DERR_DEVICELOST) {
				logPrintf("wrap_wglMakeCurrent: D3DERR_DEVICELOST\n");
				D3DGlobal.deviceLost = true;
			} else if (hr == D3DERR_DEVICENOTRESET) {
				logPrintf("wrap_wglMakeCurrent: D3DERR_DEVICENOTRESET\n");
				D3DGlobal_Reset();
			}
		}
	}

	return TRUE;
}

OPENGL_API BOOL WINAPI wrap_wglSwapBuffers( HDC )
{
	if (!D3DGlobal.hGLRC)
		return FALSE;

	if (!D3DGlobal.pDevice)
		return FALSE;

	// if we lost the device (e.g. on a mode switch, alt-tab, etc) we must try to recover it
	if (D3DGlobal.deviceLost)
	{
		// here we get the current status of the device
		HRESULT hr = D3DGlobal.pDevice->TestCooperativeLevel();

		switch (hr)
		{
		case D3D_OK:
			// device is recovered
			D3DGlobal.deviceLost = false;
			D3DState_Apply( GL_ALL_ATTRIB_BITS );
			break;

		case D3DERR_DEVICELOST:
			// device is still lost
			break;

		case D3DERR_DEVICENOTRESET:
			// device is ready to be reset
			D3DGlobal_Reset();
			break;

		default:
			break;
		}

		// yield the CPU a little
		Sleep( 10 );

		// don't bother this frame
		return TRUE;
	}

	if (D3DGlobal.sceneBegan)
	{
		// see do we need to reset the viewport as D3D restricts present to the client viewport
		// this is needed as some engines (hello FitzQuake) set multiple smaller viewports for
		// different on-screen elements (e.g. menus)
		BOOL vpreset = FALSE;

		// test dimensions for a change; we don't just blindly reset it as this may not happen every frame
		if (D3DState.viewport.X != 0) vpreset = TRUE;
		if (D3DState.viewport.Y != 0) vpreset = TRUE;
		if (D3DState.viewport.Width != D3DGlobal.hCurrentMode.Width) vpreset = TRUE;
		if (D3DState.viewport.Height != D3DGlobal.hCurrentMode.Height) vpreset = TRUE;
	
		if (vpreset)
		{
			// now reset it to full window dimensions so that the full window will present
			D3DState.viewport.X = 0;
			D3DState.viewport.Y = 0;
			D3DState.viewport.Width = D3DGlobal.hCurrentMode.Width;
			D3DState.viewport.Height = D3DGlobal.hCurrentMode.Height;
			HRESULT hr = D3DGlobal.pDevice->SetViewport(&D3DState.viewport);
			if (FAILED(hr)) {
				D3DGlobal.lastError = hr;
				return TRUE;
			}
		}

		D3DGlobal.pDevice->EndScene();
		D3DGlobal.sceneBegan = false;

		HRESULT hr;
		
		if (D3DGlobal.vSync)
			hr = D3DGlobal.pDevice->Present( nullptr, nullptr, nullptr, nullptr );
		else
			hr = D3DGlobal.pSwapChain->Present( nullptr, nullptr, nullptr, nullptr, D3DPRESENT_DONOTWAIT );

		if (hr != D3DERR_WASSTILLDRAWING) {
			if (hr == D3DERR_DEVICELOST) {
				// flag a lost device
				D3DGlobal.deviceLost = true;
			}
			else if (FAILED (hr)) {
				// something else bad happened
				D3DGlobal.lastError = hr;
			}
		}
	}

	if (D3DGlobal.pVABuffer)
		D3DGlobal.pVABuffer->ResetSwapFrame();

	//logPrintf("----- swap buffers -----\n\n");
	return TRUE;
}

static PIXELFORMATDESCRIPTOR s_d3dPixelFormat =
{
	sizeof(PIXELFORMATDESCRIPTOR),
	1,                                  // Version Number
	PFD_DRAW_TO_WINDOW |                // Format Must Support Window
	PFD_SUPPORT_OPENGL |                // Format Must Support OpenGL
	PFD_DOUBLEBUFFER,					// Must Support Double Buffering
	PFD_TYPE_RGBA,                      // Request An RGBA Format
	32,									// Select Our Color Depth
	0, 0, 0, 0, 0, 0,                   // Color Bits Ignored
	8,                                  // 8-bit Alpha Buffer
	0,                                  // Shift Bit Ignored
	0,                                  // No Accumulation Buffer
	0, 0, 0, 0,                         // Accumulation Bits Ignored
	24,                                 // Z-Buffer (Depth Buffer) bits
	8,                                  // 8-bit Stencil Buffer
	0,                                  // No Auxiliary Buffer
	PFD_MAIN_PLANE,                     // Main Drawing Layer
	0,                                  // Reserved
	0, 0, 0                             // Layer Masks Ignored
};

#if 0
static void DumpPixelFormat( PIXELFORMATDESCRIPTOR *pfd ) 
{
	logPrintf("DumpPixelFormat: 0x%x\n", pfd);
	if (!pfd) return;

	logPrintf(" Size = %i\n", pfd->nSize);
	logPrintf(" Version = %i\n", pfd->nVersion);
	logPrintf(" Flags = %i\n", pfd->dwFlags);
	logPrintf(" Pixel Type = %i\n", pfd->iPixelType);
	logPrintf(" Color Bits = %i\n", pfd->cColorBits);
	logPrintf(" Red Bits = %i\n", pfd->cRedBits);
	logPrintf(" Green Bits = %i\n", pfd->cGreenBits);
	logPrintf(" Blue Bits = %i\n", pfd->cBlueBits);
	logPrintf(" Alpha Bits = %i\n", pfd->cAlphaBits);
	logPrintf(" Accum Bits = %i\n", pfd->cAccumBits);
	logPrintf(" Depth Bits = %i\n", pfd->cDepthBits);
	logPrintf(" Stencil Bits = %i\n", pfd->cStencilBits);
	logPrintf(" Aux Buffers = %i\n", pfd->cAuxBuffers);
	logPrintf(" Layer Type = %i\n", pfd->iLayerType);
	logPrintf(" Layer Mask = %i\n", pfd->dwLayerMask);
}
#endif

OPENGL_API int WINAPI wrap_wglChoosePixelFormat( HDC, PIXELFORMATDESCRIPTOR *pfd ) 
{ 
	if ( pfd ) {
		s_d3dPixelFormat.cColorBits = pfd->cColorBits;
		D3DGlobal.iBPP = pfd->cColorBits;
		return 1;
	} 
	return 0;
}

OPENGL_API int WINAPI wrap_wglDescribePixelFormat( HDC, int, UINT, LPPIXELFORMATDESCRIPTOR pfd ) 
{ 
	if ( pfd ) memcpy( pfd, &s_d3dPixelFormat, sizeof(s_d3dPixelFormat) );
	return 1;
}

OPENGL_API int WINAPI wrap_wglGetPixelFormat( HDC ) 
{ 
	return 1;
}

OPENGL_API BOOL WINAPI wrap_wglSetPixelFormat( HDC, int, CONST PIXELFORMATDESCRIPTOR* ) 
{ 
	// just silently pass the PFD through unmodified
	return TRUE;
}

OPENGL_API BOOL WINAPI wrap_wglCopyContext( HGLRC, HGLRC, UINT )
{
	return FALSE;
}

OPENGL_API HGLRC WINAPI wrap_wglCreateLayerContext( HDC, int )
{
	return (HGLRC)0;
}

OPENGL_API BOOL WINAPI wrap_wglDescribeLayerPlane( HDC, int, int, UINT, LPLAYERPLANEDESCRIPTOR )
{
	return FALSE;
}

OPENGL_API int WINAPI wrap_wglGetLayerPaletteEntries( HDC, int, int, int, COLORREF* )
{
	return 0;
}

OPENGL_API int WINAPI wrap_wglSetLayerPaletteEntries( HDC, int, int, int, CONST COLORREF* )
{
	return 0;
}

OPENGL_API BOOL WINAPI wrap_wglRealizeLayerPalette( HDC, int, BOOL )
{
	return FALSE;
}

OPENGL_API BOOL WINAPI wrap_wglSwapLayerBuffers( HDC, UINT )
{
	return FALSE;
}

OPENGL_API BOOL WINAPI wrap_wglShareLists( HGLRC, HGLRC )
{
	return TRUE;
}
OPENGL_API BOOL WINAPI wrap_wglUseFontBitmapsA( HDC, DWORD, DWORD, DWORD )
{
	logPrintf("WARNING: wglUseFontBitmapsA is not supported\n");
	return FALSE;
}
OPENGL_API BOOL WINAPI wrap_wglUseFontBitmapsW( HDC, DWORD, DWORD, DWORD )
{
	logPrintf("WARNING: wglUseFontBitmapsW is not supported\n");
	return FALSE;
}

OPENGL_API BOOL WINAPI wrap_wglUseFontOutlinesA( HDC, DWORD, DWORD, DWORD, FLOAT, FLOAT, int, LPGLYPHMETRICSFLOAT )
{
	logPrintf("WARNING: wglUseFontOutlinesA is not supported\n");
	return FALSE;
}
OPENGL_API BOOL WINAPI wrap_wglUseFontOutlinesW( HDC, DWORD, DWORD, DWORD, FLOAT, FLOAT, int, LPGLYPHMETRICSFLOAT )
{
	logPrintf("WARNING: wglUseFontOutlinesW is not supported\n");
	return FALSE;
}

OPENGL_API BOOL WINAPI wglSwapInterval( int interval )
{
	D3DGlobal.vSync = (interval > 0);
	return TRUE;
}
OPENGL_API int WINAPI wglGetSwapInterval()
{
	return (D3DGlobal.vSync ? 1 : 0);
}
