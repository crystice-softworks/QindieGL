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
#include "d3d_matrix_stack.hpp"

D3DStateMatrix :: D3DStateMatrix()
{
	set_dirty();
}

void D3DStateMatrix :: set_dirty()
{
	m_inverse_dirty = TRUE;
	m_transpose_dirty = TRUE;
	m_invtrans_dirty = TRUE;
	m_identity = FALSE;
}

void D3DStateMatrix :: set_identity()
{
	m_inverse_dirty = TRUE;
	m_transpose_dirty = TRUE;
	m_invtrans_dirty = TRUE;
	m_identity = TRUE;
}

void D3DStateMatrix :: check_inverse()
{
	if (m_inverse_dirty) {
		if (m_identity) {
			memcpy( m_inverse, m_matrix, sizeof(D3DXMATRIX) );
		} else {
			D3DXMatrixInverse( &m_inverse, nullptr, &m_matrix );
		}
		m_inverse_dirty = FALSE;
	}
}

void D3DStateMatrix :: check_transpose()
{
	if (m_transpose_dirty) {
		if (m_identity) {
			memcpy( m_transpose, m_matrix, sizeof(D3DXMATRIX) );
		} else {
			D3DXMatrixTranspose( &m_transpose, &m_matrix );
		}
		m_transpose_dirty = FALSE;
	}
}

void D3DStateMatrix :: check_invtrans()
{
	if (m_invtrans_dirty) {
		if (m_identity) {
			memcpy( m_invtrans, m_matrix, sizeof(D3DXMATRIX) );
		} else {
			check_inverse();
			D3DXMatrixTranspose( &m_invtrans, &m_inverse );
		}
		m_invtrans_dirty = FALSE;
	}
}

//----------------------------------------------------------------

D3DMatrixStack :: D3DMatrixStack() : m_iStackDepth( 0 )
{
	for ( int i = 0; i < D3D_MAX_MATRIX_STACK_DEPTH; ++i) {
		D3DXMatrixIdentity( m_Stack[i] );
		m_Stack[i].set_identity();
	}
}

void D3DMatrixStack :: load_identity()
{
	D3DXMatrixIdentity( this->top() );
	this->top().set_identity();
}

void D3DMatrixStack :: load( const GLfloat *m )
{
	memcpy( this->top(), m, sizeof(GLfloat)*16 );
	this->top().set_dirty();
}

void D3DMatrixStack :: multiply( const GLfloat *m )
{
	FLOAT temp[16];
	memcpy( temp, m, sizeof(FLOAT)*16 );
	D3DXMatrixMultiply( this->top(), (D3DXMATRIX*)temp, this->top() );
	this->top().set_dirty();
}

HRESULT D3DMatrixStack :: push()
{
	if (m_iStackDepth == (D3D_MAX_MATRIX_STACK_DEPTH-1))
		return E_STACK_OVERFLOW;
	++m_iStackDepth;
	memcpy(	this->m_Stack[m_iStackDepth], this->m_Stack[m_iStackDepth-1], sizeof(D3DStateMatrix) );
	return S_OK;
}

HRESULT D3DMatrixStack :: pop()
{
	if (!m_iStackDepth)
		return E_STACK_UNDERFLOW;
	--m_iStackDepth;
	return S_OK;
}