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

//==================================================================================
// Evaluators
//----------------------------------------------------------------------------------
// TODO: implement them like in Mesa lib?
//==================================================================================

OPENGL_API void WINAPI glEvalCoord1d( GLdouble )
{
	logPrintf("WARNING: glEvalCoord1d: evaluators are not supported\n");
}

OPENGL_API void WINAPI glEvalCoord1dv( const GLdouble* )
{
	logPrintf("WARNING: glEvalCoord1dv: evaluators are not supported\n");
}

OPENGL_API void WINAPI glEvalCoord1f( GLfloat )
{
	logPrintf("WARNING: glEvalCoord1f: evaluators are not supported\n");
}

OPENGL_API void WINAPI glEvalCoord1fv( const GLfloat* )
{
	logPrintf("WARNING: glEvalCoord1fv: evaluators are not supported\n");
}

OPENGL_API void WINAPI glEvalCoord2d( GLdouble,  GLdouble )
{
	logPrintf("WARNING: glEvalCoord2d: evaluators are not supported\n");
}

OPENGL_API void WINAPI glEvalCoord2dv( const GLdouble* )
{
	logPrintf("WARNING: glEvalCoord2dv: evaluators are not supported\n");
}

OPENGL_API void WINAPI glEvalCoord2f( GLfloat,  GLfloat )
{
	logPrintf("WARNING: glEvalCoord2f: evaluators are not supported\n");
}

OPENGL_API void WINAPI glEvalCoord2fv( const GLfloat* )
{
	logPrintf("WARNING: glEvalCoord2fv: evaluators are not supported\n");
}

OPENGL_API void WINAPI glEvalMesh1( GLenum, GLint, GLint )
{
	logPrintf("WARNING: glEvalMesh1: evaluators are not supported\n");
}

OPENGL_API void WINAPI glEvalMesh2( GLenum, GLint, GLint, GLint, GLint )
{
	logPrintf("WARNING: glEvalMesh2: evaluators are not supported\n");
}

OPENGL_API void WINAPI glEvalPoint1( GLint )
{
	logPrintf("WARNING: glEvalPoint1: evaluators are not supported\n");
}

OPENGL_API void WINAPI glEvalPoint2( GLint, GLint )
{
	logPrintf("WARNING: glEvalPoint2: evaluators are not supported\n");
}

OPENGL_API void WINAPI glGetMapdv( GLenum, GLenum, GLdouble* )
{
	logPrintf("WARNING: glGetMapdv: evaluators are not supported\n");
}

OPENGL_API void WINAPI glGetMapfv( GLenum, GLenum, GLfloat* )
{
	logPrintf("WARNING: glGetMapfv: evaluators are not supported\n");
}

OPENGL_API void WINAPI glGetMapiv( GLenum, GLenum, GLint* )
{
	logPrintf("WARNING: glGetMapiv: evaluators are not supported\n");
}

OPENGL_API void WINAPI glMap1d( GLenum, GLdouble, GLdouble, GLint, GLint, const GLdouble* )
{
	logPrintf("WARNING: glMap1d: evaluators are not supported\n");
}

OPENGL_API void WINAPI glMap1f( GLenum, GLfloat, GLfloat, GLint, GLint, const GLfloat* )
{
	logPrintf("WARNING: glMap1f: evaluators are not supported\n");
}

OPENGL_API void WINAPI glMap2d( GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, const GLdouble* )
{
	logPrintf("WARNING: glMap2d: evaluators are not supported\n");
}

OPENGL_API void WINAPI glMap2f( GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, const GLfloat* )
{
	logPrintf("WARNING: glMap2f: evaluators are not supported\n");
}

OPENGL_API void WINAPI glMapGrid1d( GLint, GLdouble, GLdouble )
{
	logPrintf("WARNING: glMapGrid1d: evaluators are not supported\n");
}

OPENGL_API void WINAPI glMapGrid1f( GLint, GLfloat, GLfloat )
{
	logPrintf("WARNING: glMapGrid1f: evaluators are not supported\n");
}

OPENGL_API void WINAPI glMapGrid2d( GLint, GLdouble, GLdouble, GLint,  GLdouble, GLdouble )
{
	logPrintf("WARNING: glMapGrid2d: evaluators are not supported\n");
}

OPENGL_API void WINAPI glMapGrid2f( GLint, GLfloat, GLfloat, GLint, GLfloat, GLfloat )
{
	logPrintf("WARNING: glMapGrid2f: evaluators are not supported\n");
}