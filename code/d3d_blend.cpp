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

//==================================================================================
// Blending states
//----------------------------------------------------------------------------------
// TODO: add separate blend func extension support (D3DRS_SEPARATEALPHABLENDENABLE)
// TODO: add separate blend equation extension support (D3DRS_BLENDOPALPHA)
//==================================================================================

OPENGL_API void WINAPI glAlphaFunc( GLenum func, GLclampf ref )
{
	DWORD dfunc = UTIL_GLtoD3DCmpFunc(func);
	if (dfunc != D3DState.ColorBufferState.alphaTestFunc) {
		D3DState.ColorBufferState.alphaTestFunc = dfunc;
		D3DState_SetRenderState( D3DRS_ALPHAFUNC, dfunc );
	}

	DWORD dref = (DWORD)QINDIEGL_CLAMP(ref * 255);
	if (dref != D3DState.ColorBufferState.alphaTestReference) {
		D3DState.ColorBufferState.alphaTestReference = dref;
		D3DState_SetRenderState( D3DRS_ALPHAREF, dref );
	}
}

OPENGL_API void WINAPI glBlendFunc( GLenum sfactor, GLenum dfactor )
{
	if (D3DState.ColorBufferState.glBlendSrc != sfactor) {
		D3DState.ColorBufferState.glBlendSrc = sfactor;
		DWORD sfunc = UTIL_GLtoD3DBlendFunc(sfactor);
		D3DState.ColorBufferState.alphaBlendSrcFunc = sfunc;
		D3DState_SetRenderState( D3DRS_SRCBLEND, sfunc );
	}
	if (D3DState.ColorBufferState.glBlendDst != dfactor) {
		D3DState.ColorBufferState.glBlendDst = dfactor;
		DWORD dfunc = UTIL_GLtoD3DBlendFunc(dfactor);
		D3DState.ColorBufferState.alphaBlendDstFunc = dfunc;
		D3DState_SetRenderState( D3DRS_DESTBLEND, dfunc );
	}

	if (D3DGlobal.hD3DCaps.SrcBlendCaps & D3DPBLENDCAPS_BLENDFACTOR) {
		if (sfactor == GL_CONSTANT_ALPHA_EXT ||
			sfactor == GL_ONE_MINUS_CONSTANT_ALPHA_EXT ||
			dfactor == GL_CONSTANT_ALPHA_EXT ||
			dfactor == GL_ONE_MINUS_CONSTANT_ALPHA_EXT) {
			D3DState.ColorBufferState.alphaBlendUseColor = 1;
		} else if (sfactor == GL_CONSTANT_COLOR_EXT ||
			sfactor == GL_ONE_MINUS_CONSTANT_COLOR_EXT ||
			dfactor == GL_CONSTANT_COLOR_EXT ||
			dfactor == GL_ONE_MINUS_CONSTANT_COLOR_EXT) {
			D3DState.ColorBufferState.alphaBlendUseColor = 2;
		} else {
			D3DState.ColorBufferState.alphaBlendUseColor = 0;
		}
		if (D3DState.ColorBufferState.alphaBlendUseColor == 1) {
			D3DState_SetRenderState( D3DRS_BLENDFACTOR, D3DState.ColorBufferState.alphaBlendColorAAAA );
		} else if (D3DState.ColorBufferState.alphaBlendUseColor == 2) {
			D3DState_SetRenderState( D3DRS_BLENDFACTOR, D3DState.ColorBufferState.alphaBlendColorARGB );
		}
	}
}

OPENGL_API void WINAPI glBlendColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
	DWORD da = (DWORD)(alpha * 255);
	DWORD dr = (DWORD)(red * 255);
	DWORD dg = (DWORD)(green * 255);
	DWORD db = (DWORD)(blue * 255);

	D3DState.ColorBufferState.alphaBlendColorARGB = D3DCOLOR_ARGB( da, dr, dg, db );
	D3DState.ColorBufferState.alphaBlendColorAAAA = D3DCOLOR_ARGB( da, da, da, da );

	if (D3DGlobal.hD3DCaps.SrcBlendCaps & D3DPBLENDCAPS_BLENDFACTOR) {
		if (D3DState.ColorBufferState.alphaBlendUseColor == 1) {
			D3DState_SetRenderState( D3DRS_BLENDFACTOR, D3DState.ColorBufferState.alphaBlendColorAAAA );
		} else if (D3DState.ColorBufferState.alphaBlendUseColor == 2) {
			D3DState_SetRenderState( D3DRS_BLENDFACTOR, D3DState.ColorBufferState.alphaBlendColorARGB );
		}
	}
}

OPENGL_API void WINAPI glBlendEquation( GLenum mode )
{
	DWORD func = UTIL_GLtoD3DBlendOp(mode);
	if (D3DState.ColorBufferState.alphaBlendOp != func) {
		D3DState.ColorBufferState.alphaBlendOp = func;
		D3DState_SetRenderState( D3DRS_BLENDOP, func );
	}
}

OPENGL_API void WINAPI glLogicOp( GLenum )
{
	logPrintf("WARNING: glLogicOp is not supported\n");
}

OPENGL_API void WINAPI glFogfv( GLenum pname, const GLfloat *params )
{
	switch (pname) {
	case GL_FOG_MODE:
		switch ((GLenum)params[0]) {
		default:
		case GL_EXP:
			D3DState.FogState.fogMode = D3DFOG_EXP;
			break;
		case GL_EXP2:
			D3DState.FogState.fogMode = D3DFOG_EXP2;
			break;
		case GL_LINEAR:
			D3DState.FogState.fogMode = D3DFOG_LINEAR;
			break;
		}
		D3DState_SetRenderState(D3DRS_FOGTABLEMODE, (!D3DState.FogState.fogCoordMode && D3DState.HintState.fogHint <= 1) ? D3DState.FogState.fogMode : D3DFOG_NONE);
		D3DState_SetRenderState(D3DRS_FOGVERTEXMODE, (D3DState.FogState.fogCoordMode || D3DState.HintState.fogHint <= 1) ? D3DFOG_NONE : D3DState.FogState.fogMode);
		break;
	case GL_FOG_COLOR:
		D3DState.FogState.fogColor = D3DCOLOR_ARGB( QINDIEGL_CLAMP( params[3] * 255.0f ),
													QINDIEGL_CLAMP( params[0] * 255.0f ),
													QINDIEGL_CLAMP( params[1] * 255.0f ),
													QINDIEGL_CLAMP( params[2] * 255.0f ));
		D3DState_SetRenderState( D3DRS_FOGCOLOR, D3DState.FogState.fogColor );
		break;
	case GL_FOG_START:
		D3DState.FogState.fogStart = params[0];
		D3DState_SetRenderState( D3DRS_FOGSTART, UTIL_FloatToDword(D3DState.FogState.fogStart) );
		break;
	case GL_FOG_END:
		D3DState.FogState.fogEnd = params[0];
		D3DState_SetRenderState( D3DRS_FOGEND, UTIL_FloatToDword(D3DState.FogState.fogEnd) );
		break;
	case GL_FOG_DENSITY:
		D3DState.FogState.fogDensity = params[0];
		D3DState_SetRenderState( D3DRS_FOGDENSITY, UTIL_FloatToDword(D3DState.FogState.fogDensity) );
		break;
	case GL_FOG_INDEX:
		//color index mode is not supported
		break;
	case GL_FOG_COORDINATE_SOURCE_EXT:
		if (params[0] == GL_FOG_COORDINATE_EXT) {
			D3DState.FogState.fogCoordMode = 1;
		} else {
			D3DState.FogState.fogCoordMode = 0;
		}
		D3DState_SetRenderState(D3DRS_FOGTABLEMODE, (!D3DState.FogState.fogCoordMode && D3DState.HintState.fogHint <= 1) ? D3DState.FogState.fogMode : D3DFOG_NONE);
		D3DState_SetRenderState(D3DRS_FOGVERTEXMODE, (D3DState.FogState.fogCoordMode || D3DState.HintState.fogHint <= 1) ? D3DFOG_NONE : D3DState.FogState.fogMode);
		break;

	default:
		logPrintf("WARNING: glFogf - bad pname 0x%x\n", pname);
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}
}

OPENGL_API void WINAPI glFogf( GLenum pname, GLfloat param )
{
	GLfloat fparams[4];
	fparams[0] = param;
	glFogfv( pname, fparams );
}

OPENGL_API void WINAPI glFogi( GLenum pname, GLint param )
{
	GLfloat fparams[4];
	fparams[0] = (GLfloat)param;
	glFogfv( pname, fparams );
}

OPENGL_API void WINAPI glFogiv( GLenum pname, const GLint *params )
{
	GLfloat fparams[4];
	fparams[0] = (GLfloat)params[0];
	fparams[1] = (GLfloat)params[1];
	fparams[2] = (GLfloat)params[2];
	fparams[3] = (GLfloat)params[3];
	glFogfv( pname, fparams );
}
