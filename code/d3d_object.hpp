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
#ifndef QINDIEGL_D3D_OBJECT_H
#define QINDIEGL_D3D_OBJECT_H

class D3DTextureObject;

typedef enum D3DObjType_e
{
	D3D_OBJECT_TYPE_NONE = 0,
	D3D_OBJECT_TYPE_TEXTURE,
	D3D_OBJECT_FORCE_DWORD = 0x7fffffff
} D3DObjType;

typedef struct D3DObj_s
{
	D3DObjType type;
	union
	{
		D3DTextureObject *texture;
		void *any;
	} data;
} D3DObj;

class D3DObjectBuffer
{
public:
	D3DObjectBuffer();
	~D3DObjectBuffer();
	HRESULT GenObjects( D3DObjType type, GLsizei count, GLuint *identifiers );
	HRESULT DeleteObjects( D3DObjType type, GLsizei count, const GLuint *identifiers );
	GLboolean IsObject( D3DObjType type, GLuint objIdentifier );
	void *GetObjectData( D3DObjType type, GLuint objIdentifier );

protected:
	void FreeObjectMemory( D3DObj *pObject );

private:
	int		m_size;
	D3DObj *m_pBuffer;
};

#endif //QINDIEGL_D3D_OBJECT_H