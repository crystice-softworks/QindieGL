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
// User-defined clip planes
//----------------------------------------------------------------------------------
// FIXME: cache the inverse transpose of modelview matrix?
//==================================================================================

OPENGL_API void WINAPI glClipPlane( GLenum plane,  const GLdouble *equation )
{
	int planeIndex = plane - GL_CLIP_PLANE0;
	if (planeIndex < 0 || planeIndex >= IMPL_MAX_CLIP_PLANES) {
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}

	D3DXPLANE d3dxPlane((GLfloat)equation[0],
						(GLfloat)equation[1],
						(GLfloat)equation[2],
						(GLfloat)equation[3]);
	D3DXPLANE d3dxTPlane;
	D3DXPlaneTransform( &d3dxTPlane, &d3dxPlane, D3DGlobal.modelviewMatrixStack->top().invtrans() );
	
	memcpy( D3DState.TransformState.clipPlane[planeIndex], &d3dxTPlane, sizeof(D3DXPLANE));
	D3DState.TransformState.clipPlaneModified[planeIndex] = TRUE;

	if (D3DState.EnableState.clipPlaneEnableMask & (1 << planeIndex))
		D3DState.TransformState.clippingModified = TRUE;
}

OPENGL_API void WINAPI glGetClipPlane( GLenum plane, GLdouble *equation )
{
	int planeIndex = plane - GL_CLIP_PLANE0;
	if (planeIndex < 0 || planeIndex >= IMPL_MAX_CLIP_PLANES) {
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}
	equation[0] = D3DState.TransformState.clipPlane[planeIndex][0];
	equation[1] = D3DState.TransformState.clipPlane[planeIndex][1];
	equation[2] = D3DState.TransformState.clipPlane[planeIndex][2];
	equation[3] = D3DState.TransformState.clipPlane[planeIndex][3];
}