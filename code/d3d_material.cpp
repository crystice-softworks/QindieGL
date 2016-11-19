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

//==================================================================================
// Materials
//----------------------------------------------------------------------------------
// Nice concord with Direct3D materials
//==================================================================================

OPENGL_API void WINAPI glColorMaterial( GLenum face, GLenum mode )
{
	static bool warningPrinted = false;

	if( face != GL_FRONT_AND_BACK ) {
		if( !warningPrinted ) {
			warningPrinted = true;
			logPrintf( "WARNING: glColorMaterial: only GL_FRONT_AND_BACK is supported\n" );
		}
	}

	if( D3DState.LightingState.colorMaterial != mode ) {
		D3DState.LightingState.colorMaterial = mode;
		D3DState.LightingState.colorMaterialModified = TRUE;
	}
}

OPENGL_API void WINAPI glMaterialf( GLenum face, GLenum pname, GLfloat param )
{
	static bool warningPrinted = false;

	if( face != GL_FRONT_AND_BACK ) {
		if( !warningPrinted ) {
			warningPrinted = true;
			logPrintf( "WARNING: glMaterialf: only GL_FRONT_AND_BACK is supported\n" );
		}
	}

	switch( pname )
	{
	case GL_SHININESS:
		D3DState.LightingState.currentMaterial.Power = param;
		D3DState.LightingState.currentMaterialModified = TRUE;
		break;
	default:
		logPrintf( "WARNING: unknown glMaterial pname - 0x%x\n", pname );
		break;
	}
}
OPENGL_API void WINAPI glMateriali( GLenum face, GLenum pname, GLint param )
{
	glMaterialf( face, pname,(GLfloat)param );
}

OPENGL_API void WINAPI glMaterialfv( GLenum face, GLenum pname, const GLfloat *params )
{
	static bool warningPrinted = false;

	if( face != GL_FRONT_AND_BACK ) {
		if( !warningPrinted ) {
			warningPrinted = true;
			logPrintf( "WARNING: glMaterialfv: only GL_FRONT_AND_BACK is supported\n" );
		}
	}

	switch( pname )
	{
	case GL_AMBIENT:
		D3DState.LightingState.currentMaterial.Ambient.r = params[0];
		D3DState.LightingState.currentMaterial.Ambient.g = params[1];
		D3DState.LightingState.currentMaterial.Ambient.b = params[2];
		D3DState.LightingState.currentMaterial.Ambient.a = params[3];
		D3DState.LightingState.currentMaterialModified = TRUE;
		break;
	case GL_DIFFUSE:
		D3DState.LightingState.currentMaterial.Diffuse.r = params[0];
		D3DState.LightingState.currentMaterial.Diffuse.g = params[1];
		D3DState.LightingState.currentMaterial.Diffuse.b = params[2];
		D3DState.LightingState.currentMaterial.Diffuse.a = params[3];
		D3DState.LightingState.currentMaterialModified = TRUE;
		break;
	case GL_SPECULAR:
		D3DState.LightingState.currentMaterial.Specular.r = params[0];
		D3DState.LightingState.currentMaterial.Specular.g = params[1];
		D3DState.LightingState.currentMaterial.Specular.b = params[2];
		D3DState.LightingState.currentMaterial.Specular.a = params[3];
		D3DState.LightingState.currentMaterialModified = TRUE;
		break;
	case GL_EMISSION:
		D3DState.LightingState.currentMaterial.Emissive.r = params[0];
		D3DState.LightingState.currentMaterial.Emissive.g = params[1];
		D3DState.LightingState.currentMaterial.Emissive.b = params[2];
		D3DState.LightingState.currentMaterial.Emissive.a = params[3];
		D3DState.LightingState.currentMaterialModified = TRUE;
		break;
	case GL_AMBIENT_AND_DIFFUSE:
		D3DState.LightingState.currentMaterial.Ambient.r = params[0];
		D3DState.LightingState.currentMaterial.Ambient.g = params[1];
		D3DState.LightingState.currentMaterial.Ambient.b = params[2];
		D3DState.LightingState.currentMaterial.Ambient.a = params[3];
		D3DState.LightingState.currentMaterial.Diffuse.r = params[0];
		D3DState.LightingState.currentMaterial.Diffuse.g = params[1];
		D3DState.LightingState.currentMaterial.Diffuse.b = params[2];
		D3DState.LightingState.currentMaterial.Diffuse.a = params[3];
		D3DState.LightingState.currentMaterialModified = TRUE;
		break;
	case GL_SHININESS:
		D3DState.LightingState.currentMaterial.Power = params[0];
		D3DState.LightingState.currentMaterialModified = TRUE;
		break;
	default:
		logPrintf( "WARNING: unknown glMaterialv pname - 0x%x\n", pname );
		break;
	}
}
OPENGL_API void WINAPI glMaterialiv( GLenum face, GLenum pname, const GLint *params )
{
	GLfloat fparams[4];
	fparams[0] =(GLfloat)params[0];
	fparams[1] =(GLfloat)params[1];
	fparams[2] =(GLfloat)params[2];
	fparams[3] =(GLfloat)params[3];
	glMaterialfv( face, pname, fparams );	
}

OPENGL_API void WINAPI glGetMaterialfv( GLenum face, GLenum pname, GLfloat *params )
{
	static bool warningPrinted = false;

	if( face != GL_FRONT_AND_BACK ) {
		if( !warningPrinted ) {
			warningPrinted = true;
			logPrintf( "WARNING: glGetMaterialfv: only GL_FRONT_AND_BACK is supported\n" );
		}
	}

	switch( pname )
	{
	case GL_AMBIENT:
		params[0] = D3DState.LightingState.currentMaterial.Ambient.r;
		params[1] = D3DState.LightingState.currentMaterial.Ambient.g;
		params[2] = D3DState.LightingState.currentMaterial.Ambient.b;
		params[3] = D3DState.LightingState.currentMaterial.Ambient.a;
		break;
	case GL_DIFFUSE:
		params[0] = D3DState.LightingState.currentMaterial.Diffuse.r;
		params[1] = D3DState.LightingState.currentMaterial.Diffuse.g;
		params[2] = D3DState.LightingState.currentMaterial.Diffuse.b;
		params[3] = D3DState.LightingState.currentMaterial.Diffuse.a;
		break;
	case GL_SPECULAR:
		params[0] = D3DState.LightingState.currentMaterial.Specular.r;
		params[1] = D3DState.LightingState.currentMaterial.Specular.g;
		params[2] = D3DState.LightingState.currentMaterial.Specular.b;
		params[3] = D3DState.LightingState.currentMaterial.Specular.a;
		break;
	case GL_EMISSION:
		params[0] = D3DState.LightingState.currentMaterial.Emissive.r;
		params[1] = D3DState.LightingState.currentMaterial.Emissive.g;
		params[2] = D3DState.LightingState.currentMaterial.Emissive.b;
		params[3] = D3DState.LightingState.currentMaterial.Emissive.a;
		break;
	case GL_SHININESS:
		params[0] = D3DState.LightingState.currentMaterial.Power;
		break;
	default:
		logPrintf( "WARNING: unknown glGetMaterialfv pname - 0x%x\n", pname );
		break;
	}
}

OPENGL_API void WINAPI glGetMaterialiv( GLenum face, GLenum pname, GLint *params )
{
	static bool warningPrinted = false;

	if( face != GL_FRONT_AND_BACK ) {
		if( !warningPrinted ) {
			warningPrinted = true;
			logPrintf( "WARNING: glGetMaterialiv: only GL_FRONT_AND_BACK is supported\n" );
		}
	}

	switch( pname )
	{
	case GL_SHININESS:
		params[0] =( GLint )D3DState.LightingState.currentMaterial.Power;
		break;
	default:
		{
			GLfloat fparams[4];
			glGetMaterialfv( face, pname, fparams );
			params[0] =( GLint )fparams[0];
			params[1] =( GLint )fparams[1];
			params[2] =( GLint )fparams[2];
			params[3] =( GLint )fparams[3];
			break;
		}
	}
}
