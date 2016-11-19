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
#ifndef QINDIEGL_D3D_GLOBAL_H
#define QINDIEGL_D3D_GLOBAL_H

typedef IDirect3D9* (WINAPI *pfnDirect3DCreate9)( UINT SDKVersion );

enum eInternalTextureTarget
{
	D3D_TEXTARGET_1D = 0,
	D3D_TEXTARGET_2D,
	D3D_TEXTARGET_3D,
	D3D_TEXTARGET_CUBE,
	D3D_TEXTARGET_MAX
};

enum eTexTypeInternal
{
	D3D_TEXTYPE_GENERIC = 0x0000,
	D3D_TEXTYPE_X24A8 = 0x0001,
	D3D_TEXTYPE_X1R5G5B5 = 0x0002,
	D3D_TEXTYPE_A1R5G5B5 = 0x0003,
	D3D_TEXTYPE_A4R4G4B4 = 0x0004,
	D3D_TEXTYPE_INTENSITY = 0x0005,
	D3D_TEXTYPE_D16 = 0x0100,
	D3D_TEXTYPE_D24X8 = 0x0200,
	D3D_TEXTYPE_D32 = 0x0300,
	D3D_TEXTYPE_D24S8 = 0x0400,
	D3D_TEXTYPE_D15S1 = 0x0500,
	D3D_TEXTYPE_D24X4S4 = 0x0600,
};

#define D3D_TEXTYPE_DEPTHSTENCIL_MASK		0xFF00

enum ePixelPackageInternal
{
	PP_TYPE_UNPACKED = 0,
	PP_TYPE_R3_G3_B2,
	PP_TYPE_R4_G4_B4_A4,
	PP_TYPE_R5_G5_B5_A1,
	PP_TYPE_R8_G8_B8_A8,
	PP_TYPE_R10_G10_B10_A2,
};

enum eVertexArrayEnable
{
	VA_ENABLE_VERTEX_BIT = 0x0001,
	VA_ENABLE_NORMAL_BIT = 0x0002,
	VA_ENABLE_COLOR_BIT = 0x0004,
	VA_ENABLE_COLOR2_BIT = 0x0008,
	VA_ENABLE_FOG_BIT = 0x0010,
	VA_ENABLE_TEXTURE0_BIT = 0x0100,
	VA_ENABLE_TEXTURE1_BIT = 0x0200,
	VA_ENABLE_TEXTURE2_BIT = 0x0400,
	VA_ENABLE_TEXTURE3_BIT = 0x0800,
	VA_ENABLE_TEXTURE4_BIT = 0x1000,
	VA_ENABLE_TEXTURE5_BIT = 0x2000,
	VA_ENABLE_TEXTURE6_BIT = 0x4000,
	VA_ENABLE_TEXTURE7_BIT = 0x8000,
	VA_ENABLE_FORCE_DWORD = 0x7fffffff
};

#define VA_TEXTURE_BIT_SHIFT	8
#define VA_TEXTURE_BIT_MASK		0xff00

typedef struct D3DVAInfo_s
{
	GLint			elementCount;
	GLenum			elementType;
	GLsizei			stride;
	const GLubyte*	data;
	struct {
		GLint		compiledFirst;
		GLsizei		compiledLast;
	} _internal;
} D3DVAInfo;


#define IMPL_MAX_PIXEL_MAP_TABLE	32
#define IMPL_MAX_LIGHTS				8
#define IMPL_MAX_CLIP_PLANES		6

class D3DIMBuffer;
class D3DVABuffer;
class D3DObjectBuffer;
class D3DTextureObject;
class D3DMatrixStack;

typedef struct D3DGlobal_s
{
	bool					initialized;
	bool					deviceLost;
	bool					sceneBegan;
	int						skipCopyImage;
	HWND					hWnd;
	HDC						hDC;
	HGLRC					hGLRC;
	HMODULE					hD3DDll;
	D3DCAPS9				hD3DCaps;
	D3DDISPLAYMODE			hDesktopMode;
	D3DDISPLAYMODE			hCurrentMode;
	D3DPRESENT_PARAMETERS	hPresentParams;
	LPDIRECT3D9				pD3D;
	LPDIRECT3DDEVICE9		pDevice;
	LPDIRECT3DSWAPCHAIN9	pSwapChain;
	LPDIRECT3DSURFACE9		pSystemMemRT;
	LPDIRECT3DSURFACE9		pSystemMemFB;
	int						iBPP;
	bool					vSync;
	bool					hasDepthStencil;
	GLbitfield				ignoreClearMask;
	HRESULT					lastError;
	char					*szRendererName;
	char					*szExtensions;
	char					*szWExtensions;
	D3DMatrixStack			*modelviewMatrixStack;
	D3DMatrixStack			*projectionMatrixStack;
	D3DMatrixStack			*textureMatrixStack[MAX_D3D_TMU];
	int						maxActiveTMU;
	int						maxActiveLights;
	D3DIMBuffer				*pIMBuffer;
	D3DVABuffer				*pVABuffer;
	D3DObjectBuffer			*pObjectBuffer;
	D3DTextureObject*		defaultTexture[D3D_TEXTARGET_MAX];
	int						rgbaBits[4];
	int						depthBits;
	int						stencilBits;
	int						multiSamples;
	int						supportsS3TC;
	struct {
		DWORD				multisample;
		DWORD				projectionFix;
		DWORD				texcoordFix;
		DWORD				useSSE;
	} settings;
	struct {
		GLfloat*			compiledVertexData;
		GLfloat*			compiledNormalData;
		GLfloat*			compiledTexCoordData[MAX_D3D_TMU];
		DWORD*				compiledColorData;
		GLsizei				compiledVertexDataSize;
		GLsizei				compiledNormalDataSize;
		GLsizei				compiledTexCoordDataSize[MAX_D3D_TMU];
		GLsizei				compiledColorDataSize;
	} compiledVertexArray;
} D3DGlobal_t;

extern D3DGlobal_t D3DGlobal;

extern void D3DGlobal_Init( bool clearGlobals );
extern void D3DGlobal_Cleanup( bool cleanupAll );
extern const char* D3DGlobal_FormatToString( D3DFORMAT format );
extern DWORD D3DGlobal_GetRegistryValue( const char *key, const char *section, DWORD defaultValue );
extern void D3DGlobal_CPU_Detect();

#endif //QINDIEGL_D3D_GLOBAL_H