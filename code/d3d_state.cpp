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
#include "d3d_texture.hpp"
#include "d3d_matrix_stack.hpp"
#include "d3d_combiners.hpp"

D3DState_t D3DState;
static D3DState_t D3DStateCopy;
static GLbitfield D3DStateCopyMask = 0;
static GLbitfield D3DStateClientCopyMask = 0;

extern void SelectTexGenFunc( int stage, int coord );

static void D3DState_CopyClient( const D3DState_t *src, D3DState_t *dst, GLbitfield mask )
{
	if (mask & GL_CLIENT_ALL_ATTRIB_BITS) {
		memcpy(&dst->ClientTextureState, &src->ClientTextureState, sizeof(src->ClientTextureState));
	}
	if (mask & GL_CLIENT_PIXEL_STORE_BIT) {
		memcpy(&dst->ClientPixelStoreState, &src->ClientPixelStoreState, sizeof(src->ClientPixelStoreState));
	}
}

static void D3DState_Copy( const D3DState_t *src, D3DState_t *dst, GLbitfield mask )
{
	if (mask & GL_COLOR_BUFFER_BIT) {
		memcpy(&dst->ColorBufferState, &src->ColorBufferState, sizeof(src->ColorBufferState));
		if (!(mask & GL_ENABLE_BIT)) {
			dst->EnableState.alphaTestEnabled = src->EnableState.alphaTestEnabled;
			dst->EnableState.alphaBlendEnabled = src->EnableState.alphaBlendEnabled;
			dst->EnableState.ditherEnabled = src->EnableState.ditherEnabled;
		}
	}
	if (mask & GL_CURRENT_BIT) {
		memcpy(&dst->CurrentState, &src->CurrentState, sizeof(src->CurrentState));
	}
	if (mask & GL_DEPTH_BUFFER_BIT) {
		memcpy(&dst->DepthBufferState, &src->DepthBufferState, sizeof(src->DepthBufferState));
		if (!(mask & GL_ENABLE_BIT)) {
			dst->EnableState.depthTestEnabled = src->EnableState.depthTestEnabled;
		}
	}
	if (mask & GL_ENABLE_BIT) {
		memcpy(&dst->EnableState, &src->EnableState, sizeof(src->EnableState));
	}
	if (mask & GL_FOG_BIT) {
		memcpy(&dst->FogState, &src->FogState, sizeof(src->FogState));
		if (!(mask & GL_ENABLE_BIT)) {
			dst->EnableState.fogEnabled = src->EnableState.fogEnabled;
		}
	}
	if (mask & GL_HINT_BIT) {
		memcpy(&dst->HintState, &src->HintState, sizeof(src->HintState));
	}
	if (mask & GL_LIGHTING_BIT) {
		memcpy(&dst->LightingState, &src->LightingState, sizeof(src->LightingState));
		if (!(mask & GL_ENABLE_BIT)) {
			dst->EnableState.lightingEnabled = src->EnableState.lightingEnabled;
			dst->EnableState.colorMaterialEnabled = src->EnableState.colorMaterialEnabled;
			memcpy(&dst->EnableState.lightEnabled[0], &src->EnableState.lightEnabled[0], sizeof(src->EnableState.lightEnabled));
		}
	}
	if (mask & GL_POINT_BIT) {
		memcpy(&dst->PointState, &src->PointState, sizeof(src->PointState));
	}
	if (mask & GL_POLYGON_BIT) {
		memcpy(&dst->PolygonState, &src->PolygonState, sizeof(src->PolygonState));
		if (!(mask & GL_ENABLE_BIT)) {
			dst->EnableState.cullEnabled = src->EnableState.cullEnabled;
			dst->EnableState.depthBiasEnabled = src->EnableState.depthBiasEnabled;
		}
	}
	if (mask & GL_SCISSOR_BIT) {
		memcpy(&dst->ScissorState, &src->ScissorState, sizeof(src->ScissorState));
		if (!(mask & GL_ENABLE_BIT)) {
			dst->EnableState.scissorEnabled = src->EnableState.scissorEnabled;
		}
	}
	if (mask & GL_STENCIL_BUFFER_BIT) {
		memcpy(&dst->StencilBufferState, &src->StencilBufferState, sizeof(src->StencilBufferState));
		if (!(mask & GL_ENABLE_BIT)) {
			dst->EnableState.stencilTestEnabled = src->EnableState.stencilTestEnabled;
			dst->EnableState.twoSideStencilEnabled = src->EnableState.twoSideStencilEnabled;
		}
	}
	if (mask & GL_TEXTURE_BIT) {
		memcpy(&dst->TextureState, &src->TextureState, sizeof(src->TextureState));
		if (!(mask & GL_ENABLE_BIT)) {
			memcpy(&dst->EnableState.textureEnabled[0], &src->EnableState.textureEnabled[0], sizeof(src->EnableState.textureEnabled));
			memcpy(&dst->EnableState.textureTargetEnabled[0][0], &src->EnableState.textureTargetEnabled[0][0], sizeof(src->EnableState.textureTargetEnabled));
			memcpy(&dst->EnableState.texGenEnabled[0], &src->EnableState.texGenEnabled[0], sizeof(src->EnableState.texGenEnabled));
		}
	}
	if (mask & GL_TRANSFORM_BIT) {
		memcpy(&dst->TransformState, &src->TransformState, sizeof(src->TransformState));
		if (!(mask & GL_ENABLE_BIT)) {
			dst->EnableState.normalizeEnabled = src->EnableState.normalizeEnabled;
			dst->EnableState.clipPlaneEnableMask = src->EnableState.clipPlaneEnableMask;
		}
	}
	if (mask & GL_VIEWPORT_BIT) {
		memcpy(&dst->viewport, &src->viewport, sizeof(src->viewport));
	}
}

void D3DState_SetCullMode()
{
	if (!D3DGlobal.pDevice) return;
	HRESULT hr = S_OK;

	if (!D3DState.EnableState.cullEnabled) {
		hr = D3DGlobal.pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	} else {
		if (D3DState.PolygonState.frontFace == GL_CCW)
			hr = D3DGlobal.pDevice->SetRenderState(D3DRS_CULLMODE, (D3DState.PolygonState.cullMode == GL_BACK) ? D3DCULL_CW : D3DCULL_CCW );
		else
			hr = D3DGlobal.pDevice->SetRenderState(D3DRS_CULLMODE, (D3DState.PolygonState.cullMode == GL_BACK) ? D3DCULL_CCW : D3DCULL_CW );
	}
	if (FAILED(hr)) D3DGlobal.lastError = hr;
}

void D3DState_SetDepthBias()
{
	if (!D3DGlobal.pDevice) return;
	HRESULT hr = S_OK;

	if (!D3DState.EnableState.depthBiasEnabled) {
		 hr = D3DGlobal.pDevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, 0);
		 if (SUCCEEDED(hr)) hr = D3DGlobal.pDevice->SetRenderState(D3DRS_DEPTHBIAS, 0);
	} else {
		hr = D3DGlobal.pDevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, UTIL_FloatToDword(D3DState.PolygonState.depthBiasFactor));
		 if (SUCCEEDED(hr)) hr = D3DGlobal.pDevice->SetRenderState(D3DRS_DEPTHBIAS, UTIL_FloatToDword(D3DState.PolygonState.depthBiasUnits));
	}
	
	if (FAILED(hr)) D3DGlobal.lastError = hr;
}

bool D3DState_SetMatrixMode()
{
	switch ( D3DState.TransformState.matrixMode ) {
		case GL_MODELVIEW:
			D3DState.currentMatrixStack = D3DGlobal.modelviewMatrixStack;
			D3DState.currentMatrixModified = &D3DState.modelViewMatrixModified;
			return true;
		case GL_PROJECTION:
			D3DState.currentMatrixStack = D3DGlobal.projectionMatrixStack;
			D3DState.currentMatrixModified = &D3DState.projectionMatrixModified;
			return true;
		case GL_TEXTURE:
			D3DState.currentMatrixStack = D3DGlobal.textureMatrixStack[D3DState.TextureState.currentTMU];
			D3DState.currentMatrixModified = &D3DState.textureMatrixModified[D3DState.TextureState.currentTMU];
			return true;
		default:
			return false;
		}
}

void D3DState_AssureBeginScene()
{
	if (!D3DGlobal.sceneBegan) {
		HRESULT hr = D3DGlobal.pDevice->BeginScene();
		if (FAILED(hr)) {
			D3DGlobal.lastError = hr;
		} else {
			D3DGlobal.sceneBegan = true;
		}
	}
}

static void D3DState_SetTransform()
{
	HRESULT hr;
	assert( D3DGlobal.pD3D != nullptr );
	assert( D3DGlobal.pDevice != nullptr );

	if (D3DState.modelViewMatrixModified) {
		D3DState.modelViewMatrixModified = false;
		hr = D3DGlobal.pDevice->SetTransform( D3DTS_WORLD, D3DGlobal.modelviewMatrixStack->top() );
		if (FAILED(hr)) {
			D3DGlobal.lastError = hr;
			return;
		}
	}
	if (D3DState.projectionMatrixModified) {
		D3DState.projectionMatrixModified = false;
		hr = D3DGlobal.pDevice->SetTransform( D3DTS_PROJECTION, D3DGlobal.projectionMatrixStack->top() );
		if (FAILED(hr)) {
			D3DGlobal.lastError = hr;
			return;
		}
	}

	if (D3DState.TransformState.clippingModified) {
		D3DState.TransformState.clippingModified = FALSE;
		D3DState_SetRenderState( D3DRS_CLIPPLANEENABLE, D3DState.EnableState.clipPlaneEnableMask );
		for (int i = 0; i < IMPL_MAX_CLIP_PLANES; ++i) {
			if (D3DState.TransformState.clipPlaneModified[i]) {
				D3DState.TransformState.clipPlaneModified[i] = FALSE;
				hr = D3DGlobal.pDevice->SetClipPlane( i, D3DState.TransformState.clipPlane[i] );
				if (FAILED(hr)) {
					D3DGlobal.lastError = hr;
					return;
				}
			}
		}
	}
}

static void D3DState_SetLight()
{
	if (!D3DState.EnableState.lightingEnabled)
		return;

	for (int i = 0; i < D3DGlobal.maxActiveLights; ++i) {
		if (!D3DState.EnableState.lightEnabled[i])
			continue;
		if (!D3DState.LightingState.lightModified[i])
			continue;
		D3DState.LightingState.lightModified[i] = FALSE;

		//apply light parms
		D3DLIGHT9 dl;
		memset( &dl, 0, sizeof(dl) );
		dl.Range = sqrtf(FLT_MAX);
		dl.Type = D3DState.LightingState.lightType[i];
		dl.Ambient = D3DState.LightingState.lightColorAmbient[i];
		dl.Diffuse = D3DState.LightingState.lightColorDiffuse[i];
		dl.Specular = D3DState.LightingState.lightColorSpecular[i];
		dl.Attenuation0 = D3DState.LightingState.lightAttenuation[i].x;
		dl.Attenuation1 = D3DState.LightingState.lightAttenuation[i].y;
		dl.Attenuation2 = D3DState.LightingState.lightAttenuation[i].z;
		if (D3DState.LightingState.lightType[i] == D3DLIGHT_DIRECTIONAL) {
			dl.Direction = D3DState.LightingState.lightPosition[i];
		} else {
			dl.Position = D3DState.LightingState.lightPosition[i];
		}

	/*	logPrintf("Light %i: type %x\n", i, dl.Type );
		logPrintf("Light %i: position %f %f %f\n", i, dl.Position.x, dl.Position.y, dl.Position.z );
		logPrintf("Light %i: dir %f %f %f\n", i, dl.Direction.x, dl.Direction.y, dl.Direction.z );
		logPrintf("Light %i: ambient %f %f %f %f\n", i, dl.Ambient.r, dl.Ambient.g, dl.Ambient.b, dl.Ambient.a );
		logPrintf("Light %i: diffuse %f %f %f %f\n", i, dl.Diffuse.r, dl.Diffuse.g, dl.Diffuse.b, dl.Diffuse.a );
		logPrintf("Light %i: specular %f %f %f %f\n", i, dl.Specular.r, dl.Specular.g, dl.Specular.b, dl.Specular.a );
		logPrintf("Light %i: atten %f %f %f\n", i, dl.Attenuation0, dl.Attenuation1, dl.Attenuation2 );
		*/
		D3DGlobal.pDevice->SetLight( i, &dl ); 
	}

	if (D3DState.LightingState.currentMaterialModified) {
		D3DState.LightingState.currentMaterialModified = FALSE;
		D3DGlobal.pDevice->SetMaterial(&D3DState.LightingState.currentMaterial);
	}

	if (D3DState.LightingState.colorMaterialModified) {
		D3DState.LightingState.colorMaterialModified = FALSE;
		if (!D3DState.EnableState.colorMaterialEnabled) {
			D3DState_SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
			D3DState_SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
			D3DState_SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
			D3DState_SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
		} else {
			D3DState_SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, (D3DState.LightingState.colorMaterial == GL_AMBIENT || D3DState.LightingState.colorMaterial == GL_AMBIENT_AND_DIFFUSE) ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);
			D3DState_SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, (D3DState.LightingState.colorMaterial == GL_DIFFUSE || D3DState.LightingState.colorMaterial == GL_AMBIENT_AND_DIFFUSE) ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);
			D3DState_SetRenderState( D3DRS_SPECULARMATERIALSOURCE, (D3DState.LightingState.colorMaterial == GL_SPECULAR) ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);
			D3DState_SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, (D3DState.LightingState.colorMaterial == GL_EMISSION) ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);
		}
	}
}

static void D3DState_SetTextureEnvCombine( int stage, int sampler )
{
	const DWORD colorOp = UTIL_GLtoD3DTextureCombineOp( D3DState.TextureState.TextureCombineState[stage].colorOp, D3DState.TextureState.TextureCombineState[stage].colorScale );
	const DWORD alphaOp = UTIL_GLtoD3DTextureCombineOp( D3DState.TextureState.TextureCombineState[stage].alphaOp, D3DState.TextureState.TextureCombineState[stage].alphaScale );

	const DWORD colorArg1 = UTIL_GLtoD3DTextureCombineColorArg( D3DState.TextureState.TextureCombineState[stage].colorArg1, D3DState.TextureState.TextureCombineState[stage].colorOperand1 );
	const DWORD colorArg2 = UTIL_GLtoD3DTextureCombineColorArg( D3DState.TextureState.TextureCombineState[stage].colorArg2, D3DState.TextureState.TextureCombineState[stage].colorOperand2 );
	const DWORD alphaArg1 = UTIL_GLtoD3DTextureCombineAlphaArg( D3DState.TextureState.TextureCombineState[stage].alphaArg1, D3DState.TextureState.TextureCombineState[stage].alphaOperand1 );
	const DWORD alphaArg2 = UTIL_GLtoD3DTextureCombineAlphaArg( D3DState.TextureState.TextureCombineState[stage].alphaArg2, D3DState.TextureState.TextureCombineState[stage].alphaOperand2 );

//	logPrintf("Stage %i, sampler %i: COLOR op %d, arg1 %d, arg2 %d, scale %d\n", stage, sampler, colorOp, colorArg1, colorArg2, D3DState.TextureState.TextureCombineState[stage].colorScale);
//	logPrintf("Stage %i, sampler %i: ALPHA op %d, arg1 %d, arg2 %d, scale %d\n", stage, sampler, alphaOp, alphaArg1, alphaArg2, D3DState.TextureState.TextureCombineState[stage].alphaScale);

	D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_RESULTARG, D3DTA_CURRENT );
	D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLOROP, colorOp );
	D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLORARG1, colorArg1 );
	D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLORARG2, colorArg2 );

	if (colorOp == D3DTOP_LERP) {
		const DWORD colorArg3 = UTIL_GLtoD3DTextureCombineColorArg( D3DState.TextureState.TextureCombineState[stage].colorArg3, D3DState.TextureState.TextureCombineState[stage].colorOperand3 );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLORARG0, colorArg3 );
	}
	
	D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAOP, alphaOp );
	D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAARG1, alphaArg1 );
	D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAARG2, alphaArg2 );

	if (alphaOp == D3DTOP_LERP) {
		const DWORD alphaArg3 = UTIL_GLtoD3DTextureCombineAlphaArg( D3DState.TextureState.TextureCombineState[stage].alphaArg3, D3DState.TextureState.TextureCombineState[stage].alphaOperand3 );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAARG0, alphaArg3 );
	}
}

static void D3DState_SetTextureEnv( int stage, int sampler, eTexTypeInternal intformat )
{
//	logPrintf("Stage %i, sampler %i: MODE 0x%x\n", stage, sampler, D3DState.TextureState.textureEnvMode[stage]);

	switch (D3DState.TextureState.TextureCombineState[stage].envMode) {
	case GL_MODULATE:
//		logPrintf("Stage %i, sampler %i: GL_MODULATE\n", stage, sampler);
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_RESULTARG, D3DTA_CURRENT );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLOROP, D3DTOP_MODULATE );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLORARG2, D3DTA_CURRENT );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
		break;
	case GL_REPLACE:
//		logPrintf("Stage %i, sampler %i: GL_REPLACE\n", stage, sampler);
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_RESULTARG, D3DTA_CURRENT );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		break;
	case GL_DECAL:
//		logPrintf("Stage %i, sampler %i: GL_DECAL\n", stage, sampler);
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_RESULTARG, D3DTA_CURRENT );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLORARG2, D3DTA_CURRENT );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
		break;
	case GL_BLEND:
//		logPrintf("Stage %i, sampler %i: GL_BLEND\n", stage, sampler);
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_RESULTARG, D3DTA_CURRENT );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLOROP, D3DTOP_LERP );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLORARG1, D3DTA_TFACTOR );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLORARG2, D3DTA_CURRENT );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLORARG0, D3DTA_TEXTURE );
		if (intformat == D3D_TEXTYPE_INTENSITY) {
			D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAOP, D3DTOP_LERP );
			D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
			D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
			D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAARG0, D3DTA_TEXTURE );
		} else {
			D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
			D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
		}
		break;
	case GL_ADD:
//		logPrintf("Stage %i, sampler %i: GL_ADD\n", stage, sampler);
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_RESULTARG, D3DTA_CURRENT );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLOROP, D3DTOP_ADD );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_COLORARG2, D3DTA_CURRENT );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		D3DGlobal.pDevice->SetTextureStageState( sampler, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
		break;
	case GL_COMBINE_ARB:
//		logPrintf("Stage %i, sampler %i: GL_COMBINE_ARB\n", stage, sampler);
		D3DState_SetTextureEnvCombine( stage, sampler );
		break;
	default:
		logPrintf("D3DState_SetTextureEnv (stage %i): unsupported mode 0x%x\n", stage, D3DState.TextureState.TextureCombineState[stage].envMode);
		break;
	}

	if (D3DGlobal.hD3DCaps.RasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS)
		D3DGlobal.pDevice->SetSamplerState( sampler, D3DSAMP_MIPMAPLODBIAS, UTIL_FloatToDword(D3DState.TextureState.textureLodBias[stage])  );
}

void D3DState_SetTexture()
{
	if (!D3DState.TextureState.textureSamplerStateChanged)
		return;

	DWORD currentSampler = 0;
	HRESULT hr;
	bool invalidCombiners = false;

	D3DState_BuildTextureReferences();

	for (int i = 0; i < D3DGlobal.maxActiveTMU; ++i) {
		if (!D3DState.EnableState.textureEnabled[i] && 
			!(D3DState.TextureState.textureReference & (1<<i)))
			continue;

		bool matrixChanged = D3DState.TextureState.textureEnableChanged || D3DState.textureMatrixModified[i];
		if (matrixChanged) {
			D3DState.textureMatrixModified[i] = false;
			hr = D3DGlobal.pDevice->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + currentSampler), D3DGlobal.textureMatrixStack[i]->top() );
			if (FAILED(hr)) {
				D3DGlobal.lastError = hr;
				break;
			}
		}

		D3DTextureObject *bestTexture( nullptr );
		DWORD *bestTextureChanged( nullptr );
		int currentTarget = 0;

		DWORD envModeChanged = D3DState.TextureState.textureEnableChanged | D3DState.TextureState.textureEnvModeChanged[i];
		if (envModeChanged) {
			D3DState.TextureState.currentCombinerCount = 0;
			for (int j = D3D_TEXTARGET_MAX-1; j >= 0; j--) {
				if (!D3DState.EnableState.textureTargetEnabled[i][j]) 
					continue;
				currentTarget = j;
				bestTexture = D3DState.TextureState.currentTexture[i][j];
				bestTextureChanged = &D3DState.TextureState.textureChanged[i][j];
				break;
			}

			if ( !D3DState_ValidateCombiner( i, currentSampler ) )
				invalidCombiners = true;

			D3DState_SetTextureEnv( i, currentSampler, bestTexture ? bestTexture->GetInternalFormat() : D3D_TEXTYPE_GENERIC );
			D3DState.TextureState.textureEnvModeChanged[i] = FALSE;
		}

		DWORD textureStateChanged = D3DState.TextureState.textureEnableChanged | D3DState.TextureState.textureStateChanged[i];
		if (!textureStateChanged) {
			++currentSampler;
			continue;
		}

		D3DState.TextureState.textureStateChanged[i] = FALSE;

		if (!bestTexture) {
			for (int j = D3D_TEXTARGET_MAX-1; j >= 0; j--) {
				if (!D3DState.EnableState.textureTargetEnabled[i][j]) 
					continue;
				currentTarget = j;
				bestTexture = D3DState.TextureState.currentTexture[i][j];
				bestTextureChanged = &D3DState.TextureState.textureChanged[i][j];
				break;
			}
		}

		hr = D3DGlobal.pDevice->SetTexture( currentSampler, bestTexture ? bestTexture->GetD3DTexture() : nullptr );
		if (FAILED(hr)) {
			D3DGlobal.lastError = hr;
			break;
		}

		if (bestTexture) {
			D3DState.TransformState.texcoordFix[0] = 0.5f / bestTexture->GetWidth();
			D3DState.TransformState.texcoordFix[1] = 0.5f / bestTexture->GetHeight();
		}

		if (bestTextureChanged) {
			DWORD dwBestTextureChanged = D3DState.TextureState.textureEnableChanged | (*bestTextureChanged);
			if (dwBestTextureChanged) {
				//set texture stage state
				*bestTextureChanged = FALSE;

				//Set border color
				D3DGlobal.pDevice->SetSamplerState( currentSampler, D3DSAMP_BORDERCOLOR, bestTexture->GetD3DBorderColor() );

				//Set address mode
				D3DGlobal.pDevice->SetSamplerState( currentSampler, D3DSAMP_ADDRESSU, bestTexture->GetD3DAddressMode(0) );
				if (currentTarget >= D3D_TEXTARGET_2D) D3DGlobal.pDevice->SetSamplerState( currentSampler, D3DSAMP_ADDRESSV, bestTexture->GetD3DAddressMode(1) );
				if (currentTarget >= D3D_TEXTARGET_3D) D3DGlobal.pDevice->SetSamplerState( currentSampler, D3DSAMP_ADDRESSW, bestTexture->GetD3DAddressMode(2) );

				//Set filtering
				D3DGlobal.pDevice->SetSamplerState( currentSampler, D3DSAMP_MAXANISOTROPY, bestTexture->GetAnisotropy() );
				D3DGlobal.pDevice->SetSamplerState( currentSampler, D3DSAMP_MAGFILTER, bestTexture->GetD3DFilter(0) );
				D3DGlobal.pDevice->SetSamplerState( currentSampler, D3DSAMP_MINFILTER, bestTexture->GetD3DFilter(1) );
				D3DGlobal.pDevice->SetSamplerState( currentSampler, D3DSAMP_MIPFILTER, bestTexture->GetD3DFilter(2) );

				if (D3DGlobal.hD3DCaps.RasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS)
					D3DGlobal.pDevice->SetSamplerState( currentSampler, D3DSAMP_MIPMAPLODBIAS, UTIL_FloatToDword(bestTexture->GetLodBias())  );
			}
		}

		++currentSampler;
	}

	// if we've got "invalid" TNT combiners (e.g. texture as arg2), try to
	// replace it with valid D3D combiners
	if ( invalidCombiners )
		D3DState_SetupCombiners();

	if (D3DState.TextureState.currentSamplerCount != currentSampler) {
		for (DWORD i = currentSampler; i < D3DState.TextureState.currentSamplerCount; ++i) {
			hr = D3DGlobal.pDevice->SetTexture( i, nullptr );
			if (FAILED(hr)) {
				D3DGlobal.lastError = hr;
				break;
			}
			if ( i >= D3DState.TextureState.currentCombinerCount ) {
				D3DGlobal.pDevice->SetTextureStageState( i, D3DTSS_COLOROP, D3DTOP_DISABLE );
				D3DGlobal.pDevice->SetTextureStageState( i, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
			}
		}
		D3DState.TextureState.currentSamplerCount = currentSampler;
	}

	D3DState.TextureState.textureEnableChanged = FALSE;
}

void D3DState_Check()
{
	//check state for modifications and apply them
	//this is actually needed before any draw commands (glBegin, glDrawArrays etc.)
	D3DState_SetTransform();
	D3DState_SetLight();
	D3DState_SetTexture();
}

void D3DState_Apply( GLbitfield mask )
{
	if (mask & GL_COLOR_BUFFER_BIT) {
		D3DGlobal.pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DState.ColorBufferState.alphaTestFunc);
		D3DGlobal.pDevice->SetRenderState(D3DRS_ALPHAREF, D3DState.ColorBufferState.alphaTestReference);
		D3DGlobal.pDevice->SetRenderState(D3DRS_SRCBLEND, D3DState.ColorBufferState.alphaBlendSrcFunc);
		D3DGlobal.pDevice->SetRenderState(D3DRS_DESTBLEND, D3DState.ColorBufferState.alphaBlendDstFunc);
		D3DGlobal.pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DState.ColorBufferState.colorWriteMask);
		D3DGlobal.pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, D3DState.EnableState.alphaTestEnabled);
		D3DGlobal.pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, D3DState.EnableState.alphaBlendEnabled);
		if (D3DGlobal.hD3DCaps.SrcBlendCaps & D3DPBLENDCAPS_BLENDFACTOR) {
			if (D3DState.ColorBufferState.alphaBlendUseColor == 1) {
				D3DState_SetRenderState( D3DRS_BLENDFACTOR, D3DState.ColorBufferState.alphaBlendColorAAAA );
			} else if (D3DState.ColorBufferState.alphaBlendUseColor == 2) {
				D3DState_SetRenderState( D3DRS_BLENDFACTOR, D3DState.ColorBufferState.alphaBlendColorARGB );
			}
		}
		D3DGlobal.pDevice->SetRenderState( D3DRS_BLENDOP, D3DState.ColorBufferState.alphaBlendOp );
	}
	if (mask & GL_DEPTH_BUFFER_BIT) {
		D3DGlobal.pDevice->SetRenderState(D3DRS_ZFUNC, D3DState.DepthBufferState.depthTestFunc);
		D3DGlobal.pDevice->SetRenderState(D3DRS_ZWRITEENABLE, D3DState.DepthBufferState.depthWriteMask);
		D3DGlobal.pDevice->SetRenderState(D3DRS_ZENABLE, D3DState.EnableState.depthTestEnabled);
	}
	if (mask & GL_STENCIL_BUFFER_BIT) {
		D3DGlobal.pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DState.StencilBufferState.stencilTestFunc);
		D3DGlobal.pDevice->SetRenderState(D3DRS_STENCILREF, D3DState.StencilBufferState.stencilTestReference);
		D3DGlobal.pDevice->SetRenderState(D3DRS_STENCILMASK, D3DState.StencilBufferState.stencilTestMask);
		D3DGlobal.pDevice->SetRenderState(D3DRS_STENCILWRITEMASK, D3DState.StencilBufferState.stencilWriteMask);
		D3DGlobal.pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DState.StencilBufferState.stencilTestFailFunc);
		D3DGlobal.pDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DState.StencilBufferState.stencilTestZFailFunc);
		D3DGlobal.pDevice->SetRenderState(D3DRS_STENCILPASS, D3DState.StencilBufferState.stencilTestPassFunc);
		D3DGlobal.pDevice->SetRenderState(D3DRS_STENCILENABLE, D3DState.EnableState.stencilTestEnabled);
		if (D3DGlobal.hD3DCaps.StencilCaps & D3DSTENCILCAPS_TWOSIDED) {
			D3DGlobal.pDevice->SetRenderState(D3DRS_CCW_STENCILFUNC, D3DState.StencilBufferState.stencilTestFuncCCW);
			D3DGlobal.pDevice->SetRenderState(D3DRS_CCW_STENCILFAIL, D3DState.StencilBufferState.stencilTestFailFuncCCW);
			D3DGlobal.pDevice->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DState.StencilBufferState.stencilTestZFailFuncCCW);
			D3DGlobal.pDevice->SetRenderState(D3DRS_CCW_STENCILPASS, D3DState.StencilBufferState.stencilTestPassFuncCCW);
			D3DGlobal.pDevice->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, D3DState.EnableState.twoSideStencilEnabled );
		}
	}
	if (mask & GL_FOG_BIT) {
		D3DGlobal.pDevice->SetRenderState(D3DRS_FOGENABLE, D3DState.EnableState.fogEnabled);
		D3DGlobal.pDevice->SetRenderState(D3DRS_FOGCOLOR, D3DState.FogState.fogColor);
		D3DGlobal.pDevice->SetRenderState(D3DRS_FOGSTART, UTIL_FloatToDword(D3DState.FogState.fogStart));
		D3DGlobal.pDevice->SetRenderState(D3DRS_FOGEND, UTIL_FloatToDword(D3DState.FogState.fogEnd));
		D3DGlobal.pDevice->SetRenderState(D3DRS_FOGDENSITY, UTIL_FloatToDword(D3DState.FogState.fogDensity));
		D3DGlobal.pDevice->SetRenderState(D3DRS_FOGTABLEMODE, (!D3DState.FogState.fogCoordMode && D3DState.HintState.fogHint <= 1) ? D3DState.FogState.fogMode : D3DFOG_NONE);
		D3DGlobal.pDevice->SetRenderState(D3DRS_FOGVERTEXMODE, (D3DState.FogState.fogCoordMode || D3DState.HintState.fogHint <= 1) ? D3DFOG_NONE : D3DState.FogState.fogMode);
	}
	if (mask & GL_HINT_BIT) {
		if (!(mask & GL_FOG_BIT)) {
			D3DGlobal.pDevice->SetRenderState(D3DRS_FOGTABLEMODE, (!D3DState.FogState.fogCoordMode && D3DState.HintState.fogHint <= 1) ? D3DState.FogState.fogMode : D3DFOG_NONE);
			D3DGlobal.pDevice->SetRenderState(D3DRS_FOGVERTEXMODE, (D3DState.FogState.fogCoordMode || D3DState.HintState.fogHint <= 1) ? D3DFOG_NONE : D3DState.FogState.fogMode);
		}
	}
	if (mask & GL_LIGHTING_BIT) {
		D3DGlobal.pDevice->SetRenderState(D3DRS_SHADEMODE, D3DState.LightingState.shadeMode);
		D3DGlobal.pDevice->SetRenderState(D3DRS_LIGHTING, D3DState.EnableState.lightingEnabled);
		D3DGlobal.pDevice->SetRenderState(D3DRS_LOCALVIEWER, D3DState.LightingState.lightModelLocalViewer);
		D3DGlobal.pDevice->SetRenderState(D3DRS_AMBIENT, D3DState.LightingState.lightModelAmbient);
		for (int i = 0; i < D3DGlobal.maxActiveLights; ++i) {
			D3DGlobal.pDevice->LightEnable(i, D3DState.EnableState.lightEnabled[i] );
			D3DState.LightingState.lightModified[i] = TRUE;
		}
	}
	if (mask & GL_POINT_BIT) {
		D3DGlobal.pDevice->SetRenderState(D3DRS_POINTSIZE, UTIL_FloatToDword(D3DState.PointState.pointSize));
	}
	if (mask & GL_POLYGON_BIT) {
		D3DGlobal.pDevice->SetRenderState(D3DRS_FILLMODE, D3DState.PolygonState.fillMode);
		D3DState_SetCullMode();
		D3DState_SetDepthBias();
	}
	if (mask & GL_ENABLE_BIT) {
		D3DGlobal.pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, D3DState.EnableState.alphaTestEnabled);
		D3DGlobal.pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, D3DState.EnableState.alphaBlendEnabled);
		D3DGlobal.pDevice->SetRenderState(D3DRS_ZENABLE, D3DState.EnableState.depthTestEnabled);
		D3DGlobal.pDevice->SetRenderState(D3DRS_STENCILENABLE, D3DState.EnableState.stencilTestEnabled);
		D3DGlobal.pDevice->SetRenderState(D3DRS_FOGENABLE, D3DState.EnableState.fogEnabled);
		D3DGlobal.pDevice->SetRenderState(D3DRS_LIGHTING, D3DState.EnableState.lightingEnabled);
		D3DGlobal.pDevice->SetRenderState(D3DRS_DITHERENABLE, D3DState.EnableState.ditherEnabled);
		D3DGlobal.pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, D3DState.EnableState.normalizeEnabled);
		if (!(mask & GL_POLYGON_BIT)) {
			D3DState_SetCullMode();
			D3DState_SetDepthBias();
		}
		if (!(mask & GL_TEXTURE_BIT)) {
			D3DState_SetTexture();
		}
		if (D3DGlobal.hD3DCaps.StencilCaps & D3DSTENCILCAPS_TWOSIDED) {
			D3DGlobal.pDevice->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, D3DState.EnableState.twoSideStencilEnabled );
		}
	}
	if (mask & GL_TEXTURE_BIT) {
		D3DState_SetTexture();
		D3DGlobal.pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, D3DState.TextureState.textureEnvColor );
	}
	if (mask & GL_TRANSFORM_BIT) {
		D3DGlobal.pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, D3DState.EnableState.normalizeEnabled);
	}
	if (mask & GL_VIEWPORT_BIT) {
		D3DGlobal.pDevice->SetViewport(&D3DState.viewport);
	}
}

void D3DState_SetDefaults()
{
	assert( D3DGlobal.pD3D != nullptr );
	assert( D3DGlobal.pDevice != nullptr );

	D3DStateCopyMask = 0;
	D3DStateClientCopyMask = 0;
	memset( &D3DStateCopy, 0, sizeof(D3DStateCopy) );
	memset( &D3DState, 0, sizeof(D3DState) );

	D3DState.ColorBufferState.clearColor = D3DCOLOR_ARGB(0,0,0,0);
	D3DState.DepthBufferState.clearDepth = 1.0f;
	D3DState.EnableState.depthBiasEnabled = false;
	D3DState.EnableState.cullEnabled = false;
	D3DState.EnableState.twoSideStencilEnabled = false;
	D3DState.PointState.pointSize = 1.0f;
	D3DState.PolygonState.cullMode = GL_BACK;
	D3DState.PolygonState.frontFace = GL_CCW;
	D3DState.StencilBufferState.activeStencilFace = GL_CCW;
	D3DState.TransformState.matrixMode = GL_MODELVIEW;
	D3DState.CurrentState.currentColor = D3DCOLOR_XRGB( 255, 255, 255 );
	D3DState.CurrentState.currentColor2 = 0;
	D3DState.CurrentState.currentNormal[0] = 0.0f;
	D3DState.CurrentState.currentNormal[1] = 0.0f;
	D3DState.CurrentState.currentNormal[2] = 1.0f;
	D3DState.TextureState.textureSamplerStateChanged = TRUE;
	for (int i = 0; i < MAX_D3D_TMU; ++i) {
		D3DState.CurrentState.currentTexCoord[i][0] = 0.0f;
		D3DState.CurrentState.currentTexCoord[i][1] = 0.0f;
		D3DState.CurrentState.currentTexCoord[i][2] = 0.0f;
		D3DState.CurrentState.currentTexCoord[i][3] = 1.0f;
		D3DState.TextureState.textureEnvModeChanged[i] = TRUE;
		D3DState.TextureState.TextureCombineState[i].envMode = GL_MODULATE;
		D3DState.TextureState.TextureCombineState[i].colorOp = GL_MODULATE;
		D3DState.TextureState.TextureCombineState[i].alphaOp = GL_MODULATE;
		D3DState.TextureState.TextureCombineState[i].colorScale = 1;
		D3DState.TextureState.TextureCombineState[i].alphaScale = 1;
		D3DState.TextureState.TextureCombineState[i].colorArg1 = GL_TEXTURE;
		D3DState.TextureState.TextureCombineState[i].colorArg2 = GL_PREVIOUS_ARB;
		D3DState.TextureState.TextureCombineState[i].colorArg3 = GL_CONSTANT_ARB;
		D3DState.TextureState.TextureCombineState[i].colorOperand1 = GL_SRC_COLOR;
		D3DState.TextureState.TextureCombineState[i].colorOperand2 = GL_SRC_COLOR;
		D3DState.TextureState.TextureCombineState[i].colorOperand3 = GL_SRC_ALPHA;
		D3DState.TextureState.TextureCombineState[i].alphaArg1 = GL_TEXTURE;
		D3DState.TextureState.TextureCombineState[i].alphaArg2 = GL_PREVIOUS_ARB;
		D3DState.TextureState.TextureCombineState[i].alphaArg3 = GL_CONSTANT_ARB;
		D3DState.TextureState.TextureCombineState[i].alphaOperand1 = GL_SRC_ALPHA;
		D3DState.TextureState.TextureCombineState[i].alphaOperand2 = GL_SRC_ALPHA;
		D3DState.TextureState.TextureCombineState[i].alphaOperand3 = GL_SRC_ALPHA;
		for (int j = 0; j < 4; ++j) {
			D3DState.TextureState.TexGen[i][j].mode = GL_EYE_LINEAR;
			D3DState.TextureState.TexGen[i][j].objectPlane[0] = (j == 0) ? 1.0f : 0.0f;
			D3DState.TextureState.TexGen[i][j].objectPlane[1] = (j == 1) ? 1.0f : 0.0f;
			D3DState.TextureState.TexGen[i][j].objectPlane[2] = 0.0f;
			D3DState.TextureState.TexGen[i][j].objectPlane[3] = 0.0f;
			D3DState.TextureState.TexGen[i][j].eyePlane[0] = (j == 0) ? 1.0f : 0.0f;
			D3DState.TextureState.TexGen[i][j].eyePlane[1] = (j == 1) ? 1.0f : 0.0f;
			D3DState.TextureState.TexGen[i][j].eyePlane[2] = 0.0f;
			D3DState.TextureState.TexGen[i][j].eyePlane[3] = 0.0f;
			SelectTexGenFunc( i, j );
		}
	}
	D3DState.FogState.fogMode = D3DFOG_EXP;
	D3DState.FogState.fogStart = 0.0f;
	D3DState.FogState.fogEnd = 1.0f;
	D3DState.FogState.fogDensity = 1.0f;
	D3DState.HintState.fogHint = 0;
	for (int i = 0; i < IMPL_MAX_LIGHTS; ++i) {
		D3DState.LightingState.lightModified[i] = TRUE;
		D3DState.LightingState.lightType[i] = D3DLIGHT_DIRECTIONAL;
		D3DState.LightingState.lightColorAmbient[i].r = 0.0f;
		D3DState.LightingState.lightColorAmbient[i].g = 0.0f;
		D3DState.LightingState.lightColorAmbient[i].b = 0.0f;
		D3DState.LightingState.lightColorAmbient[i].a = 1.0f;
		D3DState.LightingState.lightColorDiffuse[i].r = (i==0)?1.0f:0.0f;
		D3DState.LightingState.lightColorDiffuse[i].g = (i==0)?1.0f:0.0f;
		D3DState.LightingState.lightColorDiffuse[i].b = (i==0)?1.0f:0.0f;
		D3DState.LightingState.lightColorDiffuse[i].a = 1.0f;
		D3DState.LightingState.lightColorSpecular[i].r = (i==0)?1.0f:0.0f;
		D3DState.LightingState.lightColorSpecular[i].g = (i==0)?1.0f:0.0f;
		D3DState.LightingState.lightColorSpecular[i].b = (i==0)?1.0f:0.0f;
		D3DState.LightingState.lightColorSpecular[i].a = 1.0f;
		D3DState.LightingState.lightPosition[i].x = 0.0f;
		D3DState.LightingState.lightPosition[i].y = 0.0f;
		D3DState.LightingState.lightPosition[i].z = 1.0f;
		D3DState.LightingState.lightAttenuation[i].x = 1.0f;
		D3DState.LightingState.lightAttenuation[i].y = 0.0f;
		D3DState.LightingState.lightAttenuation[i].z = 0.0f;
	}

	D3DState.LightingState.currentMaterial.Diffuse.r = 0.8f;
	D3DState.LightingState.currentMaterial.Diffuse.g = 0.8f;
	D3DState.LightingState.currentMaterial.Diffuse.b = 0.8f;
	D3DState.LightingState.currentMaterial.Diffuse.a = 1.0f;
	D3DState.LightingState.currentMaterial.Ambient.r = 0.2f;
	D3DState.LightingState.currentMaterial.Ambient.g = 0.2f;
	D3DState.LightingState.currentMaterial.Ambient.b = 0.2f;
	D3DState.LightingState.currentMaterial.Ambient.a = 1.0f;
	D3DState.LightingState.currentMaterial.Specular.r = 0.0f;
	D3DState.LightingState.currentMaterial.Specular.g = 0.0f;
	D3DState.LightingState.currentMaterial.Specular.b = 0.0f;
	D3DState.LightingState.currentMaterial.Specular.a = 1.0f;
	D3DState.LightingState.currentMaterial.Power = 0.0f;
	D3DState.LightingState.currentMaterial.Emissive.r = 0.0f;
	D3DState.LightingState.currentMaterial.Emissive.g = 0.0f;
	D3DState.LightingState.currentMaterial.Emissive.b = 0.0f;
	D3DState.LightingState.currentMaterial.Emissive.a = 1.0f;
	D3DState.LightingState.currentMaterialModified = TRUE;
	D3DState.LightingState.colorMaterial = GL_AMBIENT_AND_DIFFUSE;
	D3DState.LightingState.colorMaterialModified = TRUE;
	D3DState.LightingState.lightModelLocalViewer = TRUE;	//crap, OpenGL's non-local viewer is inverted in Z direction
	D3DState.LightingState.lightModelAmbient = D3DCOLOR_ARGB(255, 51, 51, 51);

	D3DState.currentMatrixStack = D3DGlobal.modelviewMatrixStack;
	D3DState.currentMatrixModified = &D3DState.modelViewMatrixModified;

	D3DState.ColorBufferState.glBlendDst = GL_ONE;
	D3DState.ColorBufferState.glBlendSrc = GL_ONE;
	D3DState.ColorBufferState.alphaBlendDstFunc = D3DBLEND_ZERO;
	D3DState.ColorBufferState.alphaBlendSrcFunc = D3DBLEND_ONE;
	D3DState.ColorBufferState.alphaBlendOp = D3DBLENDOP_ADD;
	D3DState.ColorBufferState.alphaTestFunc = D3DCMP_ALWAYS;
	D3DState.ColorBufferState.alphaTestReference = 0;
	D3DState.ColorBufferState.colorWriteMask = ~0u;
	D3DState.DepthBufferState.depthTestFunc = D3DCMP_LESS;
	D3DState.DepthBufferState.depthWriteMask = TRUE;
	D3DState.StencilBufferState.stencilTestFailFunc = D3DSTENCILOP_KEEP;
	D3DState.StencilBufferState.stencilTestPassFunc = D3DSTENCILOP_KEEP;
	D3DState.StencilBufferState.stencilTestZFailFunc = D3DSTENCILOP_KEEP;
	D3DState.StencilBufferState.stencilTestFunc = D3DCMP_ALWAYS;
	D3DState.StencilBufferState.stencilTestFailFuncCCW = D3DSTENCILOP_KEEP;
	D3DState.StencilBufferState.stencilTestPassFuncCCW = D3DSTENCILOP_KEEP;
	D3DState.StencilBufferState.stencilTestZFailFuncCCW = D3DSTENCILOP_KEEP;
	D3DState.StencilBufferState.stencilTestFuncCCW = D3DCMP_ALWAYS;
	D3DState.StencilBufferState.stencilTestMask = ~0u;
	D3DState.StencilBufferState.stencilWriteMask = ~0u;
	D3DState.PolygonState.fillMode = D3DFILL_SOLID;
	D3DState.LightingState.shadeMode = D3DSHADE_GOURAUD;

	D3DState.ClientPixelStoreState.unpackSwapBytes = 0;
	D3DState.ClientPixelStoreState.unpackLSBfirst = 0;
	D3DState.ClientPixelStoreState.unpackRowLength = 0;
	D3DState.ClientPixelStoreState.unpackImageHeight = 0;
	D3DState.ClientPixelStoreState.unpackSkipPixels = 0;
	D3DState.ClientPixelStoreState.unpackSkipRows = 0;
	D3DState.ClientPixelStoreState.unpackSkipImages = 0;
	D3DState.ClientPixelStoreState.unpackAlignment = 4;
	D3DState.ClientPixelStoreState.packSwapBytes = 0;
	D3DState.ClientPixelStoreState.packLSBfirst = 0;
	D3DState.ClientPixelStoreState.packRowLength = 0;
	D3DState.ClientPixelStoreState.packImageHeight = 0;
	D3DState.ClientPixelStoreState.packSkipPixels = 0;
	D3DState.ClientPixelStoreState.packSkipRows = 0;
	D3DState.ClientPixelStoreState.packSkipImages = 0;
	D3DState.ClientPixelStoreState.packAlignment = 4;
	D3DState.ClientPixelStoreState.transferMapColor = 0;
	D3DState.ClientPixelStoreState.transferRedScale = 1.0f;
	D3DState.ClientPixelStoreState.transferRedBias = 0.0f;
	D3DState.ClientPixelStoreState.transferGreenScale = 1.0f;
	D3DState.ClientPixelStoreState.transferGreenBias = 0.0f;
	D3DState.ClientPixelStoreState.transferBlueScale = 1.0f;
	D3DState.ClientPixelStoreState.transferBlueBias = 0.0f;
	D3DState.ClientPixelStoreState.transferAlphaScale = 1.0f;
	D3DState.ClientPixelStoreState.transferAlphaBias = 0.0f;
	D3DState.ClientPixelStoreState.pixelmapSizeRtoR = 1;
	D3DState.ClientPixelStoreState.pixelmapSizeGtoG = 1;
	D3DState.ClientPixelStoreState.pixelmapSizeBtoB = 1;
	D3DState.ClientPixelStoreState.pixelmapSizeAtoA = 1;

	D3DState.ClientVertexArrayState.vertexInfo._internal.compiledLast = -1;
	D3DState.ClientVertexArrayState.normalInfo._internal.compiledLast = -1;
	D3DState.ClientVertexArrayState.colorInfo._internal.compiledLast = -1;
	D3DState.ClientVertexArrayState.color2Info._internal.compiledLast = -1;
	D3DState.ClientVertexArrayState.fogInfo._internal.compiledLast = -1;
	for (int i = 0; i < MAX_D3D_TMU; ++i)
		D3DState.ClientVertexArrayState.texCoordInfo[i]._internal.compiledLast = -1;

	for (int j = 0; j < D3D_TEXTARGET_MAX; ++j) {
		for (int i = 0; i < MAX_D3D_TMU; ++i) {
			D3DState.TextureState.currentTexture[i][j] = D3DGlobal.defaultTexture[j];
		}
	}

	D3DGlobal.pDevice->GetViewport( &D3DState.viewport );

	D3DState_Apply( GL_ALL_ATTRIB_BITS );
	
	D3DGlobal.pDevice->SetRenderState( D3DRS_COLORVERTEX, TRUE );
	D3DGlobal.pDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
	D3DGlobal.pDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
	D3DGlobal.pDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );
	D3DGlobal.pDevice->SetRenderState( D3DRS_MULTISAMPLEMASK, 0x00FFFFFF );

	for (int i = 0; i < D3DGlobal.maxActiveTMU; ++i) {
		D3DGlobal.pDevice->SetSamplerState( i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
		D3DGlobal.pDevice->SetSamplerState( i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
		D3DGlobal.pDevice->SetSamplerState( i, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP );
		D3DGlobal.pDevice->SetSamplerState( i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		D3DGlobal.pDevice->SetSamplerState( i, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		D3DGlobal.pDevice->SetSamplerState( i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

		//always transform texture coordinates
		//!FIXME: maybe we should track texture matrix, and disable it if it is identity?
		D3DGlobal.pDevice->SetTextureStageState( i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT4 | D3DTTFF_PROJECTED );
	}

	D3DXMATRIX d3dIdentityMatrix;
	D3DXMatrixIdentity(&d3dIdentityMatrix);
	D3DGlobal.pDevice->SetTransform( D3DTS_WORLD, &d3dIdentityMatrix );
	D3DGlobal.pDevice->SetTransform( D3DTS_VIEW, &d3dIdentityMatrix );
	D3DGlobal.pDevice->SetTransform( D3DTS_PROJECTION, &d3dIdentityMatrix );
	D3DGlobal.pDevice->SetTransform( D3DTS_TEXTURE0, &d3dIdentityMatrix );
}

OPENGL_API void WINAPI glPushAttrib( GLbitfield mask )
{
	D3DStateCopyMask = mask;
	D3DState_Copy( &D3DState, &D3DStateCopy, mask );
}

OPENGL_API void WINAPI glPushClientAttrib( GLbitfield mask )
{
	D3DStateClientCopyMask = mask;
	D3DState_CopyClient( &D3DState, &D3DStateCopy, mask );
}

OPENGL_API void WINAPI glPopAttrib()
{
	if (!D3DStateCopyMask) {
		D3DGlobal.lastError = E_STACK_UNDERFLOW;
		return;
	}

	D3DState_Copy( &D3DStateCopy, &D3DState, D3DStateCopyMask );

	D3DState.modelViewMatrixModified = TRUE;
	D3DState.projectionMatrixModified = TRUE;
	D3DState.TransformState.clippingModified = TRUE;
	D3DState.TextureState.textureSamplerStateChanged = TRUE;
	D3DState.LightingState.currentMaterialModified = TRUE;
	D3DState.LightingState.colorMaterialModified = TRUE;
	D3DState.TextureState.textureEnableChanged = TRUE;
	for (int i = 0; i < D3DGlobal.maxActiveTMU; ++i) {
		D3DState.textureMatrixModified[i] = TRUE;
		D3DState.TextureState.textureEnvModeChanged[i] = TRUE;
	}

	D3DState_Apply( D3DStateCopyMask );

	D3DStateCopyMask = 0;
}

OPENGL_API void WINAPI glPopClientAttrib()
{
	if (!D3DStateClientCopyMask) {
		D3DGlobal.lastError = E_STACK_UNDERFLOW;
		return;
	}

	D3DState_CopyClient( &D3DStateCopy, &D3DState, D3DStateClientCopyMask );
	D3DStateClientCopyMask = 0;
}

static DWORD D3DState_IsEnabledState( GLenum cap )
{
	switch (cap) {
	case GL_ALPHA_TEST:
		return D3DState.EnableState.alphaTestEnabled;
	case GL_BLEND:
		return D3DState.EnableState.alphaBlendEnabled;
	case GL_CULL_FACE:
		return D3DState.EnableState.cullEnabled;
	case GL_DEPTH_TEST:
		return D3DState.EnableState.depthTestEnabled;
	case GL_DITHER:
		return D3DState.EnableState.ditherEnabled;
	case GL_FOG:
		return D3DState.EnableState.fogEnabled;
	case GL_LIGHTING:
		return D3DState.EnableState.lightingEnabled;
	case GL_COLOR_SUM_EXT:
		return D3DState.EnableState.colorSumEnabled;
	case GL_COLOR_MATERIAL:
		return D3DState.EnableState.colorMaterialEnabled;
	case GL_NORMALIZE:
	case GL_RESCALE_NORMAL_EXT:
		return D3DState.EnableState.normalizeEnabled;
	case GL_POLYGON_OFFSET_FILL:
		return D3DState.EnableState.depthBiasEnabled;
	case GL_SCISSOR_TEST:
		return D3DState.EnableState.scissorEnabled;
	case GL_STENCIL_TEST:
		return D3DState.EnableState.stencilTestEnabled;
	case GL_STENCIL_TEST_TWO_SIDE_EXT:
		return D3DState.EnableState.twoSideStencilEnabled;
	case GL_TEXTURE_1D:
		return D3DState.EnableState.textureTargetEnabled[D3DState.TextureState.currentTMU][D3D_TEXTARGET_1D];
	case GL_TEXTURE_2D:
		return D3DState.EnableState.textureTargetEnabled[D3DState.TextureState.currentTMU][D3D_TEXTARGET_2D];
	case GL_TEXTURE_3D_EXT:
		return D3DState.EnableState.textureTargetEnabled[D3DState.TextureState.currentTMU][D3D_TEXTARGET_3D];
	case GL_TEXTURE_CUBE_MAP_ARB:
		return D3DState.EnableState.textureTargetEnabled[D3DState.TextureState.currentTMU][D3D_TEXTARGET_CUBE];
	case GL_TEXTURE_GEN_S:
		return (D3DState.EnableState.texGenEnabled[D3DState.TextureState.currentTMU] & (1 << 0) );
	case GL_TEXTURE_GEN_T:
		return (D3DState.EnableState.texGenEnabled[D3DState.TextureState.currentTMU] & (1 << 1) );
	case GL_TEXTURE_GEN_R:
		return (D3DState.EnableState.texGenEnabled[D3DState.TextureState.currentTMU] & (1 << 2) );
	case GL_TEXTURE_GEN_Q:
		return (D3DState.EnableState.texGenEnabled[D3DState.TextureState.currentTMU] & (1 << 3) );
	case GL_CLIP_PLANE0:
	case GL_CLIP_PLANE1:
	case GL_CLIP_PLANE2:
	case GL_CLIP_PLANE3:
	case GL_CLIP_PLANE4:
	case GL_CLIP_PLANE5:
		return (D3DState.EnableState.clipPlaneEnableMask & (1 << (cap - GL_CLIP_PLANE0)));

	case GL_LIGHT0:
	case GL_LIGHT1:
	case GL_LIGHT2:
	case GL_LIGHT3:
	case GL_LIGHT4:
	case GL_LIGHT5:
	case GL_LIGHT6:
	case GL_LIGHT7:
		{
			int lightIndex = cap - GL_LIGHT0;
			if (lightIndex >= 0 && lightIndex < D3DGlobal.maxActiveLights)
				return D3DState.EnableState.lightEnabled[lightIndex];
			else
				return GL_FALSE;
		}

	case GL_AUTO_NORMAL:
	case GL_POLYGON_OFFSET_LINE:
	case GL_POLYGON_OFFSET_POINT:
		//silently ignore these caps
		return 0;

	case GL_MAP1_COLOR_4:
	case GL_MAP1_INDEX:
	case GL_MAP1_NORMAL:
	case GL_MAP1_TEXTURE_COORD_1:
	case GL_MAP1_TEXTURE_COORD_2:
	case GL_MAP1_TEXTURE_COORD_3:
	case GL_MAP1_TEXTURE_COORD_4:
	case GL_MAP1_VERTEX_3:
	case GL_MAP1_VERTEX_4:
	case GL_MAP2_COLOR_4:
	case GL_MAP2_INDEX:
	case GL_MAP2_NORMAL:
	case GL_MAP2_TEXTURE_COORD_1:
	case GL_MAP2_TEXTURE_COORD_2:
	case GL_MAP2_TEXTURE_COORD_3:
	case GL_MAP2_TEXTURE_COORD_4:
	case GL_MAP2_VERTEX_3:
	case GL_MAP2_VERTEX_4:
		return 0;

	case GL_POLYGON_SMOOTH:
	case GL_POLYGON_STIPPLE:
	case GL_POINT_SMOOTH:
		return 0;

	case GL_INDEX_LOGIC_OP:
	case GL_COLOR_LOGIC_OP:
		return 0;

	default:
		logPrintf("WARNING: glIsEnabled( 0x%x ) unimplemented\n", cap);
		D3DGlobal.lastError = E_INVALID_ENUM;
		return 0;
	}
}

static void D3DState_EnableDisableState( GLenum cap, DWORD value )
{
	switch (cap) {
	case GL_ALPHA_TEST:
		D3DState.EnableState.alphaTestEnabled = value;
		D3DState_SetRenderState( D3DRS_ALPHATESTENABLE, D3DState.EnableState.alphaTestEnabled );
		break;
	case GL_BLEND:
		D3DState.EnableState.alphaBlendEnabled = value;
		D3DState_SetRenderState( D3DRS_ALPHABLENDENABLE, D3DState.EnableState.alphaBlendEnabled );
		break;
	case GL_CULL_FACE:
		D3DState.EnableState.cullEnabled = value;
		D3DState_SetCullMode();
		break;
	case GL_DEPTH_TEST:
		D3DState.EnableState.depthTestEnabled = value;
		D3DState_SetRenderState( D3DRS_ZENABLE, D3DState.EnableState.depthTestEnabled );
		break;
	case GL_DITHER:
		D3DState.EnableState.ditherEnabled = value;
		D3DState_SetRenderState( D3DRS_DITHERENABLE, D3DState.EnableState.ditherEnabled );
		break;
	case GL_FOG:
		D3DState.EnableState.fogEnabled = value;
		D3DState_SetRenderState( D3DRS_FOGENABLE, D3DState.EnableState.fogEnabled );
		break;
	case GL_LIGHTING:
		D3DState.EnableState.lightingEnabled = value;
		D3DState_SetRenderState( D3DRS_LIGHTING, D3DState.EnableState.lightingEnabled );
		break;
	case GL_COLOR_MATERIAL:
		if (D3DState.EnableState.colorMaterialEnabled != value) {
			D3DState.EnableState.colorMaterialEnabled = value;
			D3DState.LightingState.colorMaterialModified = TRUE;
		}
		break;
	case GL_COLOR_SUM_EXT:
		D3DState.EnableState.colorSumEnabled = value;
		break;
	case GL_NORMALIZE:
	case GL_RESCALE_NORMAL_EXT:
		D3DState.EnableState.normalizeEnabled = value;
		D3DState_SetRenderState( D3DRS_NORMALIZENORMALS, D3DState.EnableState.lightingEnabled );
		break;
	case GL_POLYGON_OFFSET_FILL:
		D3DState.EnableState.depthBiasEnabled = value;
		D3DState_SetDepthBias();
		break;
	case GL_SCISSOR_TEST:
		D3DState.EnableState.scissorEnabled = value;
		D3DState_SetRenderState( D3DRS_SCISSORTESTENABLE, D3DState.EnableState.scissorEnabled );
		break;
	case GL_STENCIL_TEST:
		D3DState.EnableState.stencilTestEnabled = value;
		D3DState_SetRenderState( D3DRS_STENCILENABLE, D3DState.EnableState.stencilTestEnabled );
		break;
	case GL_TEXTURE_1D:
		if (D3DState.EnableState.textureTargetEnabled[D3DState.TextureState.currentTMU][D3D_TEXTARGET_1D] != value) {
			D3DState.EnableState.textureTargetEnabled[D3DState.TextureState.currentTMU][D3D_TEXTARGET_1D] = value;
			D3DState.TextureState.textureSamplerStateChanged = TRUE;
			D3DState.TextureState.textureEnableChanged = TRUE;
			D3DState.EnableState.textureEnabled[D3DState.TextureState.currentTMU] = value;
			if (!value) {
				for (int i = 0; i < D3D_TEXTARGET_MAX; ++i) {
					if (D3DState.EnableState.textureTargetEnabled[D3DState.TextureState.currentTMU][i]) {
						D3DState.EnableState.textureEnabled[D3DState.TextureState.currentTMU] = TRUE;
						break;
					}
				}
			}
		}
		break;
	case GL_TEXTURE_2D:
		if (D3DState.EnableState.textureTargetEnabled[D3DState.TextureState.currentTMU][D3D_TEXTARGET_2D] != value) {
			D3DState.EnableState.textureTargetEnabled[D3DState.TextureState.currentTMU][D3D_TEXTARGET_2D] = value;
			D3DState.TextureState.textureSamplerStateChanged = TRUE;
			D3DState.TextureState.textureEnableChanged = TRUE;
			D3DState.EnableState.textureEnabled[D3DState.TextureState.currentTMU] = value;
			if (!value) {
				for (int i = 0; i < D3D_TEXTARGET_MAX; ++i) {
					if (D3DState.EnableState.textureTargetEnabled[D3DState.TextureState.currentTMU][i]) {
						D3DState.EnableState.textureEnabled[D3DState.TextureState.currentTMU] = TRUE;
						break;
					}
				}
			}
		}
		break;
	case GL_TEXTURE_3D_EXT:
		if (D3DState.EnableState.textureTargetEnabled[D3DState.TextureState.currentTMU][D3D_TEXTARGET_3D] != value) {
			D3DState.EnableState.textureTargetEnabled[D3DState.TextureState.currentTMU][D3D_TEXTARGET_3D] = value;
			D3DState.TextureState.textureSamplerStateChanged = TRUE;
			D3DState.TextureState.textureEnableChanged = TRUE;
			D3DState.EnableState.textureEnabled[D3DState.TextureState.currentTMU] = value;
			if (!value) {
				for (int i = 0; i < D3D_TEXTARGET_MAX; ++i) {
					if (D3DState.EnableState.textureTargetEnabled[D3DState.TextureState.currentTMU][i]) {
						D3DState.EnableState.textureEnabled[D3DState.TextureState.currentTMU] = TRUE;
						break;
					}
				}
			}
		}
		break;
	case GL_TEXTURE_CUBE_MAP_ARB:
		if (D3DState.EnableState.textureTargetEnabled[D3DState.TextureState.currentTMU][D3D_TEXTARGET_CUBE] != value) {
			D3DState.EnableState.textureTargetEnabled[D3DState.TextureState.currentTMU][D3D_TEXTARGET_CUBE] = value;
			D3DState.TextureState.textureSamplerStateChanged = TRUE;
			D3DState.TextureState.textureEnableChanged = TRUE;
			D3DState.EnableState.textureEnabled[D3DState.TextureState.currentTMU] = value;
			if (!value) {
				for (int i = 0; i < D3D_TEXTARGET_MAX; ++i) {
					if (D3DState.EnableState.textureTargetEnabled[D3DState.TextureState.currentTMU][i]) {
						D3DState.EnableState.textureEnabled[D3DState.TextureState.currentTMU] = TRUE;
						break;
					}
				}
			}
		}
		break;
	case GL_TEXTURE_GEN_S:
		if (value) D3DState.EnableState.texGenEnabled[D3DState.TextureState.currentTMU] |= (1 << 0);
		else D3DState.EnableState.texGenEnabled[D3DState.TextureState.currentTMU] &= ~(1 << 0);
		break;
	case GL_TEXTURE_GEN_T:
		if (value) D3DState.EnableState.texGenEnabled[D3DState.TextureState.currentTMU] |= (1 << 1);
		else D3DState.EnableState.texGenEnabled[D3DState.TextureState.currentTMU] &= ~(1 << 1);
		break;
	case GL_TEXTURE_GEN_R:
		if (value) D3DState.EnableState.texGenEnabled[D3DState.TextureState.currentTMU] |= (1 << 2);
		else D3DState.EnableState.texGenEnabled[D3DState.TextureState.currentTMU] &= ~(1 << 2);
		break;
	case GL_TEXTURE_GEN_Q:
		if (value) D3DState.EnableState.texGenEnabled[D3DState.TextureState.currentTMU] |= (1 << 3);
		else D3DState.EnableState.texGenEnabled[D3DState.TextureState.currentTMU] &= ~(1 << 3);
		break;
	case GL_CLIP_PLANE0:
	case GL_CLIP_PLANE1:
	case GL_CLIP_PLANE2:
	case GL_CLIP_PLANE3:
	case GL_CLIP_PLANE4:
	case GL_CLIP_PLANE5:
		{
			int planeNum = (cap - GL_CLIP_PLANE0);
			DWORD clipEnable = D3DState.EnableState.clipPlaneEnableMask & (1 << planeNum);
			if (clipEnable != value) {
				if (value)
					D3DState.EnableState.clipPlaneEnableMask |= (1 << planeNum );
				else
					D3DState.EnableState.clipPlaneEnableMask &= ~(1 << planeNum );
				D3DState.TransformState.clippingModified = TRUE;
			}
		}
		break;

	case GL_LIGHT0:
	case GL_LIGHT1:
	case GL_LIGHT2:
	case GL_LIGHT3:
	case GL_LIGHT4:
	case GL_LIGHT5:
	case GL_LIGHT6:
	case GL_LIGHT7:
		{
			int lightIndex = cap - GL_LIGHT0;
			if (lightIndex >= 0 && lightIndex < D3DGlobal.maxActiveLights) {
				D3DState.EnableState.lightEnabled[lightIndex] = value;
				D3DGlobal.pDevice->LightEnable(lightIndex, value );
			}
			break;
		}

	case GL_STENCIL_TEST_TWO_SIDE_EXT:
		if (D3DGlobal.hD3DCaps.StencilCaps & D3DSTENCILCAPS_TWOSIDED) {
			D3DState.EnableState.twoSideStencilEnabled = value;
			D3DState_SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, D3DState.EnableState.twoSideStencilEnabled );
		}
		break;

	case GL_AUTO_NORMAL:
	case GL_LINE_STIPPLE:
	case GL_POLYGON_OFFSET_LINE:
	case GL_POLYGON_OFFSET_POINT:
		//silently ignore these caps
		break;

	case GL_MAP1_COLOR_4:
	case GL_MAP1_INDEX:
	case GL_MAP1_NORMAL:
	case GL_MAP1_TEXTURE_COORD_1:
	case GL_MAP1_TEXTURE_COORD_2:
	case GL_MAP1_TEXTURE_COORD_3:
	case GL_MAP1_TEXTURE_COORD_4:
	case GL_MAP1_VERTEX_3:
	case GL_MAP1_VERTEX_4:
	case GL_MAP2_COLOR_4:
	case GL_MAP2_INDEX:
	case GL_MAP2_NORMAL:
	case GL_MAP2_TEXTURE_COORD_1:
	case GL_MAP2_TEXTURE_COORD_2:
	case GL_MAP2_TEXTURE_COORD_3:
	case GL_MAP2_TEXTURE_COORD_4:
	case GL_MAP2_VERTEX_3:
	case GL_MAP2_VERTEX_4:
		logPrintf("WARNING: evaluators are not supported\n");
		break;

	case GL_POLYGON_SMOOTH:
	case GL_POLYGON_STIPPLE:
		if (value) logPrintf("WARNING: polygon smooth and stipple are not supported\n", cap);
		break;
	case GL_POINT_SMOOTH:
		if (value) logPrintf("WARNING: point smooth is not supported\n", cap);
		break;

	case GL_INDEX_LOGIC_OP:
	case GL_COLOR_LOGIC_OP:
		if (value) logPrintf("WARNING: logic operations are not supported\n", cap);
		break;

	case GL_MODULATE:
		//Kingpin keeps calling this... wtf?
		break;

	default:
		logPrintf("WARNING: glEnable/glDisable( 0x%x ) unimplemented\n", cap);
		D3DGlobal.lastError = E_INVALID_ENUM;
		break;
	}
}

static inline void D3DState_ChangeVertexArrayStateBit( DWORD bit, DWORD value )
{
	if (value)
		D3DState.ClientVertexArrayState.vertexArrayEnable |= bit;
	else
		D3DState.ClientVertexArrayState.vertexArrayEnable &= ~bit;
}

static void D3DState_EnableDisableClientState( GLenum cap, DWORD value )
{
	switch (cap) {
	case GL_VERTEX_ARRAY:
		D3DState_ChangeVertexArrayStateBit( VA_ENABLE_VERTEX_BIT, value );
		break;
	case GL_NORMAL_ARRAY:
		D3DState_ChangeVertexArrayStateBit( VA_ENABLE_NORMAL_BIT, value );
		break;
	case GL_COLOR_ARRAY:
		D3DState_ChangeVertexArrayStateBit( VA_ENABLE_COLOR_BIT, value );
		break;
	case GL_SECONDARY_COLOR_ARRAY_EXT:
		D3DState_ChangeVertexArrayStateBit( VA_ENABLE_COLOR2_BIT, value );
		break;
	case GL_TEXTURE_COORD_ARRAY:
		{
			DWORD vaTextureBit = 1 << (VA_TEXTURE_BIT_SHIFT + D3DState.ClientTextureState.currentClientTMU);
			D3DState_ChangeVertexArrayStateBit( vaTextureBit, value );
		}
		break;
	case GL_FOG_COORDINATE_ARRAY:
		D3DState_ChangeVertexArrayStateBit( VA_ENABLE_FOG_BIT, value );
		break;
	default:
		logPrintf("WARNING: glEnableClientState/glDisableClientState( 0x%x ) unimplemented\n", cap);
		D3DGlobal.lastError = E_INVALID_ENUM;
		break;
	}
}

OPENGL_API void WINAPI glEnable( GLenum cap )
{
	D3DState_EnableDisableState( cap, TRUE );
}

OPENGL_API void WINAPI glDisable( GLenum cap )
{
	D3DState_EnableDisableState( cap, FALSE );
}

OPENGL_API GLboolean WINAPI glIsEnabled( GLenum cap )
{
	return (D3DState_IsEnabledState( cap ) > 0 ? GL_TRUE : GL_FALSE );
}

OPENGL_API void WINAPI glEnableClientState( GLenum cap )
{
	D3DState_EnableDisableClientState( cap, TRUE );
}

OPENGL_API void WINAPI glDisableClientState( GLenum cap )
{
	D3DState_EnableDisableClientState( cap, FALSE );
}

OPENGL_API void WINAPI glHint(GLenum target,  GLenum mode)
{
	switch( target )
	{ 
	case GL_PERSPECTIVE_CORRECTION_HINT:
	case GL_POINT_SMOOTH_HINT:
	case GL_LINE_SMOOTH_HINT:
	case GL_POLYGON_SMOOTH_HINT:
		//Direct3D doesn't implement this...
		break;
	case GL_FOG_HINT:
		if ( mode == GL_FASTEST)
			D3DState.HintState.fogHint = 2;
		else if ( mode == GL_NICEST)
			D3DState.HintState.fogHint = 1;
		else
			D3DState.HintState.fogHint = 0;
		D3DGlobal.pDevice->SetRenderState(D3DRS_FOGTABLEMODE, (!D3DState.FogState.fogCoordMode && D3DState.HintState.fogHint <= 1) ? D3DState.FogState.fogMode : D3DFOG_NONE);
		D3DGlobal.pDevice->SetRenderState(D3DRS_FOGVERTEXMODE, (D3DState.FogState.fogCoordMode || D3DState.HintState.fogHint > 1) ? D3DFOG_NONE : D3DState.FogState.fogMode);
		break;

	default:
		logPrintf("WARNING: glHint - unimplemented hint 0x%x (value 0x%x)\n", target, mode);
		break;
	}
}