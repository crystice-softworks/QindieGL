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
#ifndef QINDIEGL_D3D_MATRIX_STACK_H
#define QINDIEGL_D3D_MATRIX_STACK_H

#define D3D_MAX_MATRIX_STACK_DEPTH		32

class D3DStateMatrix
{
public:
	D3DStateMatrix();
	~D3DStateMatrix() {}

	void set_dirty();
	void set_identity();

	operator D3DXMATRIX *()				{ return &m_matrix; }
	operator const D3DXMATRIX *() const	{ return &m_matrix; }
	const D3DXMATRIX *inverse() 		{ check_inverse(); return &m_inverse; }
	const D3DXMATRIX *transpose() 		{ check_transpose(); return &m_transpose; }
	const D3DXMATRIX *invtrans()		{ check_invtrans(); return &m_invtrans; }

private:
	void check_inverse();
	void check_transpose();
	void check_invtrans();

private:
	D3DXMATRIX	m_matrix;
	D3DXMATRIX	m_inverse;
	D3DXMATRIX	m_transpose;
	D3DXMATRIX	m_invtrans;
	BOOL		m_inverse_dirty;
	BOOL		m_transpose_dirty;
	BOOL		m_invtrans_dirty;
	BOOL		m_identity;
};

class D3DMatrixStack
{
public:
	D3DMatrixStack();
	~D3DMatrixStack() {}

	D3DStateMatrix &top()				{ return m_Stack[m_iStackDepth]; }
	const D3DStateMatrix &top() const	{ return m_Stack[m_iStackDepth]; }
	int stack_depth() const				{ return m_iStackDepth; }
	int max_stack_depth() const			{ return D3D_MAX_MATRIX_STACK_DEPTH; }

	void load_identity();
	void load( const GLfloat *m );
	void multiply( const GLfloat *m );

	HRESULT push();
	HRESULT pop();

private:
	int				m_iStackDepth;
	D3DStateMatrix	m_Stack[D3D_MAX_MATRIX_STACK_DEPTH];
};

#endif //QINDIEGL_D3D_MATRIX_STACK_H