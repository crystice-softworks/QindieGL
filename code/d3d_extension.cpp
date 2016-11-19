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
#include "d3d_utils.hpp"
#include "d3d_extension.hpp"

//This will enable export of our custom extensions
#define ALLOW_CHS_EXTENSIONS

//TODO:
//GL_ARB_depth_texture
//GL_ARB_vertex_buffer_object
//GL_ARB_point_sprite

OPENGL_API const char* WINAPI wglGetExtensionsStringARB( HDC )
{
	return D3DGlobal.szWExtensions;
}

typedef struct glext_entry_point_s
{
	char *name;
	char *extname;
	int  enabled;
	PROC func;
} glext_entry_point_t;

#define GL_EXT_ENTRY_POINT( postfix, extname, func, defaultEnable )		{ #func, "GL_" ## postfix ## "_" ## extname, defaultEnable, (PROC)func }, \
																		{ #func ## postfix, "GL_" ## postfix ## "_" ## extname, defaultEnable, (PROC)func }
#define WGL_EXT_ENTRY_POINT( postfix, extname, func, defaultEnable )	{ #func, "WGL_" ## postfix ## "_" ## extname, defaultEnable, (PROC)func }, \
																		{ #func ## postfix, "WGL_" ## postfix ## "_" ## extname, defaultEnable, (PROC)func }

static glext_entry_point_t glext_EntryPoints[] =
{
	//GL_EXT_texture_object (not and extension in GL 1.1)
	GL_EXT_ENTRY_POINT( "EXT", "texture_object", glDeleteTextures, -2 ),
	GL_EXT_ENTRY_POINT( "EXT", "texture_object", glGenTextures, -2 ),
	GL_EXT_ENTRY_POINT( "EXT", "texture_object", glIsTexture, -2 ),
	GL_EXT_ENTRY_POINT( "EXT", "texture_object", glBindTexture, -2 ),
	GL_EXT_ENTRY_POINT( "EXT", "texture_object", glAreTexturesResident, -2 ),
	GL_EXT_ENTRY_POINT( "EXT", "texture_object", glPrioritizeTextures, -2 ),

	//GL_ARB_texture_compression
	GL_EXT_ENTRY_POINT( "ARB", "texture_compression", glCompressedTexImage1D, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "texture_compression", glCompressedTexImage2D, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "texture_compression", glCompressedTexImage3D, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "texture_compression", glCompressedTexSubImage1D, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "texture_compression", glCompressedTexSubImage2D, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "texture_compression", glCompressedTexSubImage3D, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "texture_compression", glGetCompressedTexImage, -1 ),

	//GL_ARB_multitexture
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glActiveTexture, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glClientActiveTexture, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord1s, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord1i, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord1f, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord1d, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord2s, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord2i, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord2f, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord2d, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord3s, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord3i, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord3f, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord3d, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord4s, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord4i, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord4f, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord4d, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord1sv, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord1iv, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord1fv, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord1dv, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord2sv, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord2iv, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord2fv, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord2dv, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord3sv, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord3iv, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord3fv, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord3dv, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord4sv, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord4iv, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord4fv, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "multitexture", glMultiTexCoord4dv, -1 ),

	//GL_ARB_transpose_matrix
	GL_EXT_ENTRY_POINT( "ARB", "transpose_matrix", glLoadTransposeMatrixf, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "transpose_matrix", glLoadTransposeMatrixd, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "transpose_matrix", glMultTransposeMatrixf, -1 ),
	GL_EXT_ENTRY_POINT( "ARB", "transpose_matrix", glMultTransposeMatrixd, -1 ),

	//GL_EXT_blend_color
	GL_EXT_ENTRY_POINT( "EXT", "blend_color", glBlendColor, -1 ),

	//GL_EXT_blend_minmax and GL_EXT_blend_subtract
	GL_EXT_ENTRY_POINT( "EXT", "blend_minmax", glBlendEquation, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "blend_subtract", glBlendEquation, -1 ),

	//GL_EXT_compiled_vertex_array
	GL_EXT_ENTRY_POINT( "EXT", "compiled_vertex_array", glLockArrays, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "compiled_vertex_array", glUnlockArrays, -1 ),

	//GL_EXT_draw_range_elements
	GL_EXT_ENTRY_POINT( "EXT", "draw_range_elements", glDrawRangeElements, -1 ),

	//GL_EXT_multi_draw_arrays
	GL_EXT_ENTRY_POINT( "EXT", "multi_draw_arrays", glMultiDrawArrays, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "multi_draw_arrays", glMultiDrawElements, -1 ),
	GL_EXT_ENTRY_POINT( "SUN", "multi_draw_arrays", glMultiDrawArrays, -1 ),
	GL_EXT_ENTRY_POINT( "SUN", "multi_draw_arrays", glMultiDrawElements, -1 ),

	//GL_EXT_secondary_color
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColor3b, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColor3bv, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColor3d, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColor3dv, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColor3f, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColor3fv, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColor3i, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColor3iv, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColor3s, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColor3sv, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColor3ub, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColor3ubv, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColor3ui, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColor3uiv, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColor3us, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColor3usv, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "secondary_color", glSecondaryColorPointer, -1 ),

	//GL_EXT_fog_coord
	GL_EXT_ENTRY_POINT( "EXT", "fog_coord", glFogCoordd, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "fog_coord", glFogCoordf, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "fog_coord", glFogCoorddv, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "fog_coord", glFogCoordfv, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "fog_coord", glFogCoordPointer, -1 ),

	//GL_SGIS_multitexture
	GL_EXT_ENTRY_POINT( "SGIS", "multitexture", glSelectTexture, -1 ),
	GL_EXT_ENTRY_POINT( "SGIS", "multitexture", glMTexCoord2f, -1 ),
	GL_EXT_ENTRY_POINT( "SGIS", "multitexture", glMTexCoord2fv, -1 ),

	//GL_EXT_texture3D
	GL_EXT_ENTRY_POINT( "EXT", "texture3D", glTexImage3D, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "texture3D", glTexSubImage3D, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "texture3D", glCopyTexImage3D, -1 ),
	GL_EXT_ENTRY_POINT( "EXT", "texture3D", glCopyTexSubImage3D, -1 ),

	//GL_EXT_stencil_two_side
	GL_EXT_ENTRY_POINT( "EXT", "stencil_two_side", glActiveStencilFace, -1 ),

	//WGL_EXT_swap_control
	WGL_EXT_ENTRY_POINT( "EXT", "swap_control", wglSwapInterval, -2 ),
	WGL_EXT_ENTRY_POINT( "EXT", "swap_control", wglGetSwapInterval, -2 ),

	//WGL_ARB_extensions_string
	WGL_EXT_ENTRY_POINT( "ARB", "extensions_string", wglGetExtensionsStringARB, -2 ),

	{ NULL, NULL }
};

class CExtensionBuf
{
public:
	CExtensionBuf() : m_size( 256 ), m_cur( 0 ), m_buf( reinterpret_cast<char*>( UTIL_Alloc( m_size ) ) ) {}
	~CExtensionBuf()
	{
		UTIL_Free(m_buf);
	}
	char *CopyBuffer() const
	{
		return UTIL_AllocString( !m_cur ? "" : m_buf );
	}
	void AddExtension( const char *ext )
	{
		if (!ext) return;
		if (!D3DGlobal_GetRegistryValue(ext, "Extensions", 0)) return;
		size_t len = strlen(ext);
		CheckSpace( len + 2 );
		strcpy_s( m_buf + m_cur, len + 1, ext );
		*(m_buf + m_cur + len) = ' ';
		*(m_buf + m_cur + len + 1) = '\0';
		m_cur += len + 1;
	}
	void CheckSpace( int len )
	{
		if (m_size > m_cur + len) return;
		m_size += 256;
		m_buf = reinterpret_cast<char*>( UTIL_Realloc( m_buf, m_size ) );
	}
private:
	int m_size;
	int m_cur;
	char *m_buf; 
};

void D3DExtension_BuildExtensionsString()
{
	assert( D3DGlobal.pD3D != NULL );
	assert( D3DGlobal.pDevice != NULL );

	CExtensionBuf ExtensionBuf;
	CExtensionBuf WExtensionBuf;
	GLuint checkCaps;
	bool bCombineSupportEXT( true );
	bool bCombineSupportARB( true );

	if (D3DGlobal.maxActiveTMU > 1) ExtensionBuf.AddExtension( "GL_ARB_multitexture" );
	
	checkCaps = (D3DPTADDRESSCAPS_BORDER);
	if ((D3DGlobal.hD3DCaps.TextureAddressCaps & checkCaps) == checkCaps) ExtensionBuf.AddExtension( "GL_ARB_texture_border_clamp" );
	checkCaps = (D3DPTEXTURECAPS_CUBEMAP);
	if ((D3DGlobal.hD3DCaps.TextureCaps & checkCaps) == checkCaps) ExtensionBuf.AddExtension( "GL_ARB_texture_cube_map" );
	checkCaps = (D3DTEXOPCAPS_ADD);
	if ((D3DGlobal.hD3DCaps.TextureOpCaps & checkCaps) == checkCaps) ExtensionBuf.AddExtension( "GL_ARB_texture_env_add" );

	checkCaps = (D3DTEXOPCAPS_SELECTARG1);
	if ((D3DGlobal.hD3DCaps.TextureOpCaps & checkCaps) != checkCaps) bCombineSupportARB = false;
	checkCaps = (D3DTEXOPCAPS_MODULATE);
	if ((D3DGlobal.hD3DCaps.TextureOpCaps & checkCaps) != checkCaps) bCombineSupportARB = false;
	checkCaps = (D3DTEXOPCAPS_ADD);
	if ((D3DGlobal.hD3DCaps.TextureOpCaps & checkCaps) != checkCaps) bCombineSupportARB = false;
	checkCaps = (D3DTEXOPCAPS_ADDSIGNED);
	if ((D3DGlobal.hD3DCaps.TextureOpCaps & checkCaps) != checkCaps) bCombineSupportARB = false;
	checkCaps = (D3DTEXOPCAPS_LERP);
	if ((D3DGlobal.hD3DCaps.TextureOpCaps & checkCaps) != checkCaps) bCombineSupportARB = false;
	bCombineSupportEXT = bCombineSupportARB;
	checkCaps = (D3DTEXOPCAPS_SUBTRACT);
	if ((D3DGlobal.hD3DCaps.TextureOpCaps & checkCaps) != checkCaps) bCombineSupportARB = false;
	if (bCombineSupportARB) ExtensionBuf.AddExtension( "GL_ARB_texture_env_combine" );

	checkCaps = (D3DTEXOPCAPS_DOTPRODUCT3);
	if ((D3DGlobal.hD3DCaps.TextureOpCaps & checkCaps) == checkCaps) ExtensionBuf.AddExtension( "GL_ARB_texture_env_dot3" );

	checkCaps = (D3DPTADDRESSCAPS_MIRROR);
	if ((D3DGlobal.hD3DCaps.TextureAddressCaps & checkCaps) == checkCaps) ExtensionBuf.AddExtension( "GL_ARB_texture_mirrored_repeat" );

	if (!(D3DGlobal.hD3DCaps.TextureCaps & D3DPTEXTURECAPS_POW2) &&
		(!(D3DGlobal.hD3DCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP) || !(D3DGlobal.hD3DCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP_POW2)) &&
		(!(D3DGlobal.hD3DCaps.TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP) || !(D3DGlobal.hD3DCaps.TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP_POW2))) {
			ExtensionBuf.AddExtension( "GL_ARB_texture_non_power_of_two" );
	}

	if ( D3DGlobal.supportsS3TC ) ExtensionBuf.AddExtension( "GL_ARB_texture_compression" );

	//we implement them at driver level
	ExtensionBuf.AddExtension( "GL_ARB_transpose_matrix" );

	checkCaps = (D3DPTADDRESSCAPS_MIRRORONCE);
	if ((D3DGlobal.hD3DCaps.TextureAddressCaps & checkCaps) == checkCaps) ExtensionBuf.AddExtension( "GL_ATI_texture_mirror_once" );

#ifdef ALLOW_CHS_EXTENSIONS
	//our own specific extensions 
	//use CHS prefix (CHain Studios)
	checkCaps = (D3DPTEXTURECAPS_MIPVOLUMEMAP);
	if ((D3DGlobal.hD3DCaps.TextureCaps & checkCaps) == checkCaps) ExtensionBuf.AddExtension( "GL_CHS_mipmap_texture3D" );
#endif

	//we implement them at driver level
	ExtensionBuf.AddExtension( "GL_EXT_abgr" );
	ExtensionBuf.AddExtension( "GL_EXT_bgra" );

	checkCaps = (D3DPBLENDCAPS_BLENDFACTOR);
	if ((D3DGlobal.hD3DCaps.SrcBlendCaps & checkCaps) == checkCaps) {
		if ((D3DGlobal.hD3DCaps.DestBlendCaps & checkCaps) == checkCaps) {
			ExtensionBuf.AddExtension( "GL_EXT_blend_color" );
		}
	}
	checkCaps = (D3DPMISCCAPS_BLENDOP);
	if ((D3DGlobal.hD3DCaps.PrimitiveMiscCaps & checkCaps) == checkCaps) {
		ExtensionBuf.AddExtension( "GL_EXT_blend_minmax" );
		ExtensionBuf.AddExtension( "GL_EXT_blend_subtract" );
	}
	
	//we implement them at driver level
	ExtensionBuf.AddExtension( "GL_EXT_compiled_vertex_array" );
	ExtensionBuf.AddExtension( "GL_EXT_draw_range_elements" );
	ExtensionBuf.AddExtension( "GL_EXT_multi_draw_arrays" );
	ExtensionBuf.AddExtension( "GL_EXT_fog_coord" );
	ExtensionBuf.AddExtension( "GL_EXT_packed_pixels" );
	ExtensionBuf.AddExtension( "GL_EXT_secondary_color" );

	checkCaps = (D3DPTEXTURECAPS_VOLUMEMAP);
	if ((D3DGlobal.hD3DCaps.TextureCaps & checkCaps) == checkCaps) ExtensionBuf.AddExtension( "GL_EXT_texture3D" );
	if ( D3DGlobal.supportsS3TC ) ExtensionBuf.AddExtension( "GL_EXT_texture_compression_s3tc" );
	checkCaps = (D3DPTEXTURECAPS_CUBEMAP);
	if ((D3DGlobal.hD3DCaps.TextureCaps & checkCaps) == checkCaps) ExtensionBuf.AddExtension( "GL_EXT_texture_cube_map" );
	
	checkCaps = (D3DTEXOPCAPS_ADD);
	if ((D3DGlobal.hD3DCaps.TextureOpCaps & checkCaps) == checkCaps) ExtensionBuf.AddExtension( "GL_EXT_texture_env_add" );
	if (bCombineSupportEXT) ExtensionBuf.AddExtension( "GL_EXT_texture_env_combine" );
	checkCaps = (D3DTEXOPCAPS_DOTPRODUCT3);
	if ((D3DGlobal.hD3DCaps.TextureOpCaps & checkCaps) == checkCaps) ExtensionBuf.AddExtension( "GL_EXT_texture_env_dot3" );

	if (D3DGlobal.hD3DCaps.MaxAnisotropy > 1) ExtensionBuf.AddExtension( "GL_EXT_texture_filter_anisotropic" );

	checkCaps = (D3DPRASTERCAPS_MIPMAPLODBIAS);
	if ((D3DGlobal.hD3DCaps.RasterCaps & checkCaps) == checkCaps) {
		ExtensionBuf.AddExtension( "GL_EXT_texture_lod" );		//assume per-object bias
		ExtensionBuf.AddExtension( "GL_EXT_texture_lod_bias" );	//assume per-stage bias
	}

	ExtensionBuf.AddExtension( "GL_EXT_texture_object" );	//GL 1.0 legacy, but exists in modern drivers, therefore we add it too

	checkCaps = (D3DSTENCILCAPS_TWOSIDED);
	if ((D3DGlobal.hD3DCaps.StencilCaps & checkCaps) == checkCaps) ExtensionBuf.AddExtension( "GL_EXT_stencil_two_side" );
	checkCaps = (D3DSTENCILCAPS_INCRSAT|D3DSTENCILCAPS_DECRSAT);
	if ((D3DGlobal.hD3DCaps.StencilCaps & checkCaps) == checkCaps) ExtensionBuf.AddExtension( "GL_EXT_stencil_wrap" );

	checkCaps = (D3DPTADDRESSCAPS_MIRROR);
	if ((D3DGlobal.hD3DCaps.TextureAddressCaps & checkCaps) == checkCaps) ExtensionBuf.AddExtension( "GL_IBM_texture_mirrored_repeat" );


	checkCaps = (D3DPBLENDCAPS_SRCCOLOR|D3DPBLENDCAPS_INVSRCCOLOR);
	if ((D3DGlobal.hD3DCaps.SrcBlendCaps & checkCaps) == checkCaps) {
		checkCaps = (D3DPBLENDCAPS_DESTCOLOR|D3DPBLENDCAPS_INVDESTCOLOR);
		if ((D3DGlobal.hD3DCaps.DestBlendCaps & checkCaps) == checkCaps) {
			ExtensionBuf.AddExtension( "GL_NV_blend_square" );
		}
	}

	//we implement it at driver level
	ExtensionBuf.AddExtension( "GL_NV_texgen_reflection" );

//	checkCaps = (D3DCAPS2_CANAUTOGENMIPMAP);
	//if ((D3DGlobal.hD3DCaps.Caps2 & checkCaps) == checkCaps) ExtensionBuf.AddExtension( "GL_SGIS_generate_mipmap" );

	//For Quake2 that won't use ARB extension
	if (D3DGlobal.maxActiveTMU > 1) ExtensionBuf.AddExtension( "GL_SGIS_multitexture" );

	//an alias to GL_EXT_multi_draw_arrays
	ExtensionBuf.AddExtension( "GL_SUN_multi_draw_arrays" );

	//we implement it at driver level
	ExtensionBuf.AddExtension( "WGL_ARB_extensions_string" );
	ExtensionBuf.AddExtension( "WGL_EXT_swap_control" );

	//add WGL extensions
	WExtensionBuf.AddExtension( "WGL_ARB_extensions_string" );
	WExtensionBuf.AddExtension( "WGL_EXT_swap_control" );

	D3DGlobal.szExtensions = ExtensionBuf.CopyBuffer();
	D3DGlobal.szWExtensions = WExtensionBuf.CopyBuffer();
}

//=========================================
// wglGetProcAddress
//-----------------------------------------
// Return a requested extension proc address
//=========================================
OPENGL_API PROC WINAPI wrap_wglGetProcAddress( LPCSTR s )
{
	static size_t stubAddress = 0xBAD00000;
	const char *pszDisabledExt = NULL;

	for (int i = 0; ; ++i) {
		// no more entrypoints
		if (!glext_EntryPoints[i].name) 
			break;

		if (glext_EntryPoints[i].enabled < 0)
			glext_EntryPoints[i].enabled = D3DGlobal_GetRegistryValue(glext_EntryPoints[i].extname, "Extensions", glext_EntryPoints[i].enabled==-1 ? 0 : 1);

		if (!strcmp(s, glext_EntryPoints[i].name)) {
			if (!glext_EntryPoints[i].enabled) {
				pszDisabledExt = glext_EntryPoints[i].extname;
				break;
			} else {
				logPrintf("wglGetProcAddress: queried proc '%s'\n", s);
				return glext_EntryPoints[i].func;
			}
		}
	}

	++stubAddress;

	if (pszDisabledExt)
		logPrintf("WARNING: wglGetProcAddress: queried disabled proc '%s' (extension '%s') (stub = 0x%X)\n", s, pszDisabledExt, stubAddress);
	else
		logPrintf("WARNING: wglGetProcAddress: queried unknown proc '%s' (stub = 0x%X)\n", s, stubAddress);

	return (PROC)stubAddress;
}

OPENGL_API PROC WINAPI wrap_wglGetDefaultProcAddress( LPCSTR s )
{
	return wrap_wglGetProcAddress(s);
}

