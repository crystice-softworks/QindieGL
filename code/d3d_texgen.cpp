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
// Texture generation
//----------------------------------------------------------------------------------
// We implement OpenGL texgen functionality in software since D3D's is very poor, 
// and we don't want to use vertex shaders
//==================================================================================

typedef void (*pfnTrVertex)( const GLfloat *vertex, float *output );
typedef void (*pfnTrNormal)( const GLfloat *normal, float *output );

static void TrVertexFunc_Copy( const GLfloat *vertex, float *output )
{
	memcpy( output, vertex, sizeof(GLfloat)*4 );
}

static void TrVertexFunc_TransformByModelview( const GLfloat *vertex, float *output )
{
	D3DXVec3TransformCoord((D3DXVECTOR3*)output, (D3DXVECTOR3*)vertex, D3DGlobal.modelviewMatrixStack->top());
}

static void TrVertexFunc_TransformByModelviewAndNormalize( const GLfloat *vertex, float *output )
{
	D3DXVec3TransformCoord((D3DXVECTOR3*)output, (D3DXVECTOR3*)vertex, D3DGlobal.modelviewMatrixStack->top());
	D3DXVec3Normalize((D3DXVECTOR3*)output, (D3DXVECTOR3*)output);
}

static void TrNormalFunc_TransformByModelview( const GLfloat *normal, float *output )
{
	D3DXVec3TransformNormal((D3DXVECTOR3*)output, (D3DXVECTOR3*)normal, D3DGlobal.modelviewMatrixStack->top().invtrans());
}

static void TexGenFunc_None( int /*stage*/, int coord, const GLfloat* /*vertex*/, const GLfloat* /*normal*/, float *output_texcoord )
{
	*output_texcoord = (coord == 3) ? 1.0f : 0.0f;
}

static void TexGenFunc_ObjectLinear( int stage, int coord, const GLfloat* vertex, const GLfloat* /*normal*/, float *output_texcoord )
{
	const float *p = D3DState.TextureState.TexGen[stage][coord].objectPlane;
	float out_coord = 0.0f;

	for (int i = 0; i < 4; ++i, ++vertex, ++p)
		out_coord += (*vertex) * (*p);
	*output_texcoord = out_coord;
}

static void TexGenFunc_ObjectLinear_SSE( int stage, int coord, const GLfloat* vertex, const GLfloat* /*normal*/, float *output_texcoord )
{
	_declspec(align(16)) float sse_vertex[4];
	_declspec(align(16)) float sse_plane[4];
	float out_coord = 0.0f;

	memcpy( sse_vertex, vertex, sizeof(float)*4 );
	memcpy( sse_plane, D3DState.TextureState.TexGen[stage][coord].objectPlane, sizeof(float)*4 );

	_asm {
		movaps	xmm0, xmmword ptr[sse_vertex]
		mulps	xmm0, xmmword ptr[sse_plane]
		movaps	xmm1, xmm0
		shufps	xmm1, xmm1, 11110101b
		addps	xmm0, xmm1
		movaps	xmm1, xmm0
		shufps	xmm1, xmm1, 00000010b
		addss	xmm0, xmm1
		movss	out_coord, xmm0
	}
	*output_texcoord = out_coord;
}

static void TexGenFunc_EyeLinear( int stage, int coord, const GLfloat* vertex, const GLfloat* /*normal*/, float *output_texcoord )
{
	const float *p = D3DState.TextureState.TexGen[stage][coord].eyePlane;
	float out_coord = 0.0f;
	for (int i = 0; i < 4; ++i, ++vertex, ++p)
		out_coord += (*vertex) * (*p);
	*output_texcoord = out_coord;
}

static void TexGenFunc_EyeLinear_SSE( int stage, int coord, const GLfloat* vertex, const GLfloat* /*normal*/, float *output_texcoord )
{
	_declspec(align(16)) float sse_vertex[4];
	_declspec(align(16)) float sse_plane[4];
	float out_coord = 0.0f;

	memcpy( sse_vertex, vertex, sizeof(float)*4 );
	memcpy( sse_plane, D3DState.TextureState.TexGen[stage][coord].eyePlane, sizeof(float)*4 );

	_asm {
		movaps	xmm0, xmmword ptr[sse_vertex]
		mulps	xmm0, xmmword ptr[sse_plane]
		movaps	xmm1, xmm0
		shufps	xmm1, xmm1, 11110101b
		addps	xmm0, xmm1
		movaps	xmm1, xmm0
		shufps	xmm1, xmm1, 00000010b
		addss	xmm0, xmm1
		movss	out_coord, xmm0
	}
	*output_texcoord = out_coord;
}

static void TexGenFunc_SphereMap( int /*stage*/, int coord, const GLfloat* vertex, const GLfloat* normal, float *output_texcoord )
{
	if (coord == 3) {
		*output_texcoord = 1.0f;
	} else if (coord == 2) {
		*output_texcoord = 0.0f;
	} else {
		float r[3];
		float fdot;
		fdot = 2.0f * (normal[0]*vertex[0] + normal[1]*vertex[1] + normal[2]*vertex[2]);
		r[0] = vertex[0] - normal[0] * fdot;
		r[1] = vertex[1] - normal[1] * fdot;
		r[2] = vertex[2] - normal[2] * fdot + 1.0f;
		fdot = 2.0f * sqrtf(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
		*output_texcoord = r[coord]/fdot + 0.5f;
	}
}

static void TexGenFunc_SphereMap_SSE( int /*stage*/, int coord, const GLfloat* vertex, const GLfloat* normal, float *output_texcoord )
{
	if (coord == 3) {
		*output_texcoord = 1.0f;
	} else if (coord == 2) {
		*output_texcoord = 0.0f;
	} else {
		_declspec(align(16)) float sse_vertex[4];
		_declspec(align(16)) float sse_normal[4];
		_declspec(align(16)) float sse_scale_2x[4] = { 2.0f, 2.0f, 2.0f, 0.0f };
		_declspec(align(16)) float sse_bias_0_0_1[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
		_declspec(align(16)) float sse_bias_one_half[4] = { 0.5f, 0.5f, 0.5f, 0.0f };
		_declspec(align(16)) float out_coord[4];

		memcpy( sse_vertex, vertex, sizeof(float)*3 );
		memcpy( sse_normal, normal, sizeof(float)*3 );
		*(int*)&sse_vertex[3] = 0;
		*(int*)&sse_normal[3] = 0;

		_asm {
			movaps	xmm1, xmmword ptr[sse_vertex]
			movaps	xmm2, xmmword ptr[sse_normal]
			movaps	xmm4, xmmword ptr[sse_bias_one_half]

			//xmm0[0,1,2] = 2.0f * (normal[0]*vertex[0], normal[1]*vertex[1], normal[2]*vertex[2])
			movaps	xmm0, xmmword ptr[sse_scale_2x]
			mulps	xmm0, xmm1
			mulps	xmm0, xmm2
			//xmm0[0,1,2] = 2.0f * (normal[0]*vertex[0] + normal[1]*vertex[1] + normal[2]*vertex[2])
			movaps	xmm3, xmm0
			shufps	xmm0, xmm0, 11000000b
			shufps	xmm3, xmm3, 10010101b
			addps	xmm0, xmm3
			shufps	xmm3, xmm3, 11111111b
			addps	xmm0, xmm3
			//xmm1[0,1,2] = vertex[0,1,2] - normal[0,1,2] * fdot + { 0, 0, 1 };
			mulps	xmm2, xmm0
			subps	xmm1, xmm2
			addps	xmm1, xmmword ptr[sse_bias_0_0_1]
			//xmm0[0,1,2] = 2.0f * sqrtf(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
			movaps	xmm0, xmm1
			mulps	xmm0, xmm0
			movaps	xmm3, xmm0
			shufps	xmm3, xmm3, 11001001b
			addss	xmm0, xmm3
			shufps	xmm3, xmm3, 11000001b
			addss	xmm0, xmm3
			rsqrtss	xmm0, xmm0
			mulss	xmm0, xmm4
			shufps	xmm0, xmm0, 11000000b
			mulps	xmm1, xmm0
			addps	xmm1, xmm4
			movaps	xmmword ptr[out_coord], xmm1
		}
		*output_texcoord = out_coord[coord];
	}
}

static void TexGenFunc_ReflectionMap( int /*stage*/, int coord, const GLfloat* vertex, const GLfloat* normal, float *output_texcoord )
{
	if (coord == 3) {
		*output_texcoord = 1.0f;
	} else {
		float fdot = 2.0f * (normal[0]*vertex[0] + normal[1]*vertex[1] + normal[2]*vertex[2]);
		*output_texcoord = vertex[coord] - normal[coord] * fdot;
	}
}

static void TexGenFunc_ReflectionMap_SSE( int /*stage*/, int coord, const GLfloat* vertex, const GLfloat* normal, float *output_texcoord )
{
	if (coord == 3) {
		*output_texcoord = 1.0f;
	} else {

		_declspec(align(16)) float sse_vertex[4];
		_declspec(align(16)) float sse_normal[4];
		_declspec(align(16)) float sse_scale_2x[4] = { 2.0f, 2.0f, 2.0f, 0.0f };
		_declspec(align(16)) float out_coord[4];

		memcpy( sse_vertex, vertex, sizeof(float)*3 );
		memcpy( sse_normal, normal, sizeof(float)*3 );
		*(int*)&sse_vertex[3] = 0;
		*(int*)&sse_normal[3] = 0;

		_asm {
			movaps	xmm1, xmmword ptr[sse_vertex]
			movaps	xmm2, xmmword ptr[sse_normal]

			//xmm0[0,1,2] = 2.0f * (normal[0]*vertex[0], normal[1]*vertex[1], normal[2]*vertex[2])
			movaps	xmm0, xmmword ptr[sse_scale_2x]
			mulps	xmm0, xmm1
			mulps	xmm0, xmm2
			//xmm0[0,1,2] = 2.0f * (normal[0]*vertex[0] + normal[1]*vertex[1] + normal[2]*vertex[2])
			movaps	xmm3, xmm0
			shufps	xmm0, xmm0, 11000000b
			shufps	xmm3, xmm3, 10010101b
			addps	xmm0, xmm3
			shufps	xmm3, xmm3, 11111111b
			addps	xmm0, xmm3
			//xmm1[0,1,2] = vertex[0,1,2] - normal[0,1,2] * fdot;
			mulps	xmm2, xmm0
			subps	xmm1, xmm2
			movaps	xmmword ptr[out_coord], xmm1
		}
		*output_texcoord = out_coord[coord];
	}
}

static void TexGenFunc_NormalMap( int /*stage*/, int coord, const GLfloat* /*vertex*/, const GLfloat* normal, float *output_texcoord )
{
	if (coord == 3) {
		*output_texcoord = 1.0f;
	} else {
		*output_texcoord = normal[coord];
	}
}

void SelectTexGenFunc( int stage, int coord )
{
	switch (D3DState.TextureState.TexGen[stage][coord].mode) {
	case GL_OBJECT_LINEAR:
		D3DState.TextureState.TexGen[stage][coord].func = (D3DGlobal.settings.useSSE ? TexGenFunc_ObjectLinear_SSE : TexGenFunc_ObjectLinear);
		D3DState.TextureState.TexGen[stage][coord].trVertex = TrVertexFunc_Copy;
		D3DState.TextureState.TexGen[stage][coord].trNormal = nullptr;
		break;
	case GL_EYE_LINEAR:
		D3DState.TextureState.TexGen[stage][coord].func = (D3DGlobal.settings.useSSE ? TexGenFunc_EyeLinear_SSE : TexGenFunc_EyeLinear);
		D3DState.TextureState.TexGen[stage][coord].trVertex = TrVertexFunc_TransformByModelview;
		D3DState.TextureState.TexGen[stage][coord].trNormal = nullptr;
		break;
	case GL_SPHERE_MAP:
		D3DState.TextureState.TexGen[stage][coord].func = (D3DGlobal.settings.useSSE ? TexGenFunc_SphereMap_SSE : TexGenFunc_SphereMap);
		D3DState.TextureState.TexGen[stage][coord].trVertex = TrVertexFunc_TransformByModelviewAndNormalize;
		D3DState.TextureState.TexGen[stage][coord].trNormal = TrNormalFunc_TransformByModelview;
		break;
	case GL_REFLECTION_MAP_ARB:
		D3DState.TextureState.TexGen[stage][coord].func = (D3DGlobal.settings.useSSE ? TexGenFunc_ReflectionMap_SSE : TexGenFunc_ReflectionMap);
		D3DState.TextureState.TexGen[stage][coord].trVertex = TrVertexFunc_TransformByModelviewAndNormalize;
		D3DState.TextureState.TexGen[stage][coord].trNormal = TrNormalFunc_TransformByModelview;
		break;
	case GL_NORMAL_MAP_ARB:
		D3DState.TextureState.TexGen[stage][coord].func = TexGenFunc_NormalMap;
		D3DState.TextureState.TexGen[stage][coord].trVertex = nullptr;
		D3DState.TextureState.TexGen[stage][coord].trNormal = TrNormalFunc_TransformByModelview;
		break;
	default:
		D3DState.TextureState.TexGen[stage][coord].func = TexGenFunc_None;
		D3DState.TextureState.TexGen[stage][coord].trNormal = nullptr;
		D3DState.TextureState.TexGen[stage][coord].trNormal = nullptr;
		break;
	}
}

template<typename T>
static void SetupTexGen( int stage, int coord, GLenum pname, const T *params )
{
	switch (pname) {
	case GL_TEXTURE_GEN_MODE:
		D3DState.TextureState.TexGen[stage][coord].mode = (GLenum)params[0];
		SelectTexGenFunc( stage, coord );
		break;
	case GL_OBJECT_PLANE:
		D3DState.TextureState.TexGen[stage][coord].objectPlane[0] = (FLOAT)params[0];
		D3DState.TextureState.TexGen[stage][coord].objectPlane[1] = (FLOAT)params[1];
		D3DState.TextureState.TexGen[stage][coord].objectPlane[2] = (FLOAT)params[2];
		D3DState.TextureState.TexGen[stage][coord].objectPlane[3] = (FLOAT)params[3];
		break;
	case GL_EYE_PLANE:
		{
			D3DXPLANE d3dxPlane((GLfloat)params[0],(GLfloat)params[1],(GLfloat)params[2],(GLfloat)params[3]);
			D3DXPLANE d3dxTPlane;
			D3DXPlaneTransform( &d3dxTPlane, &d3dxPlane, D3DGlobal.modelviewMatrixStack->top().invtrans() );
			D3DState.TextureState.TexGen[stage][coord].eyePlane[0] = (FLOAT)d3dxTPlane.a;
			D3DState.TextureState.TexGen[stage][coord].eyePlane[1] = (FLOAT)d3dxTPlane.b;
			D3DState.TextureState.TexGen[stage][coord].eyePlane[2] = (FLOAT)d3dxTPlane.c;
			D3DState.TextureState.TexGen[stage][coord].eyePlane[3] = (FLOAT)d3dxTPlane.d;
		}
		break;
	default:
		logPrintf("WARNING: unknown TexGen pname - 0x%x\n", pname);
		break;
	}
}
template<typename T>
static void GetTexGen( int stage, int coord, GLenum pname, T *params )
{
	switch (pname) {
	case GL_TEXTURE_GEN_MODE:
		params[0] = (T)D3DState.TextureState.TexGen[stage][coord].mode;
		//TODO: update func
		break;
	case GL_OBJECT_PLANE:
		params[0] = (T)D3DState.TextureState.TexGen[stage][coord].objectPlane[0];
		params[1] = (T)D3DState.TextureState.TexGen[stage][coord].objectPlane[1];
		params[2] = (T)D3DState.TextureState.TexGen[stage][coord].objectPlane[2];
		params[3] = (T)D3DState.TextureState.TexGen[stage][coord].objectPlane[3];
		break;
	case GL_EYE_PLANE:
		params[0] = (T)D3DState.TextureState.TexGen[stage][coord].eyePlane[0];
		params[1] = (T)D3DState.TextureState.TexGen[stage][coord].eyePlane[1];
		params[2] = (T)D3DState.TextureState.TexGen[stage][coord].eyePlane[2];
		params[3] = (T)D3DState.TextureState.TexGen[stage][coord].eyePlane[3];
		break;
	default:
		logPrintf("WARNING: unknown TexGen pname - 0x%x\n", pname);
		break;
	}
}

OPENGL_API void WINAPI glTexGenf( GLenum coord,  GLenum pname,  GLfloat param )
{
	SetupTexGen( D3DState.TextureState.currentTMU, coord - GL_S, pname, &param );
}
OPENGL_API void WINAPI glTexGend( GLenum coord,  GLenum pname,  GLdouble param )
{
	SetupTexGen( D3DState.TextureState.currentTMU, coord - GL_S, pname, &param );
}
OPENGL_API void WINAPI glTexGeni( GLenum coord,  GLenum pname,  GLint param )
{
	SetupTexGen( D3DState.TextureState.currentTMU, coord - GL_S, pname, &param );
}
OPENGL_API void WINAPI glTexGenfv( GLenum coord,  GLenum pname,  const GLfloat *params )
{
	SetupTexGen( D3DState.TextureState.currentTMU, coord - GL_S, pname, params );
}
OPENGL_API void WINAPI glTexGendv( GLenum coord,  GLenum pname,  const GLdouble *params )
{
	SetupTexGen( D3DState.TextureState.currentTMU, coord - GL_S, pname, params );
}
OPENGL_API void WINAPI glTexGeniv( GLenum coord,  GLenum pname,  const GLint *params )
{
	SetupTexGen( D3DState.TextureState.currentTMU, coord - GL_S, pname, params );
}
OPENGL_API void WINAPI glGetTexGenfv( GLenum coord,  GLenum pname,  GLfloat *params )
{
	GetTexGen( D3DState.TextureState.currentTMU, coord - GL_S, pname, params );
}
OPENGL_API void WINAPI glGetTexGendv( GLenum coord,  GLenum pname,  GLdouble *params )
{
	GetTexGen( D3DState.TextureState.currentTMU, coord - GL_S, pname, params );
}
OPENGL_API void WINAPI glGetTexGeniv( GLenum coord,  GLenum pname,  GLint *params )
{
	GetTexGen( D3DState.TextureState.currentTMU, coord - GL_S, pname, params );
}