/***************************************************************************
* Copyright( C ) 2011-2016, Crystice Softworks.
* 
* This file is part of QindieGL source code.
* Please note that QindieGL is not driver, it's emulator.
* 
* QindieGL source code is free software; you can redistribute it and/or 
* modify it under the terms of the GNU General Public License as 
* published by the Free Software Foundation; either version 2 of 
* the License, or( at your option ) any later version.
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
// Light operations
//==================================================================================

OPENGL_API void WINAPI glGetLightfv( GLenum light, GLenum pname, GLfloat *params )
{
	int lightIndex = light - GL_LIGHT0;
	if( lightIndex < 0 || lightIndex >= IMPL_MAX_LIGHTS ) {
		logPrintf( "WARNING: glGetLightfv - bad light index %i\n", lightIndex );
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}

	switch( pname ) {
	case GL_CONSTANT_ATTENUATION:
		params[0] = D3DState.LightingState.lightAttenuation[lightIndex].x;
		break;
	case GL_LINEAR_ATTENUATION:
		params[0] = D3DState.LightingState.lightAttenuation[lightIndex].y;
		break;
	case GL_QUADRATIC_ATTENUATION:
		params[0] = D3DState.LightingState.lightAttenuation[lightIndex].z;
		break;
	case GL_SPOT_EXPONENT:
		params[0] = D3DState.LightingState.lightSpotExponent[lightIndex];
		break;
	case GL_SPOT_CUTOFF:
		params[0] = D3DState.LightingState.lightSpotCutoff[lightIndex];
		break;
	case GL_AMBIENT:
		params[0] = D3DState.LightingState.lightColorAmbient[lightIndex].r;
		params[1] = D3DState.LightingState.lightColorAmbient[lightIndex].g;
		params[2] = D3DState.LightingState.lightColorAmbient[lightIndex].b;
		params[3] = D3DState.LightingState.lightColorAmbient[lightIndex].a;
		break;
	case GL_DIFFUSE:
		params[0] = D3DState.LightingState.lightColorDiffuse[lightIndex].r;
		params[1] = D3DState.LightingState.lightColorDiffuse[lightIndex].g;
		params[2] = D3DState.LightingState.lightColorDiffuse[lightIndex].b;
		params[3] = D3DState.LightingState.lightColorDiffuse[lightIndex].a;
		break;
	case GL_SPECULAR:
		params[0] = D3DState.LightingState.lightColorSpecular[lightIndex].r;
		params[1] = D3DState.LightingState.lightColorSpecular[lightIndex].g;
		params[2] = D3DState.LightingState.lightColorSpecular[lightIndex].b;
		params[3] = D3DState.LightingState.lightColorSpecular[lightIndex].a;
		break;
	case GL_POSITION:
		params[0] = D3DState.LightingState.lightPosition[lightIndex].x;
		params[1] = D3DState.LightingState.lightPosition[lightIndex].y;
		params[2] = D3DState.LightingState.lightPosition[lightIndex].z;
		params[3] =( D3DState.LightingState.lightType[lightIndex] == D3DLIGHT_DIRECTIONAL ) ? 0.0f : 1.0f;
		break;
	case GL_SPOT_DIRECTION:
		params[0] = D3DState.LightingState.lightDirection[lightIndex].x;
		params[1] = D3DState.LightingState.lightDirection[lightIndex].y;
		params[2] = D3DState.LightingState.lightDirection[lightIndex].z;
		break;
	default:
		logPrintf( "WARNING: glGetLightfv - bad pname 0x%x\n", pname );
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}
}

OPENGL_API void WINAPI glGetLightiv( GLenum light, GLenum pname, GLint *params )
{
	GLfloat fparams[4];
	glGetLightfv( light, pname, fparams );
	params[0] =( GLint )fparams[0];
	params[1] =( GLint )fparams[1];
	params[2] =( GLint )fparams[2];
	params[3] =( GLint )fparams[3];
}

OPENGL_API void WINAPI glLightModelf( GLenum pname, GLfloat param )
{
	switch( pname ) {
	case GL_LIGHT_MODEL_LOCAL_VIEWER:
		D3DState.LightingState.lightModelLocalViewer =( param > 0 ) ? TRUE : FALSE;
		D3DState_SetRenderState( D3DRS_LOCALVIEWER, D3DState.LightingState.lightModelLocalViewer );
		break;
	case GL_LIGHT_MODEL_TWO_SIDE:
		logPrintf( "WARNING: glLightModelf - GL_LIGHT_MODEL_TWO_SIDE is not supported\n" );
		break;
	default:
		logPrintf( "WARNING: glLightModelf - bad pname 0x%x\n", pname );
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}
}
OPENGL_API void WINAPI glLightModelfv( GLenum pname, const GLfloat *params )
{
	switch( pname ) {
	case GL_LIGHT_MODEL_AMBIENT:
		D3DState.LightingState.lightModelAmbient = D3DCOLOR_ARGB( QINDIEGL_CLAMP( params[3] * 255.0f ),
																 QINDIEGL_CLAMP( params[0] * 255.0f ),
																 QINDIEGL_CLAMP( params[1] * 255.0f ),
																 QINDIEGL_CLAMP( params[2] * 255.0f ) );
		D3DState_SetRenderState( D3DRS_AMBIENT, D3DState.LightingState.lightModelAmbient );
		break;
	default:
		logPrintf( "WARNING: glLightModelfv - bad pname 0x%x\n", pname );
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}
}
OPENGL_API void WINAPI glLightModeli( GLenum pname, GLint param )
{
	glLightModelf( pname,( GLfloat )param );
}
OPENGL_API void WINAPI glLightModeliv( GLenum pname, const GLint *params )
{
	GLfloat fparams[4];
	fparams[0] =( GLfloat )params[0];
	fparams[1] =( GLfloat )params[1];
	fparams[2] =( GLfloat )params[2];
	fparams[3] =( GLfloat )params[3];
	glLightModelfv( pname, fparams );
}
OPENGL_API void WINAPI glLightf( GLenum light, GLenum pname, GLfloat param )
{
	int lightIndex = light - GL_LIGHT0;
	if( lightIndex < 0 || lightIndex >= IMPL_MAX_LIGHTS ) {
		logPrintf( "WARNING: glLightf - bad light index %i\n", lightIndex );
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}

	switch( pname ) {
	case GL_CONSTANT_ATTENUATION:
		D3DState.LightingState.lightAttenuation[lightIndex].x = param;
		break;
	case GL_LINEAR_ATTENUATION:
		D3DState.LightingState.lightAttenuation[lightIndex].y = param;
		break;
	case GL_QUADRATIC_ATTENUATION:
		D3DState.LightingState.lightAttenuation[lightIndex].z = param;
		break;
	case GL_SPOT_EXPONENT:
		D3DState.LightingState.lightSpotExponent[lightIndex] = param;
		break;
	case GL_SPOT_CUTOFF:
		D3DState.LightingState.lightSpotCutoff[lightIndex] = param;
		break;
	default:
		logPrintf( "WARNING: glLightf - bad pname 0x%x\n", pname );
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}
}
OPENGL_API void WINAPI glLightfv( GLenum light, GLenum pname, const GLfloat *params )
{
	int lightIndex = light - GL_LIGHT0;
	if( lightIndex < 0 || lightIndex >= IMPL_MAX_LIGHTS ) {
		logPrintf( "WARNING: glLightfv - bad light index %i\n", lightIndex );
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}

	switch( pname ) {
	case GL_AMBIENT:
		D3DState.LightingState.lightColorAmbient[lightIndex].r = params[0];
		D3DState.LightingState.lightColorAmbient[lightIndex].g = params[1];
		D3DState.LightingState.lightColorAmbient[lightIndex].b = params[2];
		D3DState.LightingState.lightColorAmbient[lightIndex].a = params[3];
		break;
	case GL_DIFFUSE:
		D3DState.LightingState.lightColorDiffuse[lightIndex].r = params[0];
		D3DState.LightingState.lightColorDiffuse[lightIndex].g = params[1];
		D3DState.LightingState.lightColorDiffuse[lightIndex].b = params[2];
		D3DState.LightingState.lightColorDiffuse[lightIndex].a = params[3];
		break;
	case GL_SPECULAR:
		D3DState.LightingState.lightColorSpecular[lightIndex].r = params[0];
		D3DState.LightingState.lightColorSpecular[lightIndex].g = params[1];
		D3DState.LightingState.lightColorSpecular[lightIndex].b = params[2];
		D3DState.LightingState.lightColorSpecular[lightIndex].a = params[3];
		break;
	case GL_POSITION:
		{
			D3DXVECTOR3 lpos;
			D3DXVECTOR3 lresult;
			lpos.x = params[0];
			lpos.y = params[1];
			lpos.z = params[2];
			
			if( params[3] == 0.0f ) {
				D3DXVec3TransformNormal( &lresult, &lpos, D3DGlobal.modelviewMatrixStack->top( ) );
				D3DState.LightingState.lightType[lightIndex] = D3DLIGHT_DIRECTIONAL;
				D3DState.LightingState.lightPosition[lightIndex].x = -lresult.x;
				D3DState.LightingState.lightPosition[lightIndex].y = -lresult.y;
				D3DState.LightingState.lightPosition[lightIndex].z = -lresult.z;
			} else {
				lpos.x /= params[3];
				lpos.y /= params[3];
				lpos.z /= params[3];
				D3DXVec3TransformCoord( &lresult, &lpos, D3DGlobal.modelviewMatrixStack->top( ) );
				D3DState.LightingState.lightType[lightIndex] = D3DLIGHT_POINT;
				D3DState.LightingState.lightPosition[lightIndex].x = lresult.x;
				D3DState.LightingState.lightPosition[lightIndex].y = lresult.y;
				D3DState.LightingState.lightPosition[lightIndex].z = lresult.z;
			}
		}
		break;
	case GL_SPOT_DIRECTION:
		{
			D3DXVECTOR3 lpos;
			lpos.x = params[0];
			lpos.y = params[1];
			lpos.z = params[2];
			D3DXVec3TransformNormal( &D3DState.LightingState.lightDirection[lightIndex], &lpos, D3DGlobal.modelviewMatrixStack->top( ) );
		}
		break;
	case GL_CONSTANT_ATTENUATION:
		D3DState.LightingState.lightAttenuation[lightIndex].x = params[0];
		break;
	case GL_LINEAR_ATTENUATION:
		D3DState.LightingState.lightAttenuation[lightIndex].y = params[0];
		break;
	case GL_QUADRATIC_ATTENUATION:
		D3DState.LightingState.lightAttenuation[lightIndex].z = params[0];
		break;
	case GL_SPOT_EXPONENT:
		D3DState.LightingState.lightSpotExponent[lightIndex] = params[0];
		break;
	case GL_SPOT_CUTOFF:
		D3DState.LightingState.lightSpotCutoff[lightIndex] = params[0];
		break;

	default:
		logPrintf( "WARNING: glLightfv - bad pname 0x%x\n", pname );
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}
}
OPENGL_API void WINAPI glLighti( GLenum light, GLenum pname, GLint param )
{
	glLightf( light, pname,( GLfloat )param );
}
OPENGL_API void WINAPI glLightiv( GLenum light, GLenum pname, const GLint *params )
{
	GLfloat fparams[4];
	fparams[0] =( GLfloat )params[0];
	fparams[1] =( GLfloat )params[1];
	fparams[2] =( GLfloat )params[2];
	fparams[3] =( GLfloat )params[3];
	glLightfv( light, pname, fparams );
}