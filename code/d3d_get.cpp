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
#include "d3d_array.hpp"
#include "d3d_matrix_stack.hpp"
#include "d3d_texture.hpp"

//==================================================================================
// Get* functions
//----------------------------------------------------------------------------------
// We will maintain our own state and return it, not query Direct3D state. This is
// essential if we use pure device that actually forbids any get requests
//==================================================================================

OPENGL_API const GLubyte * WINAPI glGetString( GLenum name )
{
	if (!D3DGlobal.initialized)
		return (GLubyte*)"";

	switch (name)
	{
	case GL_VENDOR: return (const GLubyte*)WRAPPER_GL_VENDOR_STRING;
	case GL_VERSION: return (const GLubyte*)WRAPPER_GL_VERSION_STRING;
	case GL_EXTENSIONS: return (const GLubyte*)D3DGlobal.szExtensions;
	case GL_RENDERER: return (const GLubyte*)D3DGlobal.szRendererName;
	case GL_WRAPPER_NAME_CHS: return (const GLubyte*)WRAPPER_GL_WRAPPER_NAME_STRING;
	case GL_WRAPPER_VERSION_CHS: return (const GLubyte*)WRAPPER_GL_WRAPPER_VERSION_STRING;
	default: return (GLubyte*)"";
	}
}

OPENGL_API GLenum WINAPI glGetError()
{
	if (SUCCEEDED(D3DGlobal.lastError))
		return GL_NO_ERROR;

	GLenum oglErrorCode;

	switch (D3DGlobal.lastError) {
	case D3DERR_OUTOFVIDEOMEMORY:
	case E_OUTOFMEMORY:
		oglErrorCode = GL_OUT_OF_MEMORY;
		break;
	case D3DERR_UNSUPPORTEDALPHAARG:
	case D3DERR_UNSUPPORTEDALPHAOPERATION:
	case D3DERR_UNSUPPORTEDCOLORARG:
	case D3DERR_UNSUPPORTEDCOLOROPERATION:
	case D3DERR_UNSUPPORTEDTEXTUREFILTER:
	case D3DERR_UNSUPPORTEDFACTORVALUE:
	case D3DERR_WRONGTEXTUREFORMAT:
	case E_INVALIDARG:
		oglErrorCode = GL_INVALID_VALUE;
		break;
	case E_STACK_OVERFLOW:
		oglErrorCode = GL_STACK_OVERFLOW;
		break;
	case E_STACK_UNDERFLOW:
		oglErrorCode = GL_STACK_UNDERFLOW;
		break;
	case E_INVALID_ENUM:
		oglErrorCode = GL_INVALID_ENUM;
		break;
	case E_INVALID_OPERATION:
	default:
		oglErrorCode = GL_INVALID_OPERATION;
		break;
	}

	D3DGlobal.lastError = S_OK;
	return oglErrorCode;
}

OPENGL_API HRESULT WINAPI glGetD3DError()
{
	return D3DGlobal.lastError;
}

#pragma warning( disable: 4310 )	// cast truncates constant value

template<typename T> static void glGet( GLenum pname, T *params )
{
	if (!params) return;

	switch (pname) {
	case GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB:
	case GL_MAX_TEXTURE_SIZE:
		params[0] = (T)(QINDIEGL_MIN( D3DGlobal.hD3DCaps.MaxTextureWidth, D3DGlobal.hD3DCaps.MaxTextureHeight ));
		break;
	case GL_MAX_3D_TEXTURE_SIZE:
		params[0] = (T)((D3DGlobal.hD3DCaps.TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP) ? D3DGlobal.hD3DCaps.MaxVolumeExtent : 0 );
		break;
	case GL_MAX_TEXTURE_COORDS_ARB:
	case GL_MAX_TEXTURE_UNITS_ARB:
	case GL_MAX_TEXTURES_SGIS:
		if (!D3DGlobal_GetRegistryValue("GL_ARB_multitexture", "Extensions", 0))
			params[0] = (T)1;
		else
			params[0] = (T)D3DGlobal.maxActiveTMU;
		break;
	case GL_SELECTED_TEXTURE_SGIS:
		params[0] = (T)(D3DState.TextureState.currentTMU + GL_TEXTURE0_SGIS);
		break;
	case GL_ACTIVE_TEXTURE_ARB:
		params[0] = (T)(D3DState.TextureState.currentTMU + GL_TEXTURE0_ARB);
		break;
	case GL_CLIENT_ACTIVE_TEXTURE_ARB:
		params[0] = (T)(D3DState.ClientTextureState.currentClientTMU + GL_TEXTURE0_ARB);
		break;
	case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
		params[0] = (T)(D3DGlobal.hD3DCaps.MaxAnisotropy);
		break;
	case GL_MAX_TEXTURE_LOD_BIAS_EXT:
		params[0] = (T)16.0;	//FIXME: compute log2(MaxTextureSize)?
		break;
	case GL_POINT_SIZE_MIN:
		params[0] = (T)1.0;
		break;
	case GL_POINT_SIZE_MAX:
		params[0] = (T)D3DGlobal.hD3DCaps.MaxPointSize;
		break;
	case GL_MAX_LIGHTS:
		params[0] = (T)D3DGlobal.maxActiveLights;
		break;
	case GL_MAX_VIEWPORT_DIMS:
		params[0] = (T)0;
		params[1] = (T)0;
		params[2] = (T)D3DGlobal.hCurrentMode.Width;
		params[3] = (T)D3DGlobal.hCurrentMode.Height;
		break;
	case GL_MAX_ATTRIB_STACK_DEPTH:
	case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
		params[0] = (T)1;
		break;
	case GL_MAX_CLIP_PLANES:
		params[0] = (T)(QINDIEGL_MIN( D3DGlobal.hD3DCaps.MaxUserClipPlanes, IMPL_MAX_CLIP_PLANES ));
		break;

	case GL_RED_BITS:
		params[0] = (T)D3DGlobal.rgbaBits[0];
		break;
	case GL_GREEN_BITS:
		params[0] = (T)D3DGlobal.rgbaBits[1];
		break;
	case GL_BLUE_BITS:
		params[0] = (T)D3DGlobal.rgbaBits[2];
		break;
	case GL_ALPHA_BITS:
		params[0] = (T)D3DGlobal.rgbaBits[3];
		break;
	case GL_DEPTH_BITS:
		params[0] = (T)D3DGlobal.depthBits;
		break;
	case GL_STENCIL_BITS:
		params[0] = (T)D3DGlobal.stencilBits;
		break;
	case GL_ACCUM_RED_BITS:
	case GL_ACCUM_GREEN_BITS:
	case GL_ACCUM_BLUE_BITS:
	case GL_ACCUM_ALPHA_BITS:
		params[0] = (T)0;
		break;
	case GL_SAMPLE_BUFFERS_ARB:
		params[0] = (T)(D3DGlobal.multiSamples > 0 ? 1 : 0);
		break;
	case GL_SAMPLES_ARB:
		params[0] = (T)D3DGlobal.multiSamples;
		break;

	case GL_UNPACK_SWAP_BYTES:
		params[0] = (T)D3DState.ClientPixelStoreState.unpackSwapBytes;
		break;
	case GL_UNPACK_LSB_FIRST:
		params[0] = (T)D3DState.ClientPixelStoreState.unpackLSBfirst;
		break;
	case GL_UNPACK_ROW_LENGTH:
		params[0] = (T)D3DState.ClientPixelStoreState.unpackRowLength;
		break;
	case GL_UNPACK_IMAGE_HEIGHT:
		params[0] = (T)D3DState.ClientPixelStoreState.unpackImageHeight;
		break;
	case GL_UNPACK_SKIP_ROWS:
		params[0] = (T)D3DState.ClientPixelStoreState.unpackSkipRows;
		break;
	case GL_UNPACK_SKIP_PIXELS:
		params[0] = (T)D3DState.ClientPixelStoreState.unpackSkipPixels;
		break;
	case GL_UNPACK_SKIP_IMAGES:
		params[0] = (T)D3DState.ClientPixelStoreState.unpackSkipImages;
		break;
	case GL_UNPACK_ALIGNMENT:
		params[0] = (T)D3DState.ClientPixelStoreState.unpackAlignment;
		break;
	case GL_PACK_SWAP_BYTES:
		params[0] = (T)D3DState.ClientPixelStoreState.packSwapBytes;
		break;
	case GL_PACK_LSB_FIRST:
		params[0] = (T)D3DState.ClientPixelStoreState.packLSBfirst;
		break;
	case GL_PACK_ROW_LENGTH:
		params[0] = (T)D3DState.ClientPixelStoreState.packRowLength;
		break;
	case GL_PACK_IMAGE_HEIGHT:
		params[0] = (T)D3DState.ClientPixelStoreState.packImageHeight;
		break;
	case GL_PACK_SKIP_ROWS:
		params[0] = (T)D3DState.ClientPixelStoreState.packSkipRows;
		break;
	case GL_PACK_SKIP_PIXELS:
		params[0] = (T)D3DState.ClientPixelStoreState.packSkipPixels;
		break;
	case GL_PACK_SKIP_IMAGES:
		params[0] = (T)D3DState.ClientPixelStoreState.packSkipImages;
		break;
	case GL_PACK_ALIGNMENT:
		params[0] = (T)D3DState.ClientPixelStoreState.packAlignment;
		break;
	case GL_MAP_COLOR:
		params[0] = (T)D3DState.ClientPixelStoreState.transferMapColor;
		break;
	case GL_RED_SCALE:
		params[0] = (T)D3DState.ClientPixelStoreState.transferRedScale;
		break;
	case GL_RED_BIAS:
		params[0] = (T)D3DState.ClientPixelStoreState.transferRedBias;
		break;
	case GL_GREEN_SCALE:
		params[0] = (T)D3DState.ClientPixelStoreState.transferGreenScale;
		break;
	case GL_GREEN_BIAS:
		params[0] = (T)D3DState.ClientPixelStoreState.transferGreenBias;
		break;
	case GL_BLUE_SCALE:
		params[0] = (T)D3DState.ClientPixelStoreState.transferBlueScale;
		break;
	case GL_BLUE_BIAS:
		params[0] = (T)D3DState.ClientPixelStoreState.transferBlueBias;
		break;
	case GL_ALPHA_SCALE:
		params[0] = (T)D3DState.ClientPixelStoreState.transferAlphaScale;
		break;
	case GL_ALPHA_BIAS:
		params[0] = (T)D3DState.ClientPixelStoreState.transferAlphaBias;
		break;
	case GL_MAX_PIXEL_MAP_TABLE:
		params[0] = (T)IMPL_MAX_PIXEL_MAP_TABLE;
		break;
	case GL_PIXEL_MAP_R_TO_R_SIZE:
		params[0] = (T)D3DState.ClientPixelStoreState.pixelmapSizeRtoR;
		break;
	case GL_PIXEL_MAP_G_TO_G_SIZE:
		params[0] = (T)D3DState.ClientPixelStoreState.pixelmapSizeGtoG;
		break;
	case GL_PIXEL_MAP_B_TO_B_SIZE:
		params[0] = (T)D3DState.ClientPixelStoreState.pixelmapSizeBtoB;
		break;
	case GL_PIXEL_MAP_A_TO_A_SIZE:
		params[0] = (T)D3DState.ClientPixelStoreState.pixelmapSizeAtoA;
		break;
	case GL_PIXEL_MAP_I_TO_I_SIZE:
	case GL_PIXEL_MAP_S_TO_S_SIZE:
	case GL_PIXEL_MAP_I_TO_R_SIZE:
	case GL_PIXEL_MAP_I_TO_G_SIZE:
	case GL_PIXEL_MAP_I_TO_B_SIZE:
	case GL_PIXEL_MAP_I_TO_A_SIZE:
		params[0] = (T)0;
		break;

	case GL_LIGHT_MODEL_LOCAL_VIEWER:
		params[0] = (T)D3DState.LightingState.lightModelLocalViewer;
		break;
	case GL_LIGHT_MODEL_AMBIENT:
		params[0] = (T)((float)((D3DState.LightingState.lightModelAmbient >> 16) & 0xFF) / 255.0f );
		params[1] = (T)((float)((D3DState.LightingState.lightModelAmbient >> 8) & 0xFF) / 255.0f );
		params[2] = (T)((float)((D3DState.LightingState.lightModelAmbient) & 0xFF) / 255.0f );
		params[3] = (T)((float)((D3DState.LightingState.lightModelAmbient >> 24) & 0xFF) / 255.0f );
		return;
	case GL_COLOR_MATERIAL_FACE:
		params[0] = (T)GL_FRONT_AND_BACK;
		break;
	case GL_COLOR_MATERIAL_PARAMETER:
		params[0] = (T)D3DState.LightingState.colorMaterial;
		break;

	case GL_FOG_COLOR:
		params[0] = (T)((float)((D3DState.FogState.fogColor >> 16) & 0xFF) / 255.0f );
		params[1] = (T)((float)((D3DState.FogState.fogColor >> 8) & 0xFF) / 255.0f );
		params[2] = (T)((float)((D3DState.FogState.fogColor) & 0xFF) / 255.0f );
		params[3] = (T)((float)((D3DState.FogState.fogColor >> 24) & 0xFF) / 255.0f );
		return;
	case GL_FOG_START:
		params[0] = (T)D3DState.FogState.fogStart;
		return;
	case GL_FOG_END:
		params[0] = (T)D3DState.FogState.fogEnd;
		return;
	case GL_FOG_DENSITY:
		params[0] = (T)D3DState.FogState.fogDensity;
		return;
	case GL_FOG_INDEX:
		params[0] = (T)0;
		return;
	case GL_FOG_COORDINATE_SOURCE_EXT:
		params[0] = (T)(D3DState.FogState.fogCoordMode ? GL_FOG_COORDINATE_EXT : GL_FRAGMENT_DEPTH_EXT);
	case GL_FOG_MODE:
		switch (D3DState.FogState.fogMode) {
		default:
		case D3DFOG_EXP:
			params[0] = (T)GL_EXP;
			break;
		case D3DFOG_EXP2:
			params[0] = (T)GL_EXP2;
			break;
		case D3DFOG_LINEAR:
			params[0] = (T)GL_LINEAR;
			break;
		}
		return;
	case GL_FOG_HINT:
		switch (D3DState.HintState.fogHint) {
		default:
		case 0:
			params[0] = (T)GL_DONT_CARE;
			break;
		case 1:
			params[0] = (T)GL_NICEST;
			break;
		case 2:
			params[0] = (T)GL_FASTEST;
			break;
		}
		return;

	case GL_MODELVIEW_MATRIX:
		{
			D3DMATRIX *pm = D3DGlobal.modelviewMatrixStack->top();
			for (int i = 0; i < 16; i++) {
				params[i] = (T)pm->m[i/4][i%4];
			}
			return;
		}
	case GL_PROJECTION_MATRIX:
		{
			D3DMATRIX *pm = D3DGlobal.projectionMatrixStack->top();
			for (int i = 0; i < 16; i++) {
				params[i] = (T)pm->m[i/4][i%4];
			}
			return;
		}
	case GL_TEXTURE_MATRIX:
		{
			D3DMATRIX *pm = D3DGlobal.textureMatrixStack[D3DState.TextureState.currentTMU]->top();
			for (int i = 0; i < 16; i++) {
				params[i] = (T)pm->m[i/4][i%4];
			}
			return;
		}
	case GL_TRANSPOSE_MODELVIEW_MATRIX_ARB:
		{
			D3DMATRIX *pm = D3DGlobal.modelviewMatrixStack->top();
			for (int i = 0; i < 16; i++) {
				params[i] = (T)pm->m[i%4][i/4];
			}
			return;
		}
	case GL_TRANSPOSE_PROJECTION_MATRIX_ARB:
		{
			D3DMATRIX *pm = D3DGlobal.projectionMatrixStack->top();
			for (int i = 0; i < 16; i++) {
				params[i] = (T)pm->m[i%4][i/4];
			}
			return;
		}
	case GL_TRANSPOSE_TEXTURE_MATRIX_ARB:
		{
			D3DMATRIX *pm = D3DGlobal.textureMatrixStack[D3DState.TextureState.currentTMU]->top();
			for (int i = 0; i < 16; i++) {
				params[i] = (T)pm->m[i%4][i/4];
			}
			return;
		}
	case GL_MODELVIEW_STACK_DEPTH:
		params[0] = (T)D3DGlobal.modelviewMatrixStack->stack_depth();
		return; 
	case GL_PROJECTION_STACK_DEPTH:
		params[0] = (T)D3DGlobal.projectionMatrixStack->stack_depth();
		return; 
	case GL_TEXTURE_STACK_DEPTH:
		params[0] = (T)D3DGlobal.textureMatrixStack[D3DState.TextureState.currentTMU]->stack_depth();
		return; 
	case GL_MAX_MODELVIEW_STACK_DEPTH:
		params[0] = (T)D3DGlobal.modelviewMatrixStack->max_stack_depth();
		return; 
	case GL_MAX_PROJECTION_STACK_DEPTH: 
		params[0] = (T)D3DGlobal.projectionMatrixStack->max_stack_depth();
		return; 
	case GL_MAX_TEXTURE_STACK_DEPTH:
		params[0] = (T)D3DGlobal.textureMatrixStack[D3DState.TextureState.currentTMU]->max_stack_depth();
		return;
	case GL_BLEND_SRC:
		params[0] = (T)(D3DState.ColorBufferState.glBlendSrc);
		break;
	case GL_BLEND_DST:
		params[0] = (T)(D3DState.ColorBufferState.glBlendDst);
		break;

	case GL_VIEWPORT:
		params[0] = (T)D3DState.viewport.X;
		params[1] = (T)(D3DGlobal.hCurrentMode.Height - (D3DState.viewport.Height + D3DState.viewport.Y));
		params[2] = (T)D3DState.viewport.Width;
		params[3] = (T)D3DState.viewport.Height;
		return;		
	case GL_DEPTH_RANGE:
		params[0] = (T)D3DState.viewport.MinZ;
		params[1] = (T)D3DState.viewport.MaxZ;
		return;
	case GL_SCISSOR_BOX:
		params[0] = (T)D3DState.ScissorState.scissorRect.left;
		params[1] = (T)(D3DGlobal.hCurrentMode.Height - D3DState.ScissorState.scissorRect.bottom);
		params[2] = (T)(D3DState.ScissorState.scissorRect.right - D3DState.ScissorState.scissorRect.left);
		params[3] = (T)(D3DState.ScissorState.scissorRect.bottom - D3DState.ScissorState.scissorRect.top);
		return;		

	case GL_CURRENT_NORMAL:
		params[0] = (T)D3DState.CurrentState.currentNormal[0];
		params[1] = (T)D3DState.CurrentState.currentNormal[1];
		params[2] = (T)D3DState.CurrentState.currentNormal[2];
		return;		
	case GL_CURRENT_COLOR:
		params[0] = (T)((float)((D3DState.CurrentState.currentColor >> 16) & 0xFF) / 255.0f );
		params[1] = (T)((float)((D3DState.CurrentState.currentColor >> 8) & 0xFF) / 255.0f );
		params[2] = (T)((float)((D3DState.CurrentState.currentColor) & 0xFF) / 255.0f );
		params[3] = (T)((float)((D3DState.CurrentState.currentColor >> 24) & 0xFF) / 255.0f );
		return;	
	case GL_CURRENT_SECONDARY_COLOR_EXT:
		params[0] = (T)((float)((D3DState.CurrentState.currentColor2 >> 16) & 0xFF) / 255.0f );
		params[1] = (T)((float)((D3DState.CurrentState.currentColor2 >> 8) & 0xFF) / 255.0f );
		params[2] = (T)((float)((D3DState.CurrentState.currentColor2) & 0xFF) / 255.0f );
		params[3] = (T)((float)((D3DState.CurrentState.currentColor >> 24) & 0xFF) / 255.0f );
		break;
	case GL_CURRENT_FOG_COORDINATE_EXT:
		params[0] = (T)((float)((D3DState.CurrentState.currentColor >> 24) & 0xFF) / 255.0f );
		return;	
	case GL_CURRENT_TEXTURE_COORDS:
		params[0] = (T)D3DState.CurrentState.currentTexCoord[D3DState.TextureState.currentTMU][0];
		params[1] = (T)D3DState.CurrentState.currentTexCoord[D3DState.TextureState.currentTMU][1];
		params[2] = (T)D3DState.CurrentState.currentTexCoord[D3DState.TextureState.currentTMU][2];
		params[3] = (T)D3DState.CurrentState.currentTexCoord[D3DState.TextureState.currentTMU][3];
		return;	
	case GL_BLEND_COLOR_EXT:
		params[0] = (T)((float)((D3DState.ColorBufferState.alphaBlendColorARGB >> 16) & 0xFF) / 255.0f );
		params[1] = (T)((float)((D3DState.ColorBufferState.alphaBlendColorARGB >> 8) & 0xFF) / 255.0f );
		params[2] = (T)((float)((D3DState.ColorBufferState.alphaBlendColorARGB) & 0xFF) / 255.0f );
		params[3] = (T)((float)((D3DState.ColorBufferState.alphaBlendColorARGB >> 24) & 0xFF) / 255.0f );
		break;

	case GL_TEXTURE_1D_BINDING_EXT:
		if (D3DTextureObject* pTex = D3DState.TextureState.currentTexture[D3DState.TextureState.currentTMU][D3D_TEXTARGET_1D]) {
			params[0] = (T)pTex->GetGLIndex();
		} else {
			params[0] = (T)0;
		}
		break;
	case GL_TEXTURE_2D_BINDING_EXT:
		if (D3DTextureObject* pTex = D3DState.TextureState.currentTexture[D3DState.TextureState.currentTMU][D3D_TEXTARGET_2D]) {
			params[0] = (T)pTex->GetGLIndex();
		} else {
			params[0] = (T)0;
		}
		break;
	case GL_TEXTURE_3D_BINDING_EXT:
		if (D3DTextureObject* pTex = D3DState.TextureState.currentTexture[D3DState.TextureState.currentTMU][D3D_TEXTARGET_3D]) {
			params[0] = (T)pTex->GetGLIndex();
		} else {
			params[0] = (T)0;
		}
		break;

	case GL_RGBA_MODE:
		params[0] = (T)TRUE;
		break;
	case GL_INDEX_MODE:
		params[0] = (T)FALSE;
		break;

	case GL_ARRAY_ELEMENT_LOCK_FIRST_EXT:
		params[0] = (T)D3DGlobal.pVABuffer->GetLockFirst();
		break;
	case GL_ARRAY_ELEMENT_LOCK_COUNT_EXT:
		params[0] = (T)D3DGlobal.pVABuffer->GetLockCount();
		break;
	case GL_MAX_ELEMENTS_VERTICES_EXT:
		params[0] = (T)0xFFFFFF;	//FIXME: tune this
		break;
	case GL_MAX_ELEMENTS_INDICES_EXT:
		params[0] = (T)D3DGlobal.hD3DCaps.MaxVertexIndex;
		break;

	default:
		logPrintf("WARNING: glGet(0x%x) is not supported\n", pname);
		params[0] = (T)0;
		break;
	}
}

#pragma warning( default: 4310 )

OPENGL_API void WINAPI glGetIntegerv( GLenum pname, GLint *params )
{
	glGet( pname, params );
}

OPENGL_API void WINAPI glGetBooleanv( GLenum pname, GLboolean *params )
{
	glGet( pname, params );
}

OPENGL_API void WINAPI glGetDoublev( GLenum pname, GLdouble *params )
{
	glGet( pname, params );
}

OPENGL_API void WINAPI glGetFloatv( GLenum pname, GLfloat *params )
{
	glGet( pname, params );
}

OPENGL_API void WINAPI glGetPointerv( GLenum pname, GLvoid* *params )
{
	switch (pname) {
	case GL_SELECTION_BUFFER_POINTER:
	case GL_FEEDBACK_BUFFER_POINTER:
		logPrintf("WARNING: glGetPointerv(0x%x): neither select nor feedback buffer is implemented\n", pname);
		*params = NULL;
		break;

	case GL_VERTEX_ARRAY_POINTER:
		*params = (GLvoid*)D3DState.ClientVertexArrayState.vertexInfo.data;
		break;
	case GL_NORMAL_ARRAY_POINTER:
		*params = (GLvoid*)D3DState.ClientVertexArrayState.normalInfo.data;
		break;
	case GL_COLOR_ARRAY_POINTER:
		*params = (GLvoid*)D3DState.ClientVertexArrayState.colorInfo.data;
		break;
	case GL_SECONDARY_COLOR_ARRAY_POINTER_EXT:
		*params = (GLvoid*)D3DState.ClientVertexArrayState.color2Info.data;
		break;
	case GL_FOG_COORDINATE_ARRAY_POINTER_EXT:
		*params = (GLvoid*)D3DState.ClientVertexArrayState.color2Info.data;
		break;
	case GL_TEXTURE_COORD_ARRAY_POINTER:
		*params = (GLvoid*)D3DState.ClientVertexArrayState.texCoordInfo[D3DState.ClientTextureState.currentClientTMU].data;
		break;

	default:
		logPrintf("WARNING: glGetPointerv(0x%x) is not supported\n", pname);
		break;
	}
}
