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
#include "d3d_immediate.hpp"
#include "d3d_array.hpp"

//!DO NOT UNCOMMENT THIS UNLESS YOU MAKE PERFORMANCE TESTS!
//#define VA_USE_IMMEDIATE_MODE

//==================================================================================
// Vertex arrays
//==================================================================================

template<typename T> 
static void D3DVA_CopyArrayToFloatsInternal( const D3DVAInfo *pVAInfo, int index, GLfloat *out )
{
	GLsizei stride = pVAInfo->stride;
	if (!stride) stride = sizeof(T) * pVAInfo->elementCount;
	const T *data = reinterpret_cast<const T*>(pVAInfo->data + index * stride);
	for (int i = 0; i < pVAInfo->elementCount; ++i)
		out[i] = (GLfloat)data[i] / std::numeric_limits<T>::max();
}
static void D3DVA_CopyArrayToFloatsInternalFloat( const D3DVAInfo *pVAInfo, int index, GLfloat *out )
{
	GLsizei stride = pVAInfo->stride;
	if (!stride) stride = sizeof(GLfloat) * pVAInfo->elementCount;
	const GLfloat *data = reinterpret_cast<const GLfloat*>(pVAInfo->data + index * stride);
	memcpy( out, data, pVAInfo->elementCount * sizeof(GLfloat) );
}
static void D3DVA_CopyArrayToFloatsInternalDouble( const D3DVAInfo *pVAInfo, int index, GLfloat *out )
{
	GLsizei stride = pVAInfo->stride;
	if (!stride) stride = sizeof(GLdouble) * pVAInfo->elementCount;
	const GLdouble *data = reinterpret_cast<const GLdouble*>(pVAInfo->data + index * stride);
	for (int i = 0; i < pVAInfo->elementCount; ++i)
		out[i] = static_cast<GLfloat>( data[i] );
}

static void D3DVA_CopyArrayToFloats( const D3DVAInfo *pVAInfo, int index, GLfloat *out )
{
	switch (pVAInfo->elementType)
	{
	case GL_BYTE:
		D3DVA_CopyArrayToFloatsInternal<GLbyte>(pVAInfo, index, out);
		break;
	case GL_UNSIGNED_BYTE:
		D3DVA_CopyArrayToFloatsInternal<GLubyte>(pVAInfo, index, out);
		break;
	case GL_SHORT:
		D3DVA_CopyArrayToFloatsInternal<GLshort>(pVAInfo, index, out);
		break;
	case GL_UNSIGNED_SHORT:
		D3DVA_CopyArrayToFloatsInternal<GLushort>(pVAInfo, index, out);
		break;
	case GL_INT:
		D3DVA_CopyArrayToFloatsInternal<GLint>(pVAInfo, index, out);
		break;
	case GL_UNSIGNED_INT:
		D3DVA_CopyArrayToFloatsInternal<GLuint>(pVAInfo, index, out);
		break;
	case GL_DOUBLE:
		D3DVA_CopyArrayToFloatsInternalDouble(pVAInfo, index, out);
		break;
	case GL_FLOAT:
	default:
		D3DVA_CopyArrayToFloatsInternalFloat(pVAInfo, index, out);
		break;
	}
}

template<typename T> 
static void D3DVA_CopyArrayToUBytesInternal( const D3DVAInfo *pVAInfo, int index, GLubyte *out )
{
	GLsizei stride = pVAInfo->stride;
	if (!stride) stride = sizeof(T) * pVAInfo->elementCount;
	const T *data = reinterpret_cast<const T*>(pVAInfo->data + index * stride);
	for (int i = 0; i < pVAInfo->elementCount; ++i)
		out[i] = static_cast<GLubyte>(QINDIEGL_CLAMP((GLfloat)data[i] * 255 / std::numeric_limits<T>::max()));
}
template<typename T> 
static void D3DVA_CopyArrayToUBytesInternalFloat( const D3DVAInfo *pVAInfo, int index, GLubyte *out )
{
	GLsizei stride = pVAInfo->stride;
	if (!stride) stride = sizeof(T) * pVAInfo->elementCount;
	const T *data = reinterpret_cast<const T*>(pVAInfo->data + index * stride);
	for (int i = 0; i < pVAInfo->elementCount; ++i)
		out[i] = static_cast<GLubyte>(QINDIEGL_CLAMP((GLfloat)data[i] * 255));
}
static void D3DVA_CopyArrayToUBytesInternalUByte( const D3DVAInfo *pVAInfo, int index, GLubyte *out )
{
	GLsizei stride = pVAInfo->stride;
	if (!stride) stride = pVAInfo->elementCount;
	const GLubyte *data = reinterpret_cast<const GLubyte*>(pVAInfo->data + index * stride);
	memcpy( out, data, pVAInfo->elementCount * sizeof(GLubyte) );
}

static void D3DVA_CopyArrayToUBytes( const D3DVAInfo *pVAInfo, int index, GLubyte *out )
{
	switch (pVAInfo->elementType)
	{
	case GL_BYTE:
		D3DVA_CopyArrayToUBytesInternal<GLbyte>(pVAInfo, index, out);
		break;
	case GL_SHORT:
		D3DVA_CopyArrayToUBytesInternal<GLshort>(pVAInfo, index, out);
		break;
	case GL_UNSIGNED_SHORT:
		D3DVA_CopyArrayToUBytesInternal<GLushort>(pVAInfo, index, out);
		break;
	case GL_INT:
		D3DVA_CopyArrayToUBytesInternal<GLint>(pVAInfo, index, out);
		break;
	case GL_UNSIGNED_INT:
		D3DVA_CopyArrayToUBytesInternal<GLuint>(pVAInfo, index, out);
		break;
	case GL_FLOAT:
		D3DVA_CopyArrayToUBytesInternalFloat<GLfloat>(pVAInfo, index, out);
		break;
	case GL_DOUBLE:
		D3DVA_CopyArrayToUBytesInternalFloat<GLdouble>(pVAInfo, index, out);
		break;
	case GL_UNSIGNED_BYTE:
	default:
		D3DVA_CopyArrayToUBytesInternalUByte(pVAInfo, index, out);
		break;
	}
}

//---------------------------------------------------
// VA buffer uses a concept of "swap frames"
// This means that each time we unlock a buffer,
// an internal intex changes, so next time we will
// lock another buffer. This will give D3D time to
// complete rendering from other buffers and not to
// wait when Lock is issued.
// The maximum number of swap frames is arbitrary,
// however low values will lower the fps, but high
// values will consume a lot of memory.
//---------------------------------------------------

static const GLsizei VABuffer_VB_Grow_Size = 256;
static const GLsizei VABuffer_IB_Grow_Size = 256;

D3DVABuffer :: D3DVABuffer()
{
	m_vertexSize = 0;
	m_indexSize = 0;
	m_lockFirst = 0;
	m_lockCount = 0;
	m_swapFrame = 0;
	for (int i = 0; i < c_MaxSwapFrame; ++i) {
		m_pVertexBuffer[i] = nullptr;
		m_pIndexBuffer[0][i] = nullptr;
		m_pIndexBuffer[1][i] = nullptr;
		m_vbAllocSize[i] = 0;
		m_ibAllocSize[0][i] = 0;
		m_ibAllocSize[1][i] = 0;
	}
}

D3DVABuffer :: ~D3DVABuffer()
{
	GLsizei vbSize = 0;
	GLsizei ibSize = 0;

	for (int i = 0; i < c_MaxSwapFrame; ++i) {
		if (m_pVertexBuffer[i]) {
			vbSize += m_vbAllocSize[i] * sizeof(GLfloat);
			m_pVertexBuffer[i]->Release();
		}
		if (m_pIndexBuffer[0][i]) {
			ibSize += m_ibAllocSize[0][i] * sizeof(GLushort);
			m_pIndexBuffer[0][i]->Release();
		}
		if (m_pIndexBuffer[1][i]) {
			ibSize += m_ibAllocSize[1][i] * sizeof(GLuint);
			m_pIndexBuffer[1][i]->Release();
		}
	}

	logPrintf("D3DVABuffer: %.2f kb vertex data, %.2f kb index data [%i swap frames]\n", vbSize / 1024.0f, ibSize / 1024.0f, c_MaxSwapFrame );
}

void D3DVABuffer :: SetMinimumVertexBufferSize( int numVerts )
{
	if (m_vbAllocSize[m_swapFrame] >= numVerts * m_vertexSize)
		return;

	if (m_pVertexBuffer[m_swapFrame])
		m_pVertexBuffer[m_swapFrame]->Release();

	m_vbAllocSize[m_swapFrame] = QINDIEGL_MAX( VABuffer_VB_Grow_Size, numVerts ) * m_vertexSize;
	HRESULT hr = D3DGlobal.pDevice->CreateVertexBuffer( m_vbAllocSize[m_swapFrame] * sizeof(GLfloat), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 
				 									    0, D3DPOOL_DEFAULT, &m_pVertexBuffer[m_swapFrame], nullptr );

	if (FAILED(hr))
		D3DGlobal.lastError = hr;
}

int D3DVABuffer :: SetMinimumIndexBufferSize( int numIndices )
{
	//select either 16-bit or 32-bit index buffer
	int currentIndexBuffer = 0;
	m_indexSize = 2;
	if (numIndices > USHRT_MAX) {
		++currentIndexBuffer;
		m_indexSize += 2;
	}

	if (m_ibAllocSize[currentIndexBuffer][m_swapFrame] >= numIndices * m_indexSize)
		return currentIndexBuffer;

	if (m_pIndexBuffer[currentIndexBuffer][m_swapFrame])
		m_pIndexBuffer[currentIndexBuffer][m_swapFrame]->Release();

	m_ibAllocSize[currentIndexBuffer][m_swapFrame] = QINDIEGL_MAX( VABuffer_IB_Grow_Size, numIndices ) * m_indexSize;
	HRESULT hr = D3DGlobal.pDevice->CreateIndexBuffer( m_ibAllocSize[currentIndexBuffer][m_swapFrame], D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 
													   currentIndexBuffer ? D3DFMT_INDEX32 : D3DFMT_INDEX16,
				 									   D3DPOOL_DEFAULT, &m_pIndexBuffer[currentIndexBuffer][m_swapFrame], nullptr );

	if (FAILED(hr))
		D3DGlobal.lastError = hr;

	return currentIndexBuffer;
}

void D3DVABuffer :: SetupTexCoords( const float *texcoords, const float *position, const float *normal, int stage, float *out_texcoords )
{
	if (!D3DState.EnableState.texGenEnabled[stage]) {
		memcpy( out_texcoords, texcoords, sizeof(float)*4 );
		return;
	}

	const float *in_coords = texcoords;
	float *out_coords = out_texcoords;

	GLenum currentGen( ~0u );
	float tr_position[4];
	float tr_normal[3];

	for (int i = 0; i < 4; ++i, ++in_coords, ++out_coords) {
		if (!(D3DState.EnableState.texGenEnabled[stage] & (1 << i))) {
			*out_coords = *in_coords;
		} else {
			if (currentGen != D3DState.TextureState.TexGen[stage][i].mode) {
				if (D3DState.TextureState.TexGen[stage][i].trVertex)
					D3DState.TextureState.TexGen[stage][i].trVertex( position, tr_position );
				if (D3DState.TextureState.TexGen[stage][i].trNormal)
					D3DState.TextureState.TexGen[stage][i].trNormal( normal, tr_normal );
				currentGen = D3DState.TextureState.TexGen[stage][i].mode;
			}
			D3DState.TextureState.TexGen[stage][i].func( stage, i, tr_position, tr_normal, out_coords );
		}
	}
}

void D3DVABuffer :: Lock( GLint first, GLint last )
{
	float defaultNormal[3] = { 0, 0, 1 };
	float vertexData[4];
	float normalData[3];

	if (first < 0 || last < first) {
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}
	if (m_lockCount > 0) {
		D3DGlobal.lastError = E_FAIL;
		return;
	}

	GLsizei count = last - first + 1;

	//Compute vertex size and FVF
	int fvf = (D3DFVF_DIFFUSE) | ((D3DState.ClientVertexArrayState.vertexInfo.elementCount == 4) ? D3DFVF_XYZW : D3DFVF_XYZ);
	int numVertexCoords = (D3DState.ClientVertexArrayState.vertexInfo.elementCount == 4) ? 4 : 3;
	m_vertexSize = numVertexCoords + 1;

	if (D3DState.ClientVertexArrayState.vertexArrayEnable & VA_ENABLE_NORMAL_BIT) {
		m_vertexSize += D3DState.ClientVertexArrayState.normalInfo.elementCount;
		fvf |= D3DFVF_NORMAL;
	}
	
	if (D3DState.ClientVertexArrayState.vertexArrayEnable & (VA_ENABLE_COLOR2_BIT|VA_ENABLE_FOG_BIT)) {
		++m_vertexSize;
		fvf |= D3DFVF_SPECULAR;
	}
	
	int numSamplers = 0;
	for ( int j = 0; j < D3DGlobal.maxActiveTMU; ++j ) {
		if (D3DState.EnableState.textureEnabled[j]) {
			m_vertexSize += 4;
			fvf |= D3DFVF_TEXCOORDSIZE4(numSamplers);
			++numSamplers;
		}
	}
	fvf |= (numSamplers << D3DFVF_TEXCOUNT_SHIFT);

	//Check if vertex buffer has enough space
	SetMinimumVertexBufferSize( count );
	if (!m_pVertexBuffer)
		return;

	//Lock vertex buffer
	GLfloat *pLockedVertices = nullptr;
	HRESULT hr = m_pVertexBuffer[m_swapFrame]->Lock( 0, count * m_vertexSize * sizeof(GLfloat), 
													 (void**)&pLockedVertices, D3DLOCK_DISCARD );
	if (FAILED(hr)) {
		D3DGlobal.lastError = hr;
		return;
	}
	
	//Fill vertex buffer with data
	for (int i = 0; i < count; ++i) {
		const int elemIndex = first + i;
		vertexData[2] = 0.0f;
		vertexData[3] = 1.0f;
		
		if (elemIndex >= D3DState.ClientVertexArrayState.vertexInfo._internal.compiledFirst &&
			elemIndex <= D3DState.ClientVertexArrayState.vertexInfo._internal.compiledLast) {
			memcpy( vertexData, D3DGlobal.compiledVertexArray.compiledVertexData + elemIndex*numVertexCoords, sizeof(GLfloat)*numVertexCoords );
		} else {
			D3DVA_CopyArrayToFloats( &D3DState.ClientVertexArrayState.vertexInfo, elemIndex, vertexData ); 
		}
		memcpy(pLockedVertices, vertexData, sizeof(GLfloat)*numVertexCoords);
		pLockedVertices += numVertexCoords;
	
		if (fvf & D3DFVF_NORMAL) {
			if (elemIndex >= D3DState.ClientVertexArrayState.normalInfo._internal.compiledFirst &&
				elemIndex <= D3DState.ClientVertexArrayState.normalInfo._internal.compiledLast) {
				memcpy( normalData, D3DGlobal.compiledVertexArray.compiledNormalData + elemIndex*3, sizeof(GLfloat)*3 );
			} else {
				D3DVA_CopyArrayToFloats( &D3DState.ClientVertexArrayState.normalInfo, elemIndex, normalData );
			}
			memcpy(pLockedVertices, normalData, sizeof(normalData));
			pLockedVertices += 3;
		} else {
			memcpy(normalData, defaultNormal, sizeof(defaultNormal));
		}

		if (D3DState.ClientVertexArrayState.vertexArrayEnable & VA_ENABLE_COLOR_BIT) {
			if (elemIndex >= D3DState.ClientVertexArrayState.colorInfo._internal.compiledFirst &&
				elemIndex <= D3DState.ClientVertexArrayState.colorInfo._internal.compiledLast) {
				*(DWORD*)pLockedVertices = D3DGlobal.compiledVertexArray.compiledColorData[elemIndex];
			} else {
				GLubyte color[4] = { 255, 255, 255, 255 };
				D3DVA_CopyArrayToUBytes( &D3DState.ClientVertexArrayState.colorInfo, elemIndex, color );
				*(DWORD*)pLockedVertices = D3DCOLOR_ARGB( color[3], color[0], color[1], color[2] );
			}
		} else {
			*(DWORD*)pLockedVertices = D3DState.CurrentState.currentColor;
		}
		++pLockedVertices;
		
		if (fvf & D3DFVF_SPECULAR) {
			GLubyte color[4] = { 0, 0, 0, 0 };
			if (D3DState.ClientVertexArrayState.vertexArrayEnable & VA_ENABLE_COLOR2_BIT) {
				D3DVA_CopyArrayToUBytes( &D3DState.ClientVertexArrayState.color2Info, elemIndex, color );
			}
			if (D3DState.ClientVertexArrayState.vertexArrayEnable & VA_ENABLE_FOG_BIT) {
				D3DVA_CopyArrayToUBytes( &D3DState.ClientVertexArrayState.fogInfo, elemIndex, color + 3 );
			}
			*(DWORD*)pLockedVertices = D3DCOLOR_ARGB( color[3], color[0], color[1], color[2] );
			++pLockedVertices;
		}

		for ( int j = 0; j < D3DGlobal.maxActiveTMU; ++j ) {
			if (D3DState.EnableState.textureEnabled[j]) {
				DWORD vaTextureBit = 1 << (VA_TEXTURE_BIT_SHIFT + j);
				if (D3DState.ClientVertexArrayState.vertexArrayEnable & vaTextureBit) {
					if (elemIndex >= D3DState.ClientVertexArrayState.texCoordInfo[j]._internal.compiledFirst &&
						elemIndex <= D3DState.ClientVertexArrayState.texCoordInfo[j]._internal.compiledLast) {
						SetupTexCoords( D3DGlobal.compiledVertexArray.compiledTexCoordData[j] + elemIndex*4, vertexData, normalData, j, pLockedVertices );
					} else {
						GLfloat texcoord[4] = { 0, 0, 0, 1 };
						D3DVA_CopyArrayToFloats( &D3DState.ClientVertexArrayState.texCoordInfo[j], elemIndex, texcoord );
						if (D3DState.TransformState.texcoordFixEnabled) {
							texcoord[0] += D3DState.TransformState.texcoordFix[0];
							texcoord[1] += D3DState.TransformState.texcoordFix[1];
						}
						SetupTexCoords( texcoord, vertexData, normalData, j, pLockedVertices );
					}
				} else if (D3DState.EnableState.texGenEnabled[j]) {
					GLfloat texcoord[4] = { 0, 0, 0, 1 };
					if (D3DState.TransformState.texcoordFixEnabled) {
						texcoord[0] += D3DState.TransformState.texcoordFix[0];
						texcoord[1] += D3DState.TransformState.texcoordFix[1];
					}
					SetupTexCoords( texcoord, vertexData, normalData, j, pLockedVertices );
				}
				pLockedVertices += 4;
			}
		}
	}

	//Unlock vertex buffer
	m_pVertexBuffer[m_swapFrame]->Unlock();

	//Set stream source
	hr = D3DGlobal.pDevice->SetStreamSource( 0, m_pVertexBuffer[m_swapFrame], 0, m_vertexSize * sizeof(GLfloat) );
	if (FAILED(hr)) {
		D3DGlobal.lastError = hr;
		return;
	}

	//Set current FVF
	hr = D3DGlobal.pDevice->SetFVF( fvf );
	if (FAILED(hr)) {
		D3DGlobal.lastError = hr;
		return;
	}

	//And we are done
	m_lockFirst = first;
	m_lockCount = count;
}

void D3DVABuffer :: Unlock()
{
	if (m_lockCount <= 0) {
		D3DGlobal.lastError = E_FAIL;
		return;
	}

	m_lockFirst = 0;
	m_lockCount = 0;

	++m_swapFrame;
	if (m_swapFrame >= c_MaxSwapFrame)
		m_swapFrame = 0;
}

template<typename T>
void D3DVABuffer :: SetIndices( GLenum mode, GLuint start, GLuint end, GLsizei count, const T *indices )
{
	if ( mode == GL_POINTS ) {
		//We don't support point lists in VA mode!
		return;
	}

	const bool bValidRange = (end >= start) && (start != ~0u);

	m_primitiveType = mode;

	//For GL_QUADS, we add 2 additional indices per quad
	//For GL_LINE_LOOP, we add 1 additional index
	m_primitiveIndexCount = count;
	if ( mode == GL_QUADS )
		m_primitiveIndexCount += (count >> 1);
	else if ( mode == GL_LINE_LOOP )
		++m_primitiveIndexCount;

	//Set index buffer size
	int currentIndexBuffer = SetMinimumIndexBufferSize( m_primitiveIndexCount );

	//Lock index buffer
	GLvoid *pLockedIndices = nullptr;
	HRESULT hr = m_pIndexBuffer[currentIndexBuffer][m_swapFrame]->Lock( 0, m_primitiveIndexCount * m_indexSize, (void**)&pLockedIndices, D3DLOCK_DISCARD );
	if (FAILED(hr)) {
		D3DGlobal.lastError = hr;
		return;
	}

	GLuint minVertexIndex;
	GLuint maxVertexIndex;

	if (!indices) {
		//Generate indices by ourselves
		//Fill index buffer with data
		GLuint dstIndex = 0;
		for (GLsizei i = 0; i < count; ++i) {
			if ((mode == GL_QUADS) && ((i % 4) == 3)) {
				SetIndex(pLockedIndices, dstIndex, i-3);
				SetIndex(pLockedIndices, dstIndex+1, i-1);
				dstIndex+=2;
			}
			//add index i
			SetIndex(pLockedIndices, dstIndex, i);
			++dstIndex;
		}

		if ( mode == GL_LINE_LOOP ) {
			SetIndex(pLockedIndices, dstIndex, 0);
		}
		minVertexIndex = 0;
		maxVertexIndex = count-1;
	} else {
		//Use provided index data
		//Fill index buffer with data
		minVertexIndex = UINT_MAX;
		maxVertexIndex = 0;

		GLuint dstIndex = 0;
		for (GLsizei i = 0; i < count; ++i) {
			if ((mode == GL_QUADS) && ((i % 4) == 3)) {
				SetIndex<T>(pLockedIndices, dstIndex, indices[i-3]);
				SetIndex<T>(pLockedIndices, dstIndex+1, indices[i-1]);
				dstIndex+=2;
			}
			//add index i
			SetIndex<T>(pLockedIndices, dstIndex, indices[i]);
			++dstIndex;

			if (!m_lockCount && !bValidRange) {
				if (indices[i] < minVertexIndex)
					minVertexIndex = indices[i];
				if (indices[i] > maxVertexIndex)
					maxVertexIndex = indices[i];
			}
		}

		if ( mode == GL_LINE_LOOP ) {
			SetIndex<T>(pLockedIndices, dstIndex, indices[0]);
		}
	}

	//Unlock index buffer
	m_pIndexBuffer[currentIndexBuffer][m_swapFrame]->Unlock();

	if (!m_lockCount) {
		if (bValidRange)
			Lock( start, end );
		else
			Lock( minVertexIndex, maxVertexIndex );
	}

	if (!indices)
		m_lockFirst = 0;	//our own indices are already offset

	//Set indices
	hr = D3DGlobal.pDevice->SetIndices( m_pIndexBuffer[currentIndexBuffer][m_swapFrame] );
	if (FAILED(hr))
		D3DGlobal.lastError = hr;
}

void D3DVABuffer :: DrawPrimitive()
{
	if (!m_primitiveIndexCount || !m_lockCount) 
		return;

	HRESULT hr;

	switch (m_primitiveType)
	{
	case GL_LINES:
		hr = D3DGlobal.pDevice->DrawIndexedPrimitive(D3DPT_LINELIST, -m_lockFirst, 0, m_lockCount, 0, m_primitiveIndexCount / 2);
		break;

	case GL_LINE_STRIP:
	case GL_LINE_LOOP:
		hr = D3DGlobal.pDevice->DrawIndexedPrimitive(D3DPT_LINESTRIP, -m_lockFirst, 0, m_lockCount, 0, m_primitiveIndexCount - 1);
		break;

	case GL_QUADS:
		// quads are converted to triangles upon lock
	case GL_TRIANGLES:
		// D3DPT_TRIANGLELIST models GL_TRIANGLES when used for either a single triangle or multiple triangles
		hr = D3DGlobal.pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, -m_lockFirst, 0, m_lockCount, 0, m_primitiveIndexCount / 3);
		break;

	case GL_QUAD_STRIP:
		// quadstrip is EXACT the same as tristrip
	case GL_TRIANGLE_STRIP:
		// regular tristrip
		hr = D3DGlobal.pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, -m_lockFirst, 0, m_lockCount, 0, m_primitiveIndexCount - 2);
		break;

	case GL_POLYGON:
		// a GL_POLYGON has the same vertex layout and order as a trifan, and can be used interchangably in OpenGL
	case GL_TRIANGLE_FAN:
		// regular trifan
		hr = D3DGlobal.pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, -m_lockFirst, 0, m_lockCount, 0, m_primitiveIndexCount - 2);
		break;
		
	default:
		// unsupported mode
		hr = S_OK;
		logPrintf("WARNING: DrawPrimitive - unsupported mode 0x%x\n", m_primitiveType);
		break;
	}

	if (FAILED(hr))
		D3DGlobal.lastError = hr;
}

//------------------------------------------------------------------------------------------------------

OPENGL_API void WINAPI glColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	D3DState.ClientVertexArrayState.colorInfo.elementCount = size;
	D3DState.ClientVertexArrayState.colorInfo.elementType = type;
	D3DState.ClientVertexArrayState.colorInfo.stride = stride;
	D3DState.ClientVertexArrayState.colorInfo.data = reinterpret_cast<const GLubyte*>(pointer);
	D3DState.ClientVertexArrayState.colorInfo._internal.compiledFirst = 0;
	D3DState.ClientVertexArrayState.colorInfo._internal.compiledLast = -1;
}
OPENGL_API void WINAPI glSecondaryColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	D3DState.ClientVertexArrayState.color2Info.elementCount = size;
	D3DState.ClientVertexArrayState.color2Info.elementType = type;
	D3DState.ClientVertexArrayState.color2Info.stride = stride;
	D3DState.ClientVertexArrayState.color2Info.data = reinterpret_cast<const GLubyte*>(pointer);
}
OPENGL_API void WINAPI glFogCoordPointer( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	D3DState.ClientVertexArrayState.fogInfo.elementCount = 1;
	D3DState.ClientVertexArrayState.fogInfo.elementType = type;
	D3DState.ClientVertexArrayState.fogInfo.stride = stride;
	D3DState.ClientVertexArrayState.fogInfo.data = reinterpret_cast<const GLubyte*>(pointer);
}
OPENGL_API void WINAPI glNormalPointer( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	D3DState.ClientVertexArrayState.normalInfo.elementCount = 3;
	D3DState.ClientVertexArrayState.normalInfo.elementType = type;
	D3DState.ClientVertexArrayState.normalInfo.stride = stride;
	D3DState.ClientVertexArrayState.normalInfo.data = reinterpret_cast<const GLubyte*>(pointer);
	D3DState.ClientVertexArrayState.normalInfo._internal.compiledFirst = 0;
	D3DState.ClientVertexArrayState.normalInfo._internal.compiledLast = -1;
}
OPENGL_API void WINAPI glTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	D3DState.ClientVertexArrayState.texCoordInfo[D3DState.ClientTextureState.currentClientTMU].elementCount = size;
	D3DState.ClientVertexArrayState.texCoordInfo[D3DState.ClientTextureState.currentClientTMU].elementType = type;
	D3DState.ClientVertexArrayState.texCoordInfo[D3DState.ClientTextureState.currentClientTMU].stride = stride;
	D3DState.ClientVertexArrayState.texCoordInfo[D3DState.ClientTextureState.currentClientTMU].data = reinterpret_cast<const GLubyte*>(pointer);
	D3DState.ClientVertexArrayState.texCoordInfo[D3DState.ClientTextureState.currentClientTMU]._internal.compiledFirst = 0;
	D3DState.ClientVertexArrayState.texCoordInfo[D3DState.ClientTextureState.currentClientTMU]._internal.compiledLast = -1;
}
OPENGL_API void WINAPI glVertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	D3DState.ClientVertexArrayState.vertexInfo.elementCount = size;
	D3DState.ClientVertexArrayState.vertexInfo.elementType = type;
	D3DState.ClientVertexArrayState.vertexInfo.stride = stride;
	D3DState.ClientVertexArrayState.vertexInfo.data = reinterpret_cast<const GLubyte*>(pointer);
	D3DState.ClientVertexArrayState.vertexInfo._internal.compiledFirst = 0;
	D3DState.ClientVertexArrayState.vertexInfo._internal.compiledLast = -1;
}
OPENGL_API void WINAPI glEdgeFlagPointer( GLsizei, const GLvoid* )
{
	logPrintf("WARNING: glEdgeFlagPointer is not supported\n");
}
OPENGL_API void WINAPI glIndexPointer( GLenum, GLsizei, const GLvoid* )
{
	logPrintf("WARNING: glIndexPointer is not supported\n");
}

OPENGL_API void WINAPI glInterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer )
{
	D3DState.ClientVertexArrayState.vertexArrayEnable = VA_ENABLE_VERTEX_BIT;
	D3DState.ClientVertexArrayState.vertexInfo._internal.compiledFirst = 0;
	D3DState.ClientVertexArrayState.vertexInfo._internal.compiledLast = -1;

	switch (format) {
	case GL_V2F:
		{
			int realStride = stride;
			if (!realStride) realStride = sizeof(GLfloat) * 2;
			D3DState.ClientVertexArrayState.vertexInfo.elementCount = 2;
			D3DState.ClientVertexArrayState.vertexInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.vertexInfo.stride = realStride;
			D3DState.ClientVertexArrayState.vertexInfo.data = reinterpret_cast<const GLubyte*>(pointer);
		}
		break;
	case GL_V3F:
		{
			int realStride = stride;
			if (!realStride) realStride = sizeof(GLfloat) * 3;
			D3DState.ClientVertexArrayState.vertexInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.vertexInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.vertexInfo.stride = realStride;
			D3DState.ClientVertexArrayState.vertexInfo.data = reinterpret_cast<const GLubyte*>(pointer);
		}
		break;
	case GL_C4UB_V2F:
		{
			int realStride = stride;
			if (!realStride) realStride = 4 + sizeof(GLfloat) * 2;
			D3DState.ClientVertexArrayState.vertexArrayEnable |= VA_ENABLE_COLOR_BIT;
			D3DState.ClientVertexArrayState.colorInfo.elementCount = 4;
			D3DState.ClientVertexArrayState.colorInfo.elementType = GL_UNSIGNED_BYTE;
			D3DState.ClientVertexArrayState.colorInfo.stride = realStride;
			D3DState.ClientVertexArrayState.colorInfo.data = reinterpret_cast<const GLubyte*>(pointer);
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.vertexInfo.elementCount = 2;
			D3DState.ClientVertexArrayState.vertexInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.vertexInfo.stride = realStride;
			D3DState.ClientVertexArrayState.vertexInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 4;
		}
		break;
	case GL_C4UB_V3F:
		{
			int realStride = stride;
			if (!realStride) realStride = 4 + sizeof(GLfloat) * 3;
			D3DState.ClientVertexArrayState.vertexArrayEnable |= VA_ENABLE_COLOR_BIT;
			D3DState.ClientVertexArrayState.colorInfo.elementCount = 4;
			D3DState.ClientVertexArrayState.colorInfo.elementType = GL_UNSIGNED_BYTE;
			D3DState.ClientVertexArrayState.colorInfo.stride = realStride;
			D3DState.ClientVertexArrayState.colorInfo.data = reinterpret_cast<const GLubyte*>(pointer);
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.vertexInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.vertexInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.vertexInfo.stride = realStride;
			D3DState.ClientVertexArrayState.vertexInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 4;
		}
		break;
	case GL_C3F_V3F:
		{
			int realStride = stride;
			if (!realStride) realStride = sizeof(GLfloat) * 6;
			D3DState.ClientVertexArrayState.vertexArrayEnable |= VA_ENABLE_COLOR_BIT;
			D3DState.ClientVertexArrayState.colorInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.colorInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.colorInfo.stride = realStride;
			D3DState.ClientVertexArrayState.colorInfo.data = reinterpret_cast<const GLubyte*>(pointer);
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.vertexInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.vertexInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.vertexInfo.stride = realStride;
			D3DState.ClientVertexArrayState.vertexInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 3*sizeof(GLfloat);
		}
		break;
	case GL_N3F_V3F:
		{
			int realStride = stride;
			if (!realStride) realStride = sizeof(GLfloat) * 6;
			D3DState.ClientVertexArrayState.vertexArrayEnable |= VA_ENABLE_NORMAL_BIT;
			D3DState.ClientVertexArrayState.normalInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.normalInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.normalInfo.stride = realStride;
			D3DState.ClientVertexArrayState.normalInfo.data = reinterpret_cast<const GLubyte*>(pointer);
			D3DState.ClientVertexArrayState.normalInfo._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.normalInfo._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.vertexInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.vertexInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.vertexInfo.stride = realStride;
			D3DState.ClientVertexArrayState.vertexInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 3*sizeof(GLfloat);
		}
		break;
	case GL_C4F_N3F_V3F:
		{
			int realStride = stride;
			if (!realStride) realStride = sizeof(GLfloat) * 10;
			D3DState.ClientVertexArrayState.vertexArrayEnable |= (VA_ENABLE_COLOR_BIT | VA_ENABLE_NORMAL_BIT);
			D3DState.ClientVertexArrayState.colorInfo.elementCount = 4;
			D3DState.ClientVertexArrayState.colorInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.colorInfo.stride = realStride;
			D3DState.ClientVertexArrayState.colorInfo.data = reinterpret_cast<const GLubyte*>(pointer);
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.normalInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.normalInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.normalInfo.stride = realStride;
			D3DState.ClientVertexArrayState.normalInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 4*sizeof(GLfloat);
			D3DState.ClientVertexArrayState.normalInfo._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.normalInfo._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.vertexInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.vertexInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.vertexInfo.stride = realStride;
			D3DState.ClientVertexArrayState.vertexInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 7*sizeof(GLfloat);
		}
		break;
	case GL_T2F_V3F:
		{
			int realStride = stride;
			if (!realStride) realStride = sizeof(GLfloat) * 5;
			D3DState.ClientVertexArrayState.vertexArrayEnable |= VA_ENABLE_TEXTURE0_BIT;
			D3DState.ClientVertexArrayState.texCoordInfo[0].elementCount = 2;
			D3DState.ClientVertexArrayState.texCoordInfo[0].elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.texCoordInfo[0].stride = realStride;
			D3DState.ClientVertexArrayState.texCoordInfo[0].data = reinterpret_cast<const GLubyte*>(pointer);
			D3DState.ClientVertexArrayState.texCoordInfo[0]._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.texCoordInfo[0]._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.vertexInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.vertexInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.vertexInfo.stride = realStride;
			D3DState.ClientVertexArrayState.vertexInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 2*sizeof(GLfloat);
		}
		break;
	case GL_T4F_V4F:
		{
			int realStride = stride;
			if (!realStride) realStride = sizeof(GLfloat) * 8;
			D3DState.ClientVertexArrayState.vertexArrayEnable |= VA_ENABLE_TEXTURE0_BIT;
			D3DState.ClientVertexArrayState.texCoordInfo[0].elementCount = 4;
			D3DState.ClientVertexArrayState.texCoordInfo[0].elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.texCoordInfo[0].stride = realStride;
			D3DState.ClientVertexArrayState.texCoordInfo[0].data = reinterpret_cast<const GLubyte*>(pointer);
			D3DState.ClientVertexArrayState.texCoordInfo[0]._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.texCoordInfo[0]._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.vertexInfo.elementCount = 4;
			D3DState.ClientVertexArrayState.vertexInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.vertexInfo.stride = realStride;
			D3DState.ClientVertexArrayState.vertexInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 4*sizeof(GLfloat);
		}
		break;
	case GL_T2F_C4UB_V3F:
		{
			int realStride = stride;
			if (!realStride) realStride = 4 + sizeof(GLfloat) * 5;
			D3DState.ClientVertexArrayState.vertexArrayEnable |= (VA_ENABLE_TEXTURE0_BIT | VA_ENABLE_COLOR_BIT);
			D3DState.ClientVertexArrayState.texCoordInfo[0].elementCount = 2;
			D3DState.ClientVertexArrayState.texCoordInfo[0].elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.texCoordInfo[0].stride = realStride;
			D3DState.ClientVertexArrayState.texCoordInfo[0].data = reinterpret_cast<const GLubyte*>(pointer);
			D3DState.ClientVertexArrayState.texCoordInfo[0]._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.texCoordInfo[0]._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.colorInfo.elementCount = 4;
			D3DState.ClientVertexArrayState.colorInfo.elementType = GL_UNSIGNED_BYTE;
			D3DState.ClientVertexArrayState.colorInfo.stride = realStride;
			D3DState.ClientVertexArrayState.colorInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 2*sizeof(GLfloat);
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.vertexInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.vertexInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.vertexInfo.stride = realStride;
			D3DState.ClientVertexArrayState.vertexInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 2*sizeof(GLfloat) + 4;
		}
		break;
	case GL_T2F_C3F_V3F:
		{
			int realStride = stride;
			if (!realStride) realStride = sizeof(GLfloat) * 8;
			D3DState.ClientVertexArrayState.vertexArrayEnable |= (VA_ENABLE_TEXTURE0_BIT | VA_ENABLE_COLOR_BIT);
			D3DState.ClientVertexArrayState.texCoordInfo[0].elementCount = 2;
			D3DState.ClientVertexArrayState.texCoordInfo[0].elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.texCoordInfo[0].stride = realStride;
			D3DState.ClientVertexArrayState.texCoordInfo[0].data = reinterpret_cast<const GLubyte*>(pointer);
			D3DState.ClientVertexArrayState.texCoordInfo[0]._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.texCoordInfo[0]._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.colorInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.colorInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.colorInfo.stride = realStride;
			D3DState.ClientVertexArrayState.colorInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 2*sizeof(GLfloat);
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.vertexInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.vertexInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.vertexInfo.stride = realStride;
			D3DState.ClientVertexArrayState.vertexInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 5*sizeof(GLfloat);
		}
		break;
	case GL_T2F_N3F_V3F:
		{
			int realStride = stride;
			if (!realStride) realStride = sizeof(GLfloat) * 8;
			D3DState.ClientVertexArrayState.vertexArrayEnable |= (VA_ENABLE_TEXTURE0_BIT | VA_ENABLE_NORMAL_BIT);
			D3DState.ClientVertexArrayState.texCoordInfo[0].elementCount = 2;
			D3DState.ClientVertexArrayState.texCoordInfo[0].elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.texCoordInfo[0].stride = realStride;
			D3DState.ClientVertexArrayState.texCoordInfo[0].data = reinterpret_cast<const GLubyte*>(pointer);
			D3DState.ClientVertexArrayState.texCoordInfo[0]._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.texCoordInfo[0]._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.normalInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.normalInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.normalInfo.stride = realStride;
			D3DState.ClientVertexArrayState.normalInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 2*sizeof(GLfloat);
			D3DState.ClientVertexArrayState.normalInfo._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.normalInfo._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.vertexInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.vertexInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.vertexInfo.stride = realStride;
			D3DState.ClientVertexArrayState.vertexInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 5*sizeof(GLfloat);
		}
		break;
	case GL_T2F_C4F_N3F_V3F:
		{
			int realStride = stride;
			if (!realStride) realStride = sizeof(GLfloat) * 12;
			D3DState.ClientVertexArrayState.vertexArrayEnable |= (VA_ENABLE_TEXTURE0_BIT | VA_ENABLE_NORMAL_BIT | VA_ENABLE_COLOR_BIT);
			D3DState.ClientVertexArrayState.texCoordInfo[0].elementCount = 2;
			D3DState.ClientVertexArrayState.texCoordInfo[0].elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.texCoordInfo[0].stride = realStride;
			D3DState.ClientVertexArrayState.texCoordInfo[0].data = reinterpret_cast<const GLubyte*>(pointer);
			D3DState.ClientVertexArrayState.texCoordInfo[0]._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.texCoordInfo[0]._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.colorInfo.elementCount = 4;
			D3DState.ClientVertexArrayState.colorInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.colorInfo.stride = realStride;
			D3DState.ClientVertexArrayState.colorInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 2*sizeof(GLfloat);
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.normalInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.normalInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.normalInfo.stride = realStride;
			D3DState.ClientVertexArrayState.normalInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 6*sizeof(GLfloat);
			D3DState.ClientVertexArrayState.normalInfo._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.normalInfo._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.vertexInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.vertexInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.vertexInfo.stride = realStride;
			D3DState.ClientVertexArrayState.vertexInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 9*sizeof(GLfloat);
		}
		break;
	case GL_T4F_C4F_N3F_V4F:
		{
			int realStride = stride;
			if (!realStride) realStride = sizeof(GLfloat) * 14;
			D3DState.ClientVertexArrayState.vertexArrayEnable |= (VA_ENABLE_TEXTURE0_BIT | VA_ENABLE_NORMAL_BIT | VA_ENABLE_COLOR_BIT);
			D3DState.ClientVertexArrayState.texCoordInfo[0].elementCount = 4;
			D3DState.ClientVertexArrayState.texCoordInfo[0].elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.texCoordInfo[0].stride = realStride;
			D3DState.ClientVertexArrayState.texCoordInfo[0].data = reinterpret_cast<const GLubyte*>(pointer);
			D3DState.ClientVertexArrayState.texCoordInfo[0]._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.texCoordInfo[0]._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.colorInfo.elementCount = 4;
			D3DState.ClientVertexArrayState.colorInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.colorInfo.stride = realStride;
			D3DState.ClientVertexArrayState.colorInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 4*sizeof(GLfloat);
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.normalInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.normalInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.normalInfo.stride = realStride;
			D3DState.ClientVertexArrayState.normalInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 8*sizeof(GLfloat);
			D3DState.ClientVertexArrayState.normalInfo._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.normalInfo._internal.compiledLast = -1;
			D3DState.ClientVertexArrayState.vertexInfo.elementCount = 3;
			D3DState.ClientVertexArrayState.vertexInfo.elementType = GL_FLOAT;
			D3DState.ClientVertexArrayState.vertexInfo.stride = realStride;
			D3DState.ClientVertexArrayState.vertexInfo.data = reinterpret_cast<const GLubyte*>(pointer) + 11*sizeof(GLfloat);
		}
		break;
	default:
		logPrintf("WARNING: glInterleavedArrays: unsupported format 0x%x\n", format);
		break;
	}
}

OPENGL_API void WINAPI glArrayElement( GLint i )
{
	assert( D3DGlobal.pIMBuffer != nullptr );
	if (!(D3DState.ClientVertexArrayState.vertexArrayEnable & VA_ENABLE_VERTEX_BIT)) {
		logPrintf("WARNING: glArrayElement: vertex array is disabled\n");
		return;
	}

	if (D3DState.ClientVertexArrayState.vertexArrayEnable & VA_ENABLE_NORMAL_BIT) {
		D3DVA_CopyArrayToFloats( &D3DState.ClientVertexArrayState.normalInfo, i, D3DState.CurrentState.currentNormal );
	}
	if (D3DState.ClientVertexArrayState.vertexArrayEnable & VA_ENABLE_COLOR_BIT) {
		GLubyte color[4] = { 255, 255, 255, 255 };
		D3DVA_CopyArrayToUBytes( &D3DState.ClientVertexArrayState.colorInfo, i, color );
		D3DState.CurrentState.currentColor = D3DCOLOR_ARGB( color[3], color[0], color[1], color[2] );
	}
	if (D3DState.ClientVertexArrayState.vertexArrayEnable & VA_ENABLE_COLOR2_BIT) {
		GLubyte color[4] = { 0, 0, 0, 0 };
		D3DVA_CopyArrayToUBytes( &D3DState.ClientVertexArrayState.color2Info, i, color );
		D3DState.CurrentState.currentColor2 = D3DCOLOR_ARGB( color[3], color[0], color[1], color[2] );
	}
	if (D3DState.ClientVertexArrayState.vertexArrayEnable & VA_ENABLE_FOG_BIT) {
		GLubyte fogCoord;
		D3DVA_CopyArrayToUBytes( &D3DState.ClientVertexArrayState.fogInfo, i, &fogCoord );
		D3DState.CurrentState.currentColor2 &= (fogCoord << 24);
		D3DState.CurrentState.currentColor2 |= (fogCoord << 24);
	}
	for ( int j = 0; j < D3DGlobal.maxActiveTMU; ++j ) {
		DWORD vaTextureBit = 1 << (VA_TEXTURE_BIT_SHIFT + j);
		if (D3DState.ClientVertexArrayState.vertexArrayEnable & vaTextureBit) {
			GLfloat tc[4] = { 0, 0, 0, 1 };
			D3DVA_CopyArrayToFloats( &D3DState.ClientVertexArrayState.texCoordInfo[j], i, tc );
			if (D3DState.TransformState.texcoordFixEnabled) {
				tc[0] += D3DState.TransformState.texcoordFix[0];
				tc[1] += D3DState.TransformState.texcoordFix[1];
			}
			memcpy( D3DState.CurrentState.currentTexCoord[j], tc, sizeof(tc) );
		}
	}

	GLfloat vertex[4] = { 0, 0, 0, 1 };
	D3DVA_CopyArrayToFloats( &D3DState.ClientVertexArrayState.vertexInfo, i, vertex );

	if (D3DState.ClientVertexArrayState.vertexInfo.elementCount == 4) 
		D3DGlobal.pIMBuffer->AddVertex( vertex[0], vertex[1], vertex[2], vertex[3] );
	else
		D3DGlobal.pIMBuffer->AddVertex( vertex[0], vertex[1], vertex[2] );
}

static void internal_DrawArrays( GLenum mode, GLint first, GLsizei count )
{
#if defined(VA_USE_IMMEDIATE_MODE)
	assert( D3DGlobal.pIMBuffer != nullptr );
	D3DGlobal.pIMBuffer->Begin( mode );

	for (int i = 0; i < count; ++i)
		glArrayElement( first + i );

	D3DGlobal.pIMBuffer->End();
#else
	if ( mode == GL_POINTS ) {
		//points are not supported within DIP, so use immediate mode
		assert( D3DGlobal.pIMBuffer != nullptr );
		D3DGlobal.pIMBuffer->Begin( mode );
		for (int i = 0; i < count; ++i) {
			glArrayElement( first + i );
		}
		D3DGlobal.pIMBuffer->End();
	} else {
		//use vertex array mode
		assert( D3DGlobal.pVABuffer != nullptr );
		D3DGlobal.pVABuffer->SetIndices<GLushort>( mode, first, first + count - 1, count, nullptr );
		D3DGlobal.pVABuffer->DrawPrimitive();
		D3DGlobal.pVABuffer->Unlock();
	}
#endif

//	if (D3DState.ClientVertexArrayState.vertexArrayEnable & VA_ENABLE_COLOR_BIT)
//		D3DState.CurrentState.currentColor = 0xFFFFFFFF;
}
static void internal_DrawElements( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type,  const GLvoid *indices )
{
#if defined(VA_USE_IMMEDIATE_MODE)
	assert( D3DGlobal.pIMBuffer != nullptr );
	D3DGlobal.pIMBuffer->Begin( mode );

	switch (type) {
	case GL_UNSIGNED_BYTE:
		{
			const GLubyte *myindices = reinterpret_cast<const GLubyte*>(indices);
			for (int i = 0; i < count; ++i)
				glArrayElement( myindices[i] );
		}
		break;
	case GL_UNSIGNED_SHORT:
		{
			const GLushort *myindices = reinterpret_cast<const GLushort*>(indices);
			for (int i = 0; i < count; ++i)
				glArrayElement( myindices[i] );
		}
		break;
	case GL_UNSIGNED_INT:
		{
			const GLuint *myindices = reinterpret_cast<const GLuint*>(indices);
			for (int i = 0; i < count; ++i)
				glArrayElement( myindices[i] );
		}
		break;
	default:
		logPrintf("WARNING: DrawElements: unsupported index type 0x%x\n", type);
		break;
	}

	D3DGlobal.pIMBuffer->End();
#else
	if ( mode == GL_POINTS ) {
		//points are not supported within DIP, so use immediate mode
		assert( D3DGlobal.pIMBuffer != nullptr );
		D3DGlobal.pIMBuffer->Begin( mode );

		switch (type) {
		case GL_UNSIGNED_BYTE:
			{
				const GLubyte *myindices = reinterpret_cast<const GLubyte*>(indices);
				for (int i = 0; i < count; ++i)
					glArrayElement( myindices[i] );
			}
			break;
		case GL_UNSIGNED_SHORT:
			{
				const GLushort *myindices = reinterpret_cast<const GLushort*>(indices);
				for (int i = 0; i < count; ++i)
					glArrayElement( myindices[i] );
			}
			break;
		case GL_UNSIGNED_INT:
			{
				const GLuint *myindices = reinterpret_cast<const GLuint*>(indices);
				for (int i = 0; i < count; ++i)
					glArrayElement( myindices[i] );
			}
			break;
		default:
			logPrintf("WARNING: DrawElements: unsupported index type 0x%x\n", type);
			break;
		}

		D3DGlobal.pIMBuffer->End();
	} else {
		//use vertex array mode
		assert( D3DGlobal.pVABuffer != nullptr );
		
		switch (type) {
		case GL_UNSIGNED_BYTE:
			D3DGlobal.pVABuffer->SetIndices<GLubyte>( mode, start, end, count, (const GLubyte*)indices );
			break;
		case GL_UNSIGNED_SHORT:
			D3DGlobal.pVABuffer->SetIndices<GLushort>( mode, start, end, count, (const GLushort*)indices );
			break;
		case GL_UNSIGNED_INT:
			D3DGlobal.pVABuffer->SetIndices<GLuint>( mode, start, end, count, (const GLuint*)indices );
			break;
		default:
			logPrintf("WARNING: DrawElements: unsupported index type 0x%x\n", type);
			break;
		}

		D3DGlobal.pVABuffer->DrawPrimitive();
		D3DGlobal.pVABuffer->Unlock();
	}
#endif

//	if (D3DState.ClientVertexArrayState.vertexArrayEnable & VA_ENABLE_COLOR_BIT)
//		D3DState.CurrentState.currentColor = 0xFFFFFFFF;
}

OPENGL_API void WINAPI glDrawArrays( GLenum mode, GLint first, GLsizei count )
{
	D3DState_Check();
	D3DState_AssureBeginScene();
	internal_DrawArrays( mode, first, count );
}
OPENGL_API void WINAPI glDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
	D3DState_Check();
	D3DState_AssureBeginScene();
	internal_DrawElements( mode, ~0u, 0, count, type, indices );
}

OPENGL_API void WINAPI glDrawRangeElements( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices )
{
	D3DState_Check();
	D3DState_AssureBeginScene();
	internal_DrawElements( mode, start, end, count, type, indices );
}

OPENGL_API void WINAPI glMultiDrawArrays( GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount )
{
	D3DState_Check();
	D3DState_AssureBeginScene();
	for (int i = 0; i < primcount; ++i)
		if (count[i] > 0) internal_DrawArrays( mode, first[i], count[i] );
}

OPENGL_API void WINAPI glMultiDrawElements( GLenum mode, GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount )
{
	D3DState_Check();
	D3DState_AssureBeginScene();
	for (int i = 0; i < primcount; ++i)
		if (count[i] > 0) internal_DrawElements( mode, ~0u, 0, count[i], type, indices[i] );
}

template<typename T>
static inline bool CheckCompiledArraySize( T **arr, GLsizei *curSize, GLsizei reqSize )
{
	if (*curSize >= reqSize) 
		return true;

	*curSize = reqSize;

	if (!(*arr))
		*arr = (T*)UTIL_Alloc( reqSize * sizeof(T) );
	else
		*arr = (T*)UTIL_Realloc( *arr, reqSize * sizeof(T) );

	return (*arr != nullptr);
}

OPENGL_API void WINAPI glUnlockArrays( void )
{
	if (D3DState.ClientVertexArrayState.vertexInfo._internal.compiledLast >= 0) {
		D3DState.ClientVertexArrayState.vertexInfo._internal.compiledFirst = 0;
		D3DState.ClientVertexArrayState.vertexInfo._internal.compiledLast = -1;
	}
	if (D3DState.ClientVertexArrayState.normalInfo._internal.compiledLast >= 0) {
		D3DState.ClientVertexArrayState.normalInfo._internal.compiledFirst = 0;
		D3DState.ClientVertexArrayState.normalInfo._internal.compiledLast = -1;
	}
	if (D3DState.ClientVertexArrayState.colorInfo._internal.compiledLast >= 0) {
		D3DState.ClientVertexArrayState.colorInfo._internal.compiledFirst = 0;
		D3DState.ClientVertexArrayState.colorInfo._internal.compiledLast = -1;
	}
	for (int i = 0; i < MAX_D3D_TMU; ++i) {
		if (D3DState.ClientVertexArrayState.texCoordInfo[i]._internal.compiledLast >= 0) {
			D3DState.ClientVertexArrayState.texCoordInfo[i]._internal.compiledFirst = 0;
			D3DState.ClientVertexArrayState.texCoordInfo[i]._internal.compiledLast = -1;
		}
	}
}

OPENGL_API void WINAPI glLockArrays( GLint first, GLsizei count )
{
	//compile vertex, normal, texcoord and color arrays
	//secondary color and fog coord arrays are NOT compiled

	//compile vertex array
	if (D3DState.ClientVertexArrayState.vertexInfo._internal.compiledLast >= 0) {
		if (D3DState.ClientVertexArrayState.vertexArrayEnable & VA_ENABLE_VERTEX_BIT) {
			GLfloat **ppdata = &D3DGlobal.compiledVertexArray.compiledVertexData;
			GLint elemCount;
			if (D3DState.ClientVertexArrayState.vertexInfo.elementCount == 4)
				elemCount = 4;
			else
				elemCount = 3;
	
			if (!CheckCompiledArraySize<GLfloat>( ppdata, &D3DGlobal.compiledVertexArray.compiledVertexDataSize, count * elemCount )) {
				D3DGlobal.lastError = E_OUTOFMEMORY;
				return;
			}

			GLfloat *pdata = *ppdata;
			for (int i = 0; i < count; ++i) {
				D3DVA_CopyArrayToFloats( &D3DState.ClientVertexArrayState.vertexInfo, first + i, pdata );
				if (D3DState.ClientVertexArrayState.vertexInfo.elementCount == 2)
					pdata[2] = 0.0f;
				pdata += elemCount;
			}

			D3DState.ClientVertexArrayState.vertexInfo._internal.compiledFirst = first;
			D3DState.ClientVertexArrayState.vertexInfo._internal.compiledLast = first + count - 1;
		}
	}

	//compile normal array
	if (D3DState.ClientVertexArrayState.normalInfo._internal.compiledLast >= 0) {
		if (D3DState.ClientVertexArrayState.vertexArrayEnable & VA_ENABLE_NORMAL_BIT) {
			GLfloat **ppdata = &D3DGlobal.compiledVertexArray.compiledNormalData;
		
			if (!CheckCompiledArraySize<GLfloat>( ppdata, &D3DGlobal.compiledVertexArray.compiledNormalDataSize, count * 3 )) {
				D3DGlobal.lastError = E_OUTOFMEMORY;
				return;
			}

			GLfloat *pdata = *ppdata;
			for (int i = 0; i < count; ++i) {
				D3DVA_CopyArrayToFloats( &D3DState.ClientVertexArrayState.normalInfo, first + i, pdata );
				pdata += 3;
			}

			D3DState.ClientVertexArrayState.normalInfo._internal.compiledFirst = first;
			D3DState.ClientVertexArrayState.normalInfo._internal.compiledLast = first + count - 1;
		}
	}

	//compile color array
	if (D3DState.ClientVertexArrayState.colorInfo._internal.compiledLast >= 0) {
		if (D3DState.ClientVertexArrayState.vertexArrayEnable & VA_ENABLE_COLOR_BIT) {
			DWORD **ppdata = &D3DGlobal.compiledVertexArray.compiledColorData;
		
			if (!CheckCompiledArraySize<DWORD>( ppdata, &D3DGlobal.compiledVertexArray.compiledColorDataSize, count )) {
				D3DGlobal.lastError = E_OUTOFMEMORY;
				return;
			}

			DWORD *pdata = *ppdata;
			for (int i = 0; i < count; ++i) {
				GLubyte color[4] = { 255, 255, 255, 255 };
				D3DVA_CopyArrayToUBytes( &D3DState.ClientVertexArrayState.colorInfo, first + i, color );
				*pdata = D3DCOLOR_ARGB( color[3], color[0], color[1], color[2] );
				++pdata;
			}

			D3DState.ClientVertexArrayState.colorInfo._internal.compiledFirst = first;
			D3DState.ClientVertexArrayState.colorInfo._internal.compiledLast = first + count - 1;
		}
	}

	//compile texcoord arrays
	for ( int j = 0; j < D3DGlobal.maxActiveTMU; ++j ) {
		if (D3DState.ClientVertexArrayState.texCoordInfo[j]._internal.compiledLast >= 0) {
			DWORD vaTextureBit = 1 << (VA_TEXTURE_BIT_SHIFT + j);
			if (D3DState.ClientVertexArrayState.vertexArrayEnable & vaTextureBit) {
				GLfloat **ppdata = &D3DGlobal.compiledVertexArray.compiledTexCoordData[j];
		
				if (!CheckCompiledArraySize<GLfloat>( ppdata, &D3DGlobal.compiledVertexArray.compiledTexCoordDataSize[j], count * 4 )) {
					D3DGlobal.lastError = E_OUTOFMEMORY;
					return;
				}

				GLfloat *pdata = *ppdata;
				for (int i = 0; i < count; ++i) {
					GLfloat tc[4] = { 0, 0, 0, 1 };
					D3DVA_CopyArrayToFloats( &D3DState.ClientVertexArrayState.texCoordInfo[j], first + i, tc );
					if (D3DState.TransformState.texcoordFixEnabled) {
						tc[0] += D3DState.TransformState.texcoordFix[0];
						tc[1] += D3DState.TransformState.texcoordFix[1];
					}
					memcpy( pdata, tc, sizeof(tc) );
					pdata += 4;
				}

				D3DState.ClientVertexArrayState.texCoordInfo[j]._internal.compiledFirst = first;
				D3DState.ClientVertexArrayState.texCoordInfo[j]._internal.compiledLast = first + count - 1;
			}
		}
	}
}