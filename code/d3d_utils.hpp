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
#ifndef QINDIEGL_D3D_UTILS_H
#define QINDIEGL_D3D_UTILS_H

inline DWORD UTIL_FloatToDword( float f ) 
{ 
	return *((DWORD*)&f); 
}
inline void *UTIL_Alloc( int size ) 
{ 
	return calloc( 1, size );
}
inline void *UTIL_Realloc( void *p, int size ) 
{ 
	return realloc( p, size );
}
inline void UTIL_Free( void *s ) 
{ 
	if(s) free(s); 
}
inline char *UTIL_AllocString( const char *s ) 
{ 
	if (!s) return NULL; 
	size_t len = strlen(s);
	char *s2 = (char*)calloc( 1, len+1 );
	strncpy_s(s2, len+1, s, len);
	return s2;
}

inline void UTIL_FreeString( char *s ) 
{ 
	if(s) free(s); 
}

inline DWORD UTIL_GLtoD3DCmpFunc( GLenum func )
{
	//Fast GL to D3D conversion
	return (func + 1 - GL_NEVER);
}

inline DWORD UTIL_GLtoD3DBlendFunc( GLenum func )
{
	switch (func) {
	default:
	case GL_ONE: return D3DBLEND_ONE;
	case GL_ZERO: return D3DBLEND_ZERO;
	case GL_SRC_COLOR: return D3DBLEND_SRCCOLOR;
	case GL_ONE_MINUS_SRC_COLOR: return D3DBLEND_INVSRCCOLOR;
	case GL_DST_COLOR: return D3DBLEND_DESTCOLOR;
	case GL_ONE_MINUS_DST_COLOR: return D3DBLEND_INVDESTCOLOR;
	case GL_SRC_ALPHA: return D3DBLEND_SRCALPHA;
	case GL_ONE_MINUS_SRC_ALPHA: return D3DBLEND_INVSRCALPHA;
	case GL_DST_ALPHA: return D3DBLEND_DESTALPHA;
	case GL_ONE_MINUS_DST_ALPHA: return D3DBLEND_INVDESTALPHA;
	case GL_SRC_ALPHA_SATURATE: return D3DBLEND_SRCALPHASAT;
	case GL_CONSTANT_COLOR_EXT: return D3DBLEND_BLENDFACTOR;
	case GL_ONE_MINUS_CONSTANT_COLOR_EXT: return D3DBLEND_INVBLENDFACTOR;
	case GL_CONSTANT_ALPHA_EXT: return D3DBLEND_BLENDFACTOR;
	case GL_ONE_MINUS_CONSTANT_ALPHA_EXT: return D3DBLEND_INVBLENDFACTOR;
	}
}

inline DWORD UTIL_GLtoD3DBlendOp( GLenum func )
{
	switch (func) {
	default:
	case GL_FUNC_ADD_EXT: return D3DBLENDOP_ADD;
	case GL_MIN_EXT: return D3DBLENDOP_MIN;
	case GL_MAX_EXT: return D3DBLENDOP_MAX;
	case GL_FUNC_SUBTRACT_EXT: return D3DBLENDOP_SUBTRACT;
	case GL_FUNC_REVERSE_SUBTRACT_EXT: return D3DBLENDOP_REVSUBTRACT;
	}
}

inline DWORD UTIL_GLtoD3DStencilFunc( GLenum func )
{
	switch (func) {
	default:
	case GL_KEEP: return D3DSTENCILOP_KEEP;
	case GL_ZERO: return D3DSTENCILOP_ZERO;
	case GL_REPLACE: return D3DSTENCILOP_REPLACE;
	case GL_INCR: return D3DSTENCILOP_INCRSAT;
	case GL_DECR: return D3DSTENCILOP_DECRSAT;
	case GL_INVERT: return D3DSTENCILOP_INVERT;
	case GL_INCR_WRAP_EXT: return D3DSTENCILOP_INCR;
	case GL_DECR_WRAP_EXT: return D3DSTENCILOP_DECR;
	}
}

inline DWORD UTIL_GLtoD3DTextureCombineOp( GLenum func, DWORD scale )
{
	switch (func) {
	default:
	case GL_REPLACE: return D3DTOP_SELECTARG1;
	case GL_MODULATE: return ((scale < 2) ? D3DTOP_MODULATE : ( (scale == 4) ? D3DTOP_MODULATE4X : D3DTOP_MODULATE2X ));
	case GL_ADD: return D3DTOP_ADD;
	case GL_ADD_SIGNED_ARB: return ((scale < 2) ? D3DTOP_ADDSIGNED : D3DTOP_ADDSIGNED2X);
	case GL_SUBTRACT_ARB: return D3DTOP_SUBTRACT;
	case GL_INTERPOLATE_ARB: return D3DTOP_LERP;
	case GL_DOT3_RGB_ARB:
	case GL_DOT3_RGBA_ARB:
	case GL_DOT3_RGB_EXT:
	case GL_DOT3_RGBA_EXT: return D3DTOP_DOTPRODUCT3;
	}
}

inline DWORD UTIL_GLtoD3DTextureCombineColorArg( GLenum func, GLenum op )
{
	DWORD arg;
	switch (func) {
	default:
	case GL_PREVIOUS_ARB: arg = D3DTA_CURRENT; break;
	case GL_TEXTURE0_ARB:
	case GL_TEXTURE0_SGIS:
	case GL_TEXTURE: arg = D3DTA_TEXTURE; break;
	case GL_PRIMARY_COLOR_ARB: arg = D3DTA_DIFFUSE; break;
	case GL_CONSTANT_ARB: arg = D3DTA_TFACTOR; break;
	}

	if (op == GL_SRC_ALPHA || op == GL_ONE_MINUS_SRC_ALPHA)
		arg |= D3DTA_ALPHAREPLICATE;
	if (op == GL_ONE_MINUS_SRC_COLOR || op == GL_ONE_MINUS_SRC_ALPHA)
		arg |= D3DTA_COMPLEMENT;

	return arg;
}

inline DWORD UTIL_GLtoD3DTextureCombineAlphaArg( GLenum func, GLenum op )
{
	DWORD arg;
	switch (func) {
	default:
	case GL_PREVIOUS_ARB: arg = D3DTA_CURRENT; break;
	case GL_TEXTURE0_ARB:
	case GL_TEXTURE0_SGIS:
	case GL_TEXTURE: arg = D3DTA_TEXTURE; break;
	case GL_PRIMARY_COLOR_ARB: arg = D3DTA_DIFFUSE; break;
	case GL_CONSTANT_ARB: arg = D3DTA_TFACTOR; break;
	}

	if (op == GL_ONE_MINUS_SRC_ALPHA)
		arg |= D3DTA_COMPLEMENT;

	return arg;
}

inline int UTIL_GLtoD3DAddressIndex( GLenum func )
{
	switch (func) {
	case GL_TEXTURE_WRAP_S: 
		return 0;
	case GL_TEXTURE_WRAP_T:
		return 1;
	case GL_TEXTURE_WRAP_R:
		return 2;	
	default:
		logPrintf("WARNING: unsupported texture address 0x%x\n", func);
		return 0;
	}
}
inline D3DTEXTUREADDRESS UTIL_GLtoD3DAddressMode( GLenum func )
{
	switch (func) {
	default:
	case GL_CLAMP: 
	case GL_CLAMP_TO_EDGE:
		return D3DTADDRESS_CLAMP;
	case GL_REPEAT: 
		return D3DTADDRESS_WRAP;
	case GL_CLAMP_TO_BORDER_ARB: 
		return D3DTADDRESS_BORDER;
	case GL_MIRROR_CLAMP_EXT:
	case GL_MIRROR_CLAMP_TO_EDGE_EXT:
	case GL_MIRROR_CLAMP_TO_BORDER_EXT:	//just for compatibility - we won't report a support for EXT_texture_mirror clamp
		return D3DTADDRESS_MIRRORONCE;
	case GL_MIRRORED_REPEAT_ARB:
		return D3DTADDRESS_MIRROR;
	}
}

inline int UTIL_GLTextureTargettoInternalIndex( GLenum target )
{
	switch (target) {
	case GL_TEXTURE_1D: 
		return 0;
	case GL_TEXTURE_2D: 
		return 1;
	case GL_TEXTURE_3D_EXT: 
		return 2;
	case GL_TEXTURE_CUBE_MAP_ARB:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB:
		return 3;
	default:
		logPrintf("WARNING: unsupported texture target 0x%x\n", target);
		return -1;
	}
}


#endif //QINDIEGL_D3D_UTILS_H