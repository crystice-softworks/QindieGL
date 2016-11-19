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
#ifndef QINDIEGL_D3D_TEXTURE_H
#define QINDIEGL_D3D_TEXTURE_H

class D3DTextureObject
{
public:
	explicit D3DTextureObject( GLuint gl_index );
	~D3DTextureObject();

	void FreeD3DTexture();
	HRESULT CreateD3DTexture( GLenum target, GLsizei width, GLsizei height, GLsizei depth, GLboolean border, D3DFORMAT format, GLboolean mipmaps );
	HRESULT RecreateD3DTexture( GLboolean mipmaps );
	HRESULT FillTextureLevel( GLint cubeface, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels );
	HRESULT FillTextureSubLevel( GLint cubeface, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels );
	HRESULT CopyTextureSubLevel( GLint cubeface, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height );
	HRESULT FillCompressedTextureLevel( GLint cubeface, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLsizei imageSize, const GLvoid *pixels );
	HRESULT GetTexImage( GLint cubeface, GLint level, GLenum format,  GLenum type,  GLvoid *pixels );
	HRESULT DumpTexture();
	void CheckMipmapAutogen();

	LPDIRECT3DBASETEXTURE9 GetD3DTexture() const { return m_pD3DBaseTexture; }
	GLenum GetTarget() const { return m_target; }
	D3DTEXTUREADDRESS GetD3DAddressMode( int index ) const
	{
		assert( index >= 0 && index < 3 );
		return m_d3dAddressMode[index];
	}
	D3DTEXTUREFILTERTYPE GetD3DFilter( int index ) const
	{
		assert( index >= 0 && index < 3 );
		return ((index < 2 && m_anisotropy > 1) ? D3DTEXF_ANISOTROPIC : m_d3dFilter[index]);
	}
	D3DCOLOR GetD3DBorderColor() const
	{
		return m_borderColor;
	}
	GLenum GetAddressMode( int coord ) const { return m_glAddressMode[coord]; }
	GLenum GetFilter( int index ) const { return m_glFilter[index]; }
	GLboolean GetMipmapAutogen() const { return m_autogenMipmaps; }
	D3DCOLOR GetBorderColor() const { return (m_border ? m_borderColor : 0); }
	GLuint GetAnisotropy() const { return m_anisotropy; }
	DWORD GetPriority() const { return m_priority; }
	GLuint GetWidth() const { return m_width; }
	GLuint GetHeight() const { return m_height; }
	GLuint GetDepth() const { return m_depth; }
	GLfloat GetLodBias() const { return m_lodBias; }
	GLuint GetGLIndex() const { return m_glIndex; }
	eTexTypeInternal GetInternalFormat() const { return m_internalFormat; }

	void SetAddressMode( GLenum coord, GLenum mode );
	void SetMagFilter( GLenum mode );
	void SetMinFilter( GLenum mode );
	void SetTarget( GLenum target ) { m_target = target; }
	void SetMipmapAutogen( GLboolean value ) { m_autogenMipmaps = value; }
	void SetBorderColor( D3DCOLOR value ) { m_borderColor = value; }
	void SetAnisotropy( GLuint value ) { m_anisotropy = QINDIEGL_MIN( D3DGlobal.hD3DCaps.MaxAnisotropy, QINDIEGL_MAX( 1, value ) ); }
	void SetPriority( DWORD value ) { m_priority = value; }
	void SetLodBias( GLfloat value ) { m_lodBias = value; }

private:
	union {
		LPDIRECT3DBASETEXTURE9 		m_pD3DBaseTexture;
		LPDIRECT3DTEXTURE9			m_pD3DTexture;
		LPDIRECT3DVOLUMETEXTURE9	m_pD3DVolumeTexture;
		LPDIRECT3DCUBETEXTURE9		m_pD3DCubeTexture;
	};
	D3DTEXTUREADDRESS		m_d3dAddressMode[3];
	D3DTEXTUREFILTERTYPE	m_d3dFilter[3];
	GLenum					m_glAddressMode[3];
	GLenum					m_glFilter[2];
	GLenum					m_target;
	GLuint					m_glIndex;
	GLuint					m_anisotropy;
	GLboolean				m_autogenMipmaps;
	GLboolean				m_border;
	GLboolean				m_mipmaps;
	GLsizei					m_width, m_height, m_depth;
	GLfloat					m_lodBias;
	D3DFORMAT				m_format;
	eTexTypeInternal		m_internalFormat;
	int						m_dstbytes;
	D3DCOLOR				m_borderColor;
	DWORD					m_priority;
};

#endif //QINDIEGL_D3D_TEXTURE_H