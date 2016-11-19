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
#ifndef QINDIEGL_D3D_IMMEDIATE_H
#define QINDIEGL_D3D_IMMEDIATE_H

class D3DIMBuffer
{
	static const size_t c_IMBufferGrowSize = 256;
	typedef struct
	{
		float position[4];
		float normal[3];
		DWORD color;
		DWORD color2;
		float texCoord[MAX_D3D_TMU][4];
	} D3DIMBufferVertex;
public:
	D3DIMBuffer();
	~D3DIMBuffer();
	void Begin( GLenum primType );
	void End();
	void AddVertex( float x, float y, float z );
	void AddVertex( float x, float y, float z, float w );

protected:
	void EnsureBufferSize( int numVerts );
	UINT ReorderBufferToFVF( int fvf );
	void SetupTexCoords( D3DIMBufferVertex *pVertex, int stage );

private:
	D3DIMBufferVertex *m_pBuffer;
	DWORD		m_samplerMask;
	GLenum		m_primitiveType;
	int			m_maxVertexCount;
	int			m_vertexCount;
	int			m_passedVertexCount;
	bool		m_bBegan;
	bool		m_bXYZW;
};

#endif //QINDIEGL_D3D_IMMEDIATE_H