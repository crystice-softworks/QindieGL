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
#include "d3d_object.hpp"
#include "d3d_texture.hpp"

//==================================================================================
// OpenGL object management
//==================================================================================

D3DObjectBuffer :: D3DObjectBuffer() : m_size( 0 ), m_pBuffer( nullptr )
{
}

D3DObjectBuffer :: ~D3DObjectBuffer()
{
	for (int i = 0; i < m_size; ++i)
		FreeObjectMemory(&m_pBuffer[i]);

	UTIL_Free(m_pBuffer);
}

HRESULT D3DObjectBuffer :: GenObjects( D3DObjType type, GLsizei count, GLuint *identifiers )
{
	if (!identifiers) return E_INVALIDARG;

	//all identifiers must be subsequent
	if (m_size >= count) {
		for (int i = 0; i + count <= m_size; )	{
			int j;
			for (j = 0; j < count; ++j)	{
				if (m_pBuffer[i+j].type != D3D_OBJECT_TYPE_NONE)
					break;
			}
			if ( j < count ) {
				i += j + 1;
				continue;
			}
			//found a number of identifiers
			for ( int k = 0; k < count; ++k ) {
				m_pBuffer[i+k].type = type;
				m_pBuffer[i+k].data.any = nullptr;
				identifiers[k] = i+k+1;
			}
			return S_OK;
		}
	}

	int oldsize = m_size;
	m_size += count;
	if (!m_pBuffer) {
		m_pBuffer = (D3DObj*)UTIL_Alloc(sizeof(D3DObj) * m_size);
		memset( m_pBuffer, 0, sizeof(D3DObj) * m_size );
	} else {
		m_pBuffer = (D3DObj*)UTIL_Realloc(m_pBuffer, sizeof(D3DObj) * m_size);
		memset( m_pBuffer + oldsize, 0, sizeof(D3DObj) * (m_size - oldsize) );
	}

	if (!m_pBuffer)
		return E_OUTOFMEMORY;

	//allocated a number of identifiers
	for ( int k = 0; k < count; ++k ) {
		m_pBuffer[oldsize+k].type = type;
		m_pBuffer[oldsize+k].data.any = nullptr;
		identifiers[k] = oldsize+k+1;
	}
	return S_OK;
}

HRESULT D3DObjectBuffer :: DeleteObjects( D3DObjType type, GLsizei count, const GLuint *identifiers )
{
	if (!identifiers) return E_INVALIDARG;
	if (!m_pBuffer) return D3DERR_INVALIDCALL;
	for (int i = 0; i < count; ++i) {
		if (identifiers[i] == 0 || identifiers[i] > (GLuint)m_size)
			continue;
		if (m_pBuffer[identifiers[i]-1].type != type)
			continue;

		FreeObjectMemory(&m_pBuffer[identifiers[i]-1]);
		m_pBuffer[identifiers[i]-1].type = D3D_OBJECT_TYPE_NONE;
	}
	return S_OK;
}

GLboolean D3DObjectBuffer :: IsObject( D3DObjType type, GLuint objIdentifier )
{
	if (objIdentifier == 0 || objIdentifier > (GLuint)m_size)
		return GL_FALSE;
	if (m_pBuffer[objIdentifier-1].type != type)
		return GL_FALSE;
	return GL_TRUE;
}

void *D3DObjectBuffer :: GetObjectData( D3DObjType type, GLuint objIdentifier )
{
	if (!objIdentifier) return nullptr;

	if (objIdentifier > (GLuint)m_size || m_pBuffer[objIdentifier-1].type != type)
	{
		//create new object
		if (objIdentifier > (GLuint)m_size) {
			int oldsize = m_size;
			m_size = objIdentifier;
			if (!m_pBuffer) {
				m_pBuffer = (D3DObj*)UTIL_Alloc(sizeof(D3DObj) * m_size);
				memset( m_pBuffer, 0, sizeof(D3DObj) * m_size );
			} else {
				m_pBuffer = (D3DObj*)UTIL_Realloc(m_pBuffer, sizeof(D3DObj) * m_size);
				memset( m_pBuffer + oldsize, 0, sizeof(D3DObj) * (m_size - oldsize) );
			}
			if (!m_pBuffer) return nullptr;
			m_pBuffer[objIdentifier-1].data.any = nullptr;
		} else { //or overwrite existing
			FreeObjectMemory(&m_pBuffer[objIdentifier-1]);
		}
		m_pBuffer[objIdentifier-1].type = type;
	}
	if (!m_pBuffer[objIdentifier-1].data.any) {
		switch (m_pBuffer[objIdentifier-1].type)
		{
		case D3D_OBJECT_TYPE_TEXTURE:
			m_pBuffer[objIdentifier-1].data.texture = new D3DTextureObject(objIdentifier);
			break;
		default:
			break;
		}
	}
	return m_pBuffer[objIdentifier-1].data.any;
}

void D3DObjectBuffer :: FreeObjectMemory( D3DObj *pObject )
{
	if (!pObject)
		return;

	if (pObject->data.any) {
		switch (pObject->type)
		{
		case D3D_OBJECT_TYPE_TEXTURE:
			//pObject->data.texture->DumpTexture();
			delete pObject->data.texture;
			break;
		default:
			break;
		}
		pObject->data.any = nullptr;
	}
}
