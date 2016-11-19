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

//==================================================================================
// Misc functions
//==================================================================================

OPENGL_API void WINAPI glClear( GLbitfield mask )
{
	if (!D3DGlobal.initialized) {
		D3DGlobal.lastError = E_FAIL;
		return;
	}
	DWORD clearMask = 0;
	if (mask & GL_COLOR_BUFFER_BIT) clearMask |= D3DCLEAR_TARGET;
	if (mask & GL_DEPTH_BUFFER_BIT) clearMask |= D3DCLEAR_ZBUFFER;
	if (mask & GL_STENCIL_BUFFER_BIT) clearMask |= D3DCLEAR_STENCIL;

	HRESULT hr = D3DGlobal.pDevice->Clear( 0, nullptr, clearMask & ~(D3DGlobal.ignoreClearMask), D3DState.ColorBufferState.clearColor, D3DState.DepthBufferState.clearDepth, D3DState.StencilBufferState.clearStencil );
	if (FAILED(hr)) D3DGlobal.lastError = hr;
}
OPENGL_API void WINAPI glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
	DWORD da = (DWORD)(alpha * 255);
	DWORD dr = (DWORD)(red * 255);
	DWORD dg = (DWORD)(green * 255);
	DWORD db = (DWORD)(blue * 255);
	D3DState.ColorBufferState.clearColor = D3DCOLOR_ARGB( da, dr, dg, db );
}
OPENGL_API void WINAPI glClearDepth( GLclampd depth )
{
	D3DState.DepthBufferState.clearDepth = (float)depth;
}
OPENGL_API void WINAPI glClearStencil( GLint s )
{
	D3DState.StencilBufferState.clearStencil = s;
}
OPENGL_API void WINAPI glClearIndex( GLfloat )
{
	// d3d doesn't support color index mode
}
OPENGL_API void WINAPI glClearAccum( GLfloat, GLfloat, GLfloat, GLfloat )
{
	// d3d doesn't support accumulator
	logPrintf("WARNING: glClearAccum: accumulator is not supported\n");
}
OPENGL_API void WINAPI glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha )
{
	DWORD mask = 0;
	if (red) mask |= D3DCOLORWRITEENABLE_RED;
	if (green) mask |= D3DCOLORWRITEENABLE_GREEN;
	if (blue) mask |= D3DCOLORWRITEENABLE_BLUE;
	if (alpha) mask |= D3DCOLORWRITEENABLE_ALPHA;
	if (mask != D3DState.ColorBufferState.colorWriteMask) {
		D3DState.ColorBufferState.colorWriteMask = mask;
		D3DState_SetRenderState( D3DRS_COLORWRITEENABLE, mask );
	}
}
OPENGL_API void WINAPI glCullFace( GLenum mode )
{
	if (D3DState.PolygonState.cullMode != mode) {
		D3DState.PolygonState.cullMode = mode;
		D3DState_SetCullMode();
	}
}
OPENGL_API void WINAPI glFrontFace( GLenum mode )
{
	if (D3DState.PolygonState.frontFace != mode) {
		D3DState.PolygonState.frontFace = mode;
		D3DState_SetCullMode();
	}
}
OPENGL_API void WINAPI glDepthFunc( GLenum func )
{
	DWORD dfunc = UTIL_GLtoD3DCmpFunc(func);
	if (dfunc != D3DState.DepthBufferState.depthTestFunc) {
		D3DState.DepthBufferState.depthTestFunc = dfunc;
		D3DState_SetRenderState( D3DRS_ZFUNC, dfunc );
	}
}
OPENGL_API void WINAPI glDepthMask( GLboolean flag )
{
	if (D3DState.DepthBufferState.depthWriteMask != flag) {
		D3DState.DepthBufferState.depthWriteMask = flag;
		D3DState_SetRenderState( D3DRS_ZWRITEENABLE, flag );
	}
}
OPENGL_API void WINAPI glDepthRange( GLclampd zNear, GLclampd zFar )
{
	D3DState.viewport.MinZ = (float)zNear;
	D3DState.viewport.MaxZ = (float)zFar;
	if (!D3DGlobal.initialized) {
		D3DGlobal.lastError = E_FAIL;
		return;
	}
	HRESULT hr = D3DGlobal.pDevice->SetViewport(&D3DState.viewport);
	if (FAILED(hr)) D3DGlobal.lastError = hr;
}
OPENGL_API void WINAPI glIndexMask( GLuint )
{
	// d3d doesn't support color index mode
}
OPENGL_API void WINAPI glDrawBuffer( GLenum )
{
	// d3d doesn't like us messing with the front buffer, so here
	// we just silently ignore requests to change the draw buffer
}
OPENGL_API void WINAPI glReadBuffer( GLenum )
{
	// d3d doesn't like us messing with the front buffer, so here
	// we just silently ignore requests to change the draw buffer
}
OPENGL_API void WINAPI glPolygonMode( GLenum face, GLenum mode )
{
	if (face != GL_FRONT_AND_BACK) {
		logPrintf("WARNING: glPolygonMode: only GL_FRONT_AND_BACK is supported\n");
	}

	const DWORD dmode = mode + 1 - GL_POINT;

	if (D3DState.PolygonState.fillMode != dmode) {
		D3DState.PolygonState.fillMode = dmode;
		D3DState_SetRenderState( D3DRS_FILLMODE, dmode );
	}
}
OPENGL_API void WINAPI glPolygonOffset( GLfloat factor, GLfloat units )
{
	D3DState.PolygonState.depthBiasFactor = -factor * 0.0025f;
	D3DState.PolygonState.depthBiasUnits = units * 0.000125f;
	D3DState_SetDepthBias();
}
OPENGL_API void WINAPI glPolygonStipple( const GLubyte* )
{
	static bool warningPrinted = false;
	if (!warningPrinted) {
		warningPrinted = true;
		logPrintf("WARNING: glPolygonStipple is not supported\n");
	}
}
OPENGL_API void WINAPI glGetPolygonStipple( GLubyte* )
{
	static bool warningPrinted = false;
	if (!warningPrinted) {
		warningPrinted = true;
		logPrintf("WARNING: glGetPolygonStipple is not supported\n");
	}
}
OPENGL_API void WINAPI glLineStipple( GLint, GLushort )
{
	static bool warningPrinted = false;
	if (!warningPrinted) {
		warningPrinted = true;
		logPrintf("WARNING: glLineStipple is not supported\n");
	}
}
OPENGL_API void WINAPI glLineWidth( GLfloat )
{
	static bool warningPrinted = false;
	if (!warningPrinted) {
		warningPrinted = true;
		logPrintf("WARNING: glLineWidth is not supported\n");
	}
}
OPENGL_API void WINAPI glShadeModel( GLenum mode )
{
	const DWORD dmode = (mode == GL_FLAT) ? D3DSHADE_FLAT : D3DSHADE_GOURAUD;

	if (dmode != D3DState.LightingState.shadeMode) {
		D3DState.LightingState.shadeMode = dmode;
		D3DState_SetRenderState( D3DRS_SHADEMODE, dmode );
	}
}
OPENGL_API void WINAPI glPointSize( GLfloat size )
{
	size = QINDIEGL_MAX( QINDIEGL_MIN( size, D3DGlobal.hD3DCaps.MaxPointSize ), 1.0f );
	if (D3DState.PointState.pointSize != size) {
		D3DState.PointState.pointSize = size;
		D3DState_SetRenderState( D3DRS_POINTSIZE, UTIL_FloatToDword(size) );
	}
}
OPENGL_API void WINAPI glAccum( GLenum, GLfloat )
{
	// d3d doesn't support accumulator
	logPrintf("WARNING: glAccum: accumulator is not supported\n");
}
OPENGL_API void WINAPI glFlush()
{
	// ignore
}
OPENGL_API void WINAPI glFinish()
{
	// we force a Present in our SwapBuffers function so this is unneeded
}
OPENGL_API void WINAPI glViewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
	// translate from OpenGL bottom-left to D3D top-left
	y = D3DGlobal.hCurrentMode.Height - (height + y);

	D3DState.viewport.X = x;
	D3DState.viewport.Y = y;
	D3DState.viewport.Width = width;
	D3DState.viewport.Height = height;
	if (!D3DGlobal.initialized) {
		D3DGlobal.lastError = E_FAIL;
		return;
	}
	HRESULT hr = D3DGlobal.pDevice->SetViewport(&D3DState.viewport);
	if (FAILED(hr)) D3DGlobal.lastError = hr;
}
OPENGL_API void WINAPI glScissor( GLint x, GLint y, GLsizei width, GLsizei height )
{
	// translate from OpenGL bottom-left to D3D top-left
	y = D3DGlobal.hCurrentMode.Height - (height + y);

	D3DState.ScissorState.scissorRect.left = x;
	D3DState.ScissorState.scissorRect.right = x + width;
	D3DState.ScissorState.scissorRect.top = y;
	D3DState.ScissorState.scissorRect.bottom = y + height;
	if (!D3DGlobal.initialized) {
		D3DGlobal.lastError = E_FAIL;
		return;
	}
	HRESULT hr = D3DGlobal.pDevice->SetScissorRect(&D3DState.ScissorState.scissorRect);
	if (FAILED(hr)) D3DGlobal.lastError = hr;
}
OPENGL_API void WINAPI glDebugEntry( DWORD, DWORD )
{
	//what the hell is that?
}
