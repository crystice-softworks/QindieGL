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
#ifndef QINDIEGL_D3D_STATE_H
#define QINDIEGL_D3D_STATE_H

typedef void (*pfnTexGen)( int stage, int coord, const GLfloat *vertex, const float *normal, float *output_texcoord );
typedef void (*pfnTrVertex)( const GLfloat *vertex, float *output );
typedef void (*pfnTrNormal)( const GLfloat *normal, float *output );

typedef struct D3DState_s
{
	D3DVIEWPORT9		viewport;	
	D3DMatrixStack*		currentMatrixStack;
	bool				modelViewMatrixModified;
	bool				projectionMatrixModified;
	bool				textureMatrixModified[MAX_D3D_TMU];
	bool*				currentMatrixModified;

	struct {
		DWORD			alphaTestFunc;
		DWORD			alphaTestReference;
		DWORD			alphaBlendSrcFunc;
		DWORD			alphaBlendDstFunc;
		DWORD			alphaBlendColorARGB;
		DWORD			alphaBlendColorAAAA;
		DWORD			alphaBlendUseColor;
		DWORD			alphaBlendOp;
		D3DCOLOR		clearColor;
		DWORD			colorWriteMask;
		GLenum			glBlendSrc;
		GLenum			glBlendDst;
	} ColorBufferState;
	struct {
		DWORD			currentColor;
		DWORD			currentColor2;
		FLOAT			currentNormal[3];
		FLOAT			currentTexCoord[MAX_D3D_TMU][4];
	} CurrentState;
	struct {
		DWORD			depthTestFunc;
		DWORD			depthWriteMask;
		FLOAT			clearDepth;
	} DepthBufferState;
	struct {
		DWORD			alphaTestEnabled;
		DWORD			alphaBlendEnabled;
		DWORD			ditherEnabled;
		DWORD			depthTestEnabled;
		DWORD			depthBiasEnabled;
		DWORD			cullEnabled;
		DWORD			fogEnabled;
		DWORD			scissorEnabled;
		DWORD			stencilTestEnabled;
		DWORD			twoSideStencilEnabled;
		DWORD			lightingEnabled;
		DWORD			colorMaterialEnabled;
		DWORD			colorSumEnabled;
		DWORD			normalizeEnabled;
		DWORD			clipPlaneEnableMask;
		DWORD			textureEnabled[MAX_D3D_TMU];
		DWORD			texGenEnabled[MAX_D3D_TMU];
		DWORD			textureTargetEnabled[MAX_D3D_TMU][D3D_TEXTARGET_MAX];
		DWORD			lightEnabled[IMPL_MAX_LIGHTS];
	} EnableState;
	struct {
		DWORD			fogColor;
		DWORD			fogCoordMode;
		D3DFOGMODE		fogMode;
		FLOAT			fogStart;
		FLOAT			fogEnd;
		FLOAT			fogDensity;
	} FogState;
	struct {
		DWORD			fogHint;
	} HintState;
	struct {
		DWORD			shadeMode;
		DWORD			lightModelLocalViewer;
		D3DCOLOR		lightModelAmbient;
		DWORD			lightModified[IMPL_MAX_LIGHTS];
		D3DLIGHTTYPE	lightType[IMPL_MAX_LIGHTS];
		D3DCOLORVALUE	lightColorAmbient[IMPL_MAX_LIGHTS];
		D3DCOLORVALUE	lightColorDiffuse[IMPL_MAX_LIGHTS];
		D3DCOLORVALUE	lightColorSpecular[IMPL_MAX_LIGHTS];
		D3DXVECTOR3		lightPosition[IMPL_MAX_LIGHTS];
		D3DXVECTOR3		lightDirection[IMPL_MAX_LIGHTS];
		D3DVECTOR		lightAttenuation[IMPL_MAX_LIGHTS];
		FLOAT			lightSpotExponent[IMPL_MAX_LIGHTS];
		FLOAT			lightSpotCutoff[IMPL_MAX_LIGHTS];
		D3DMATERIAL9	currentMaterial;
		DWORD			currentMaterialModified;
		GLenum			colorMaterial;
		DWORD			colorMaterialModified;
	} LightingState;
	struct {
		FLOAT			pointSize;
	} PointState;
	struct {
		DWORD			cullMode;
		DWORD			frontFace;
		DWORD			fillMode;
		FLOAT			depthBiasFactor;
		FLOAT			depthBiasUnits;
	} PolygonState;
	struct {
		RECT 			scissorRect;
	} ScissorState;
	struct {
		DWORD			stencilTestReference;
		DWORD			stencilTestMask;
		DWORD			stencilWriteMask;
		DWORD			activeStencilFace;
		DWORD			clearStencil;
		DWORD			stencilTestFunc;
		DWORD			stencilTestFailFunc;
		DWORD			stencilTestPassFunc;
		DWORD			stencilTestZFailFunc;
		DWORD			stencilTestFuncCCW;
		DWORD			stencilTestFailFuncCCW;
		DWORD			stencilTestPassFuncCCW;
		DWORD			stencilTestZFailFuncCCW;
	} StencilBufferState;
	struct {
		DWORD			currentTMU;
		DWORD			currentSamplerCount;
		DWORD			currentCombinerCount;
		D3DTextureObject* currentTexture[MAX_D3D_TMU][D3D_TEXTARGET_MAX];
		DWORD			textureSamplerStateChanged;
		DWORD			textureEnableChanged;
		DWORD			textureChanged[MAX_D3D_TMU][D3D_TEXTARGET_MAX];
		DWORD			textureStateChanged[MAX_D3D_TMU];
		FLOAT			textureLodBias[MAX_D3D_TMU];
		DWORD			textureEnvModeChanged[MAX_D3D_TMU];
		DWORD			textureEnvColor;
		DWORD			textureReference;
		struct {
			GLenum		mode;
			FLOAT		objectPlane[4];
			FLOAT		eyePlane[4];
			pfnTexGen	func;
			pfnTrVertex trVertex;
			pfnTrNormal trNormal;
		} TexGen[MAX_D3D_TMU][4];
		struct {
			GLenum		envMode;
			DWORD		colorScale;
			DWORD		alphaScale;
			GLenum		colorOp;
			GLenum		colorArg1;
			GLenum		colorArg2;
			GLenum		colorArg3;
			GLenum		colorOperand1;
			GLenum		colorOperand2;
			GLenum		colorOperand3;
			GLenum		alphaOp;
			GLenum		alphaArg1;
			GLenum		alphaArg2;
			GLenum		alphaArg3;
			GLenum		alphaOperand1;
			GLenum		alphaOperand2;
			GLenum		alphaOperand3;
		} TextureCombineState[MAX_D3D_TMU];
	} TextureState;
	struct {
		GLenum			matrixMode;
		DWORD			texcoordFixEnabled;
		FLOAT			texcoordFix[2];
		FLOAT			clipPlane[IMPL_MAX_CLIP_PLANES][4];
		DWORD			clipPlaneModified[IMPL_MAX_CLIP_PLANES];
		DWORD			clippingModified;
	} TransformState;

	struct {
		DWORD			unpackSwapBytes;
		DWORD			unpackLSBfirst;
		DWORD			unpackRowLength;
		DWORD			unpackImageHeight;
		DWORD			unpackSkipPixels;
		DWORD			unpackSkipRows;
		DWORD			unpackSkipImages;
		DWORD			unpackAlignment;
		DWORD			packSwapBytes;
		DWORD			packLSBfirst;
		DWORD			packRowLength;
		DWORD			packImageHeight;
		DWORD			packSkipPixels;
		DWORD			packSkipRows;
		DWORD			packSkipImages;
		DWORD			packAlignment;
		DWORD			transferMapColor;
		FLOAT			transferRedScale;
		FLOAT			transferRedBias;
		FLOAT			transferGreenScale;
		FLOAT			transferGreenBias;
		FLOAT			transferBlueScale;
		FLOAT			transferBlueBias;
		FLOAT			transferAlphaScale;
		FLOAT			transferAlphaBias;
		DWORD			pixelmapSizeRtoR;
		DWORD			pixelmapSizeGtoG;
		DWORD			pixelmapSizeBtoB;
		DWORD			pixelmapSizeAtoA;
		FLOAT			pixelmapRtoR[IMPL_MAX_PIXEL_MAP_TABLE];
		FLOAT			pixelmapGtoG[IMPL_MAX_PIXEL_MAP_TABLE];
		FLOAT			pixelmapBtoB[IMPL_MAX_PIXEL_MAP_TABLE];
		FLOAT			pixelmapAtoA[IMPL_MAX_PIXEL_MAP_TABLE];
	} ClientPixelStoreState;
	struct {
		DWORD			currentClientTMU;
	} ClientTextureState;
	struct {
		DWORD			vertexArrayEnable;
		D3DVAInfo		vertexInfo;
		D3DVAInfo		normalInfo;
		D3DVAInfo		colorInfo;
		D3DVAInfo		color2Info;
		D3DVAInfo		fogInfo;
		D3DVAInfo		texCoordInfo[MAX_D3D_TMU];
	} ClientVertexArrayState;

} D3DState_t;

extern D3DState_t D3DState;

inline void D3DState_SetRenderState( D3DRENDERSTATETYPE state, DWORD value )
{
	if (!D3DGlobal.initialized) {
		D3DGlobal.lastError = E_FAIL;
		return;
	}
	HRESULT hr = D3DGlobal.pDevice->SetRenderState(state, value);
	if (FAILED(hr)) D3DGlobal.lastError = hr;
}

extern void D3DState_SetDefaults();
extern void D3DState_Apply( GLbitfield mask );
extern bool D3DState_SetMatrixMode();
extern void D3DState_SetCullMode();
extern void D3DState_SetDepthBias();
extern void D3DState_AssureBeginScene();
extern void D3DState_Check();

#endif //QINDIEGL_D3D_STATE_H