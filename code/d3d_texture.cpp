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
#include "d3d_pixels.hpp"

//==================================================================================
// Texturing
//----------------------------------------------------------------------------------
// TODO: implement EYE_PLANE and OBJECT_PLANE 
//		 as D3D transform matrices (see 
//		"Special effects" topic)
// TODO: S3TC-compressed textures
//==================================================================================

D3DTextureObject :: D3DTextureObject( GLuint gl_index )
{
	m_pD3DTexture = nullptr;
	m_d3dAddressMode[0] = D3DTADDRESS_WRAP;
	m_d3dAddressMode[1] = D3DTADDRESS_WRAP;
	m_d3dAddressMode[2] = D3DTADDRESS_WRAP;
	m_glAddressMode[0] = GL_REPEAT;
	m_glAddressMode[1] = GL_REPEAT;
	m_glAddressMode[2] = GL_REPEAT;
	m_d3dFilter[0] = D3DTEXF_LINEAR;
	m_d3dFilter[1] = D3DTEXF_POINT;
	m_d3dFilter[2] = D3DTEXF_LINEAR;
	m_glFilter[0] = GL_LINEAR;
	m_glFilter[1] = GL_NEAREST_MIPMAP_LINEAR;
	m_border = GL_FALSE;
	m_autogenMipmaps = GL_FALSE;
	m_mipmaps = GL_FALSE;
	m_width = 0;
	m_height = 0;
	m_format = D3DFMT_UNKNOWN;
	m_internalFormat = D3D_TEXTYPE_GENERIC;
	m_borderColor = 0;
	m_anisotropy = 1;
	m_priority = 0;
	m_lodBias = 0;
	m_glIndex = gl_index;
}

D3DTextureObject :: ~D3DTextureObject()
{
	FreeD3DTexture();
}

void D3DTextureObject :: FreeD3DTexture()
{
	if (m_pD3DTexture) {
		if (m_target == GL_TEXTURE_3D_EXT) {
			logPrintf("FreeD3DTexture: %i x %i x %i x %s\n", m_width, m_height, m_depth, D3DGlobal_FormatToString(m_format) );
		} else {
			logPrintf("FreeD3DTexture: %i x %i x %s\n", m_width, m_height, D3DGlobal_FormatToString(m_format) );
		}
		m_pD3DTexture->Release();
		m_pD3DTexture = nullptr;
	}
}

HRESULT D3DTextureObject :: CreateD3DTexture( GLenum target, GLsizei width, GLsizei height, GLsizei depth, GLboolean border, D3DFORMAT format, GLboolean mipmaps )
{
	if (m_pD3DTexture) 
		return S_OK;

	if (target == GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB ||
		target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB ||
		target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB ||
		target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB ||
		target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB ||
		target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB) {
			target = GL_TEXTURE_CUBE_MAP_ARB;
			if (width != height) {
				logPrintf("ERROR: Cubemap must be square! (%i x %i)\n", width, height);
				return E_INVALIDARG;
			}
	}

	m_mipmaps = mipmaps;
	m_width = width;
	m_height = height;
	m_depth = depth;
	m_format = format;
	m_target = target;
	m_border = border;
	m_internalFormat = D3D_TEXTYPE_GENERIC;
	m_dstbytes = 0;

	if (m_autogenMipmaps)
		mipmaps = GL_TRUE;

	HRESULT hr;
		
	if (target == GL_TEXTURE_3D_EXT) {
		logPrintf("CreateD3DTexture: %i x %i x %i x %s (mipmaps = %s)\n", width, height, depth, D3DGlobal_FormatToString(m_format), mipmaps ? "true" : "false" );
		hr = D3DGlobal.pDevice->CreateVolumeTexture(width, height, depth, mipmaps ? 0 : 1, 0, format, D3DPOOL_MANAGED, &m_pD3DVolumeTexture, NULL);
		if (m_pD3DVolumeTexture) m_pD3DVolumeTexture->SetPriority( m_priority );
	} else if (target == GL_TEXTURE_CUBE_MAP_ARB) {
		logPrintf("CreateD3DTexture: %i x %i x %s (cubemap) (mipmaps = %s)\n", width, height, D3DGlobal_FormatToString(m_format), mipmaps ? "true" : "false" );
		hr = D3DGlobal.pDevice->CreateCubeTexture(width, mipmaps ? 0 : 1, 0, format, D3DPOOL_MANAGED, &m_pD3DCubeTexture, NULL);
		if (m_pD3DCubeTexture) m_pD3DCubeTexture->SetPriority( m_priority );
	} else {
		logPrintf("CreateD3DTexture: %i x %i x %s (mipmaps = %s)\n", width, height, D3DGlobal_FormatToString(m_format), mipmaps ? "true" : "false" );
		hr = D3DGlobal.pDevice->CreateTexture(width, height, mipmaps ? 0 : 1, 0, format, D3DPOOL_MANAGED, &m_pD3DTexture, NULL);
		if (m_pD3DTexture) m_pD3DTexture->SetPriority( m_priority );
	}

	return hr;
}

HRESULT D3DTextureObject :: RecreateD3DTexture( GLboolean mipmaps )
{
	if (!m_pD3DTexture) return E_FAIL;
	if (m_mipmaps == mipmaps) return S_OK;

	if (m_target == GL_TEXTURE_3D_EXT) {
		logPrintf("RecreateD3DTexture: %i x %i x %i x %s (mipmaps = %s)\n", m_width, m_height, m_depth, D3DGlobal_FormatToString(m_format), mipmaps ? "true" : "false" );
	} else {
		logPrintf("RecreateD3DTexture: %i x %i x %s (mipmaps = %s)\n", m_width, m_height, D3DGlobal_FormatToString(m_format), mipmaps ? "true" : "false" );
	}

	HRESULT hr;

	if (m_target == GL_TEXTURE_3D_EXT)
	{
		LPDIRECT3DVOLUMETEXTURE9 newTexture;
		hr = D3DGlobal.pDevice->CreateVolumeTexture(m_width, m_height, m_depth, mipmaps ? 0 : 1, 0, m_format, D3DPOOL_MANAGED, &newTexture, NULL);
		if (FAILED(hr)) return hr;	

		LPDIRECT3DVOLUME9 srcsurf, dstsurf;
		hr = m_pD3DVolumeTexture->GetVolumeLevel( 0, &srcsurf );
		if (FAILED(hr)) return hr;	
		hr = newTexture->GetVolumeLevel( 0, &dstsurf );
		if (FAILED(hr)) return hr;	

		hr = D3DXLoadVolumeFromVolume(dstsurf, NULL, NULL, srcsurf, NULL, NULL, D3DX_FILTER_NONE, 0);
		if (FAILED(hr)) return hr;	

		srcsurf->Release();
		dstsurf->Release();

		m_pD3DVolumeTexture->Release();
		m_pD3DVolumeTexture = newTexture;
		if (m_pD3DVolumeTexture) m_pD3DVolumeTexture->SetPriority( m_priority );
	}
	else if (m_target == GL_TEXTURE_CUBE_MAP_ARB)
	{
		LPDIRECT3DCUBETEXTURE9 newTexture;
		hr = D3DGlobal.pDevice->CreateCubeTexture(m_width, mipmaps ? 0 : 1, 0, m_format, D3DPOOL_MANAGED, &newTexture, NULL);
		if (FAILED(hr)) return hr;	

		LPDIRECT3DSURFACE9 srcsurf, dstsurf;
		for (int i = 0; i < 6; ++i) {
			hr = m_pD3DCubeTexture->GetCubeMapSurface( (D3DCUBEMAP_FACES)i, 0, &srcsurf );
			if (FAILED(hr)) return hr;	
			hr = newTexture->GetCubeMapSurface( (D3DCUBEMAP_FACES)i, 0, &dstsurf );
			if (FAILED(hr)) return hr;	

			hr = D3DXLoadSurfaceFromSurface(dstsurf, NULL, NULL, srcsurf, NULL, NULL, D3DX_FILTER_NONE, 0);
			if (FAILED(hr)) return hr;	

			srcsurf->Release();
			dstsurf->Release();
		}

		m_pD3DCubeTexture->Release();
		m_pD3DCubeTexture = newTexture;
		if (m_pD3DCubeTexture) m_pD3DCubeTexture->SetPriority( m_priority );
	}
	else
	{
		LPDIRECT3DTEXTURE9 newTexture;
		hr = D3DGlobal.pDevice->CreateTexture(m_width, m_height, mipmaps ? 0 : 1, 0, m_format, D3DPOOL_MANAGED, &newTexture, NULL);
		if (FAILED(hr)) return hr;	

		LPDIRECT3DSURFACE9 srcsurf, dstsurf;
		hr = m_pD3DTexture->GetSurfaceLevel( 0, &srcsurf );
		if (FAILED(hr)) return hr;	
		hr = newTexture->GetSurfaceLevel( 0, &dstsurf );
		if (FAILED(hr)) return hr;	

		hr = D3DXLoadSurfaceFromSurface(dstsurf, NULL, NULL, srcsurf, NULL, NULL, D3DX_FILTER_NONE, 0);
		if (FAILED(hr)) return hr;	
		
		srcsurf->Release();
		dstsurf->Release();

		m_pD3DTexture->Release();
		m_pD3DTexture = newTexture;
		if (m_pD3DTexture) m_pD3DTexture->SetPriority( m_priority );
	}


	m_mipmaps = mipmaps;
	return hr;
}

HRESULT D3DTextureObject :: FillTextureLevel( GLint cubeface, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels )
{
	switch (internalformat) {
	case GL_INTENSITY:
	case GL_INTENSITY8:
		m_internalFormat = D3D_TEXTYPE_INTENSITY;
		m_dstbytes = 2;
		break;		
	case 1:
	case GL_LUMINANCE:
	case GL_LUMINANCE8:
		m_dstbytes = 1;
		break;		
	case 2:
	case GL_LUMINANCE_ALPHA:
	case GL_LUMINANCE8_ALPHA8:
		m_dstbytes = 2;
		break;
	case GL_RGB5:
		m_internalFormat = D3D_TEXTYPE_X1R5G5B5;
		m_dstbytes = 2;
		break;
	case GL_RGB5_A1:
		m_internalFormat = D3D_TEXTYPE_A1R5G5B5;
		m_dstbytes = 2;
		break;
	case GL_RGBA4:
		m_internalFormat = D3D_TEXTYPE_A4R4G4B4;
		m_dstbytes = 2;
		break;
	case GL_ALPHA:
	case GL_ALPHA8:
		m_internalFormat = D3D_TEXTYPE_X24A8;
	case 3:
	case GL_RGB:
	case GL_RGB8:
	case GL_BGR_EXT:
	case 4:
	case GL_RGBA:
	case GL_RGBA8:
	case GL_BGRA_EXT:
	case GL_ABGR_EXT:
		m_dstbytes = 4;
		break;
	default:
		logPrintf("WARNING: Texture internal format 0x%x is not supported\n", internalformat);
		return E_INVALIDARG;
	}

	HRESULT hr;
	GLubyte *dstdata;
	GLint pitch;
	GLint pitch2;

	if (!pixels)
		return S_OK;

	if (m_target == GL_TEXTURE_3D_EXT) {
		D3DLOCKED_BOX lockrect;
		hr = m_pD3DVolumeTexture->LockBox( level, &lockrect, NULL, 0 );
		if (FAILED(hr)) return hr;

		dstdata = (GLubyte*)lockrect.pBits;
		pitch = lockrect.RowPitch;
		pitch2 = lockrect.SlicePitch;
	} else if (m_target == GL_TEXTURE_CUBE_MAP_ARB) {
		D3DLOCKED_RECT lockrect;
		hr = m_pD3DCubeTexture->LockRect( (D3DCUBEMAP_FACES)cubeface, level, &lockrect, NULL, 0 );
		if (FAILED(hr)) return hr;

		dstdata = (GLubyte*)lockrect.pBits;
		pitch = lockrect.Pitch;
		pitch2 = 0;
	} else {
		D3DLOCKED_RECT lockrect;
		hr = m_pD3DTexture->LockRect( level, &lockrect, NULL, 0 );
		if (FAILED(hr)) return hr;

		dstdata = (GLubyte*)lockrect.pBits;
		pitch = lockrect.Pitch;
		pitch2 = 0;
	}

	hr = D3DPixels_Unpack( width, height, depth, pitch, pitch2, dstdata, m_dstbytes, m_internalFormat, false, format, type, pixels );
	if (FAILED(hr)) {
		if (m_target == GL_TEXTURE_3D_EXT) {	
			m_pD3DVolumeTexture->UnlockBox( level );
		} else if (m_target == GL_TEXTURE_CUBE_MAP_ARB) {
			m_pD3DCubeTexture->UnlockRect( (D3DCUBEMAP_FACES)cubeface, level );
		} else {
			m_pD3DTexture->UnlockRect( level );
		}
		return hr;
	}

	if (m_target == GL_TEXTURE_3D_EXT) {
		hr = m_pD3DVolumeTexture->UnlockBox( level );
	} else if (m_target == GL_TEXTURE_CUBE_MAP_ARB) {
		hr = m_pD3DCubeTexture->UnlockRect( (D3DCUBEMAP_FACES)cubeface, level );
	} else {
		hr = m_pD3DTexture->UnlockRect( level );
	}

	return hr;
}

HRESULT D3DTextureObject :: FillTextureSubLevel( GLint cubeface, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels )
{
	HRESULT hr;
	GLubyte *dstdata;
	GLint pitch;
	GLint pitch2;

	if (m_target == GL_TEXTURE_3D_EXT) {
		D3DBOX updaterect;
		D3DLOCKED_BOX lockrect;

		updaterect.Left = xoffset;
		updaterect.Right = xoffset + width;
		updaterect.Top = yoffset;
		updaterect.Bottom = yoffset + height;
		updaterect.Front = zoffset;
		updaterect.Back = zoffset + depth;

		hr = m_pD3DVolumeTexture->LockBox( level, &lockrect, &updaterect, 0 );
		if (FAILED(hr)) return hr;

		dstdata = (GLubyte*)lockrect.pBits;
		pitch = lockrect.RowPitch;
		pitch2 = lockrect.SlicePitch;
	} else if (m_target == GL_TEXTURE_CUBE_MAP_ARB) {
		RECT updaterect;
		D3DLOCKED_RECT lockrect;

		updaterect.left = xoffset;
		updaterect.right = xoffset + width;
		updaterect.top = yoffset;
		updaterect.bottom = yoffset + height;

		hr = m_pD3DCubeTexture->LockRect( (D3DCUBEMAP_FACES)cubeface, level, &lockrect, &updaterect, 0 );
		if (FAILED(hr)) return hr;

		dstdata = (GLubyte*)lockrect.pBits;
		pitch = lockrect.Pitch;
		pitch2 = 0;
	} else {
		RECT updaterect;
		D3DLOCKED_RECT lockrect;

		updaterect.left = xoffset;
		updaterect.right = xoffset + width;
		updaterect.top = yoffset;
		updaterect.bottom = yoffset + height;

		hr = m_pD3DTexture->LockRect( level, &lockrect, &updaterect, 0 );
		if (FAILED(hr)) return hr;

		dstdata = (GLubyte*)lockrect.pBits;
		pitch = lockrect.Pitch;
		pitch2 = 0;
	}

	hr = D3DPixels_Unpack( width, height, depth, pitch, pitch2, dstdata, m_dstbytes, m_internalFormat, false, format, type, pixels );
	if (FAILED(hr)) {
		if (m_target == GL_TEXTURE_3D_EXT) {	
			m_pD3DVolumeTexture->UnlockBox( level );
		} else if (m_target == GL_TEXTURE_CUBE_MAP_ARB) {
			m_pD3DCubeTexture->UnlockRect( (D3DCUBEMAP_FACES)cubeface, level );
		} else {
			m_pD3DTexture->UnlockRect( level );
		}
		return hr;
	}

	if (m_target == GL_TEXTURE_3D_EXT) {	
		hr = m_pD3DVolumeTexture->UnlockBox( level );
	} else if (m_target == GL_TEXTURE_CUBE_MAP_ARB) {
		hr = m_pD3DCubeTexture->UnlockRect( (D3DCUBEMAP_FACES)cubeface, level );
	} else {
		hr = m_pD3DTexture->UnlockRect( level );
	}
	return hr;
}

HRESULT D3DTextureObject :: CopyTextureSubLevel( GLint cubeface, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height )
{
	HRESULT hr;
	LPDIRECT3DSURFACE9 lpRenderTarget( nullptr );

	if (m_target == GL_TEXTURE_3D_EXT) {
		logPrintf("WARNING: CopyTextureSubLevel is not supported for 3D textures\n");
		return E_FAIL;
	}

	hr = D3DGlobal.pDevice->GetRenderTarget( 0, &lpRenderTarget );
	if (FAILED(hr)) {
		D3DGlobal.lastError = hr;
		logPrintf("WARNING: CopyTextureSubLevel: GetRenderTarget failed with error '%s'\n", DXGetErrorString9(hr));
		return hr;
	} 

	if (!D3DGlobal.pSystemMemRT) 
	{
		D3DSURFACE_DESC desc;

		// because we don't have a lockable backbuffer we instead copy it off to an image surface
		// this will also handle translation between different backbuffer formats
		hr = lpRenderTarget->GetDesc(&desc);
		if (FAILED(hr)) {
			D3DGlobal.lastError = hr;
			logPrintf("WARNING: CopyTextureSubLevel: GetDesc failed with error '%s'\n", DXGetErrorString9(hr));
			lpRenderTarget->Release();
			return hr;
		}

		//create offscreen surface that will hold an antialiased screen image
		hr = D3DGlobal.pDevice->CreateOffscreenPlainSurface( desc.Width, 
															 desc.Height, 
															 desc.Format, 
															 D3DPOOL_SYSTEMMEM, 
															 &D3DGlobal.pSystemMemRT, NULL );
		if (FAILED(hr)) {
			D3DGlobal.lastError = hr;
			logPrintf("WARNING: CopyTextureSubLevel: CreateOffscreenPlainSurface failed with error '%s'\n", DXGetErrorString9(hr));
			lpRenderTarget->Release();
			return hr;
		}

		if (desc.MultiSampleType != D3DMULTISAMPLE_NONE) {
			if (!D3DGlobal.pSystemMemFB) {
				hr = D3DGlobal.pDevice->CreateRenderTarget( desc.Width, 
															desc.Height, 
															desc.Format, 
															D3DMULTISAMPLE_NONE, 
															0, FALSE, 
															&D3DGlobal.pSystemMemFB, NULL );
				if (FAILED(hr)) {
					D3DGlobal.lastError = hr;
					logPrintf("WARNING: CopyTextureSubLevel: CreateRenderTarget failed with error '%s'\n", DXGetErrorString9(hr));
					lpRenderTarget->Release();
					return hr;
				}
			}
		}
	}

	if (D3DGlobal.pSystemMemFB) {
		hr = D3DGlobal.pDevice->StretchRect( lpRenderTarget, NULL, D3DGlobal.pSystemMemFB, NULL, D3DTEXF_NONE );
		if (FAILED(hr)) {
			D3DGlobal.lastError = hr;
			logPrintf("WARNING: CopyTextureSubLevel: StretchRect failed with error '%s'\n", DXGetErrorString9(hr));
			lpRenderTarget->Release();
			return hr;
		}
		lpRenderTarget->Release();
		lpRenderTarget = D3DGlobal.pSystemMemFB;
	}

	hr = D3DGlobal.pDevice->GetRenderTargetData( lpRenderTarget, D3DGlobal.pSystemMemRT );
	if (FAILED(hr)) {
		D3DGlobal.lastError = hr;
		logPrintf("WARNING: CopyTextureSubLevel: GetRenderTargetData failed with error '%s'\n", DXGetErrorString9(hr));
		if (lpRenderTarget != D3DGlobal.pSystemMemFB)
			lpRenderTarget->Release();
		return hr;
	}

	RECT srcrect;
	D3DLOCKED_RECT srclockrect;
	RECT dstrect;
	D3DLOCKED_RECT dstlockrect;

	y = D3DGlobal.hCurrentMode.Height - (height + y);
	srcrect.left = x;
	srcrect.right = x + width;
	srcrect.top = y;
	srcrect.bottom = y + height;
	dstrect.left = xoffset;
	dstrect.right = xoffset + width;
	dstrect.top = yoffset;
	dstrect.bottom = yoffset + height;

	hr = D3DGlobal.pSystemMemRT->LockRect( &srclockrect, &srcrect, D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY );
	if (FAILED(hr)) {
		D3DGlobal.lastError = hr;
		logPrintf("WARNING: CopyTextureSubLevel: LockRect #1 failed with error '%s'\n", DXGetErrorString9(hr));
		if (lpRenderTarget != D3DGlobal.pSystemMemFB)
			lpRenderTarget->Release();
		return hr;
	}

	if (m_target == GL_TEXTURE_CUBE_MAP_ARB) {
		hr = m_pD3DCubeTexture->LockRect( (D3DCUBEMAP_FACES)cubeface, level, &dstlockrect, &dstrect, D3DLOCK_DISCARD );
		if (FAILED(hr)) {
			D3DGlobal.lastError = hr;
			logPrintf("WARNING: CopyTextureSubLevel: LockRect #2 failed with error '%s'\n", DXGetErrorString9(hr));
			if (lpRenderTarget != D3DGlobal.pSystemMemFB)
				lpRenderTarget->Release();
			return hr;
		}
	} else {
		hr = m_pD3DTexture->LockRect( level, &dstlockrect, &dstrect, D3DLOCK_DISCARD );
		if (FAILED(hr)) {
			D3DGlobal.lastError = hr;
			logPrintf("WARNING: CopyTextureSubLevel: LockRect #2 failed with error '%s'\n", DXGetErrorString9(hr));
			if (lpRenderTarget != D3DGlobal.pSystemMemFB)
				lpRenderTarget->Release();
			return hr;
		}
	}

	//copy pixels flipping vertical
	for (int i = 0; i < height; ++i) {
		memcpy( (byte*)dstlockrect.pBits + i*dstlockrect.Pitch, (byte*)srclockrect.pBits + (height-1-i)*srclockrect.Pitch, width * 4 );
	}
	
	D3DGlobal.pSystemMemRT->UnlockRect();

	if (lpRenderTarget != D3DGlobal.pSystemMemFB)
		lpRenderTarget->Release();

	if (m_target == GL_TEXTURE_CUBE_MAP_ARB) {
		hr = m_pD3DCubeTexture->UnlockRect( (D3DCUBEMAP_FACES)cubeface, level );
	} else {
		hr = m_pD3DTexture->UnlockRect( level );
	}	

	return hr;
}

HRESULT D3DTextureObject :: FillCompressedTextureLevel( GLint cubeface, GLint level, GLint /*internalformat*/, GLsizei /*width*/, GLsizei /*height*/, GLsizei /*depth*/, GLsizei imageSize, const GLvoid *pixels )
{
	HRESULT hr;
	GLubyte *dstdata;
	GLint pitch;
	GLint pitch2;

	if (!pixels)
		return S_OK;

	if (m_target == GL_TEXTURE_3D_EXT) {
		D3DLOCKED_BOX lockrect;
		hr = m_pD3DVolumeTexture->LockBox( level, &lockrect, nullptr, 0 );
		if (FAILED(hr)) return hr;

		dstdata = (GLubyte*)lockrect.pBits;
		pitch = lockrect.RowPitch;
		pitch2 = lockrect.SlicePitch;
	} else if (m_target == GL_TEXTURE_CUBE_MAP_ARB) {
		D3DLOCKED_RECT lockrect;
		hr = m_pD3DCubeTexture->LockRect( (D3DCUBEMAP_FACES)cubeface, level, &lockrect, nullptr, 0 );
		if (FAILED(hr)) return hr;

		dstdata = (GLubyte*)lockrect.pBits;
		pitch = lockrect.Pitch;
		pitch2 = 0;
	} else {
		D3DLOCKED_RECT lockrect;
		hr = m_pD3DTexture->LockRect( level, &lockrect, nullptr, 0 );
		if (FAILED(hr)) return hr;

		dstdata = (GLubyte*)lockrect.pBits;
		pitch = lockrect.Pitch;
		pitch2 = 0;
	}

	memcpy( dstdata, pixels, imageSize );

	if (m_target == GL_TEXTURE_3D_EXT) {
		hr = m_pD3DVolumeTexture->UnlockBox( level );
	} else if (m_target == GL_TEXTURE_CUBE_MAP_ARB) {
		hr = m_pD3DCubeTexture->UnlockRect( (D3DCUBEMAP_FACES)cubeface, level );
	} else {
		hr = m_pD3DTexture->UnlockRect( level );
	}

	return hr;
}

HRESULT D3DTextureObject :: GetTexImage( GLint cubeface, GLint level, GLenum format, GLenum type, GLvoid *pixels )
{
	HRESULT hr;
	GLubyte *srcdata;
	GLint pitch;
	GLint pitch2;

	if (level > 0 && !m_mipmaps) 
		return E_INVALIDARG;

	if (m_target == GL_TEXTURE_3D_EXT) {
		D3DLOCKED_BOX lockrect;
		hr = m_pD3DVolumeTexture->LockBox( level, &lockrect, nullptr, D3DLOCK_READONLY );
		if (FAILED(hr)) return hr;

		srcdata = (GLubyte*)lockrect.pBits;
		pitch = lockrect.RowPitch;
		pitch2 = lockrect.SlicePitch;
	} else if (m_target == GL_TEXTURE_CUBE_MAP_ARB) {
		D3DLOCKED_RECT lockrect;
		hr = m_pD3DCubeTexture->LockRect( (D3DCUBEMAP_FACES)cubeface, level, &lockrect, nullptr, D3DLOCK_READONLY );
		if (FAILED(hr)) return hr;

		srcdata = (GLubyte*)lockrect.pBits;
		pitch = lockrect.Pitch;
		pitch2 = 0;
	} else {
		D3DLOCKED_RECT lockrect;
		hr = m_pD3DTexture->LockRect( level, &lockrect, nullptr, D3DLOCK_READONLY );
		if (FAILED(hr)) return hr;

		srcdata = (GLubyte*)lockrect.pBits;
		pitch = lockrect.Pitch;
		pitch2 = 0;
	}

	hr = D3DPixels_Pack( m_width, m_height, m_depth, pitch, pitch2, srcdata, m_dstbytes, m_internalFormat, false, format, type, pixels );
	if (FAILED(hr)) {
		if (m_target == GL_TEXTURE_3D_EXT) {	
			m_pD3DVolumeTexture->UnlockBox( level );
		} else if (m_target == GL_TEXTURE_CUBE_MAP_ARB) {
			m_pD3DCubeTexture->UnlockRect( (D3DCUBEMAP_FACES)cubeface, level );
		} else {
			m_pD3DTexture->UnlockRect( level );
		}
		return hr;
	}

	if (m_target == GL_TEXTURE_3D_EXT) {
		hr = m_pD3DVolumeTexture->UnlockBox( level );
	} else if (m_target == GL_TEXTURE_CUBE_MAP_ARB) {
		hr = m_pD3DCubeTexture->UnlockRect( (D3DCUBEMAP_FACES)cubeface, level );
	} else {
		hr = m_pD3DTexture->UnlockRect( level );
	}

	return hr;
}

HRESULT D3DTextureObject :: DumpTexture()
{
	HRESULT hr;
	GLubyte *srcdata;
	GLint pitch;
	GLint pitch2;
	static GLubyte dumpBuffer[18+1024*1024*4];
	static int dumpCounter = 0;

	if (!m_pD3DTexture)
		return E_FAIL;

	if (m_width > 1024 || m_height > 1024)
		return E_OUTOFMEMORY;

	memset(dumpBuffer, 0, 18);
	dumpBuffer[2] = 2;
	dumpBuffer[12] = static_cast<GLubyte>( m_width & 255 );
	dumpBuffer[13] = static_cast<GLubyte>( m_width >> 8 );
	dumpBuffer[14] = static_cast<GLubyte>( m_height & 255 );
	dumpBuffer[15] = static_cast<GLubyte>( m_height >> 8 );
	dumpBuffer[16] = 32;

	if (m_target == GL_TEXTURE_3D_EXT) {
		//Cannot dump 3D image
		return S_OK;
	} else if (m_target == GL_TEXTURE_CUBE_MAP_ARB) {
		//Cannot dump cubemap
		return S_OK;
	} else {
		D3DLOCKED_RECT lockrect;
		hr = m_pD3DTexture->LockRect( 0, &lockrect, nullptr, D3DLOCK_READONLY );
		if (FAILED(hr)) return hr;

		srcdata = (GLubyte*)lockrect.pBits;
		pitch = lockrect.Pitch;
		pitch2 = 0;
	}

	hr = D3DPixels_Pack( m_width, m_height, m_depth, pitch, pitch2, srcdata, m_dstbytes, m_internalFormat, true, GL_BGRA, GL_UNSIGNED_BYTE, dumpBuffer+18 );
	if (FAILED(hr)) {
		m_pD3DTexture->UnlockRect( 0 );
		return hr;
	}

	char filename[MAX_PATH];
	_mkdir("dump");
	sprintf_s(filename, "dump\\texture%i.tga", dumpCounter);

	FILE *fp( nullptr );
	if ( fopen_s( &fp, filename, "wb") )
		return E_ACCESSDENIED;

	++dumpCounter;
	
	fwrite( dumpBuffer, 1, m_height*m_width*4+18 , fp );
	fclose( fp );

	hr = m_pD3DTexture->UnlockRect( 0 );
	return hr;
}

void D3DTextureObject :: CheckMipmapAutogen()
{
	if (m_autogenMipmaps)
		m_pD3DBaseTexture->GenerateMipSubLevels();
}

void D3DTextureObject :: SetAddressMode( GLenum coord, GLenum mode ) 
{
	int realCoord = UTIL_GLtoD3DAddressIndex( coord );
	assert( realCoord >= 0 && realCoord < 3 );
	m_glAddressMode[realCoord] = mode;
	m_d3dAddressMode[realCoord] = UTIL_GLtoD3DAddressMode(mode);
}
void D3DTextureObject :: SetMagFilter( GLenum mode ) 
{
	switch (mode) {
	case GL_NEAREST:
	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_NEAREST_MIPMAP_LINEAR:
		m_d3dFilter[0] = D3DTEXF_POINT;
		break;
	default:
		m_d3dFilter[0] = D3DTEXF_LINEAR;
		break;
	}
	m_glFilter[0] = mode;
}
void D3DTextureObject :: SetMinFilter( GLenum mode ) 
{
	switch (mode) {
	case GL_NEAREST:
		m_d3dFilter[1] = D3DTEXF_POINT;
		m_d3dFilter[2] = D3DTEXF_NONE;
		break;
	case GL_LINEAR:
		m_d3dFilter[1] = D3DTEXF_LINEAR;
		m_d3dFilter[2] = D3DTEXF_NONE;
		break;
	case GL_NEAREST_MIPMAP_NEAREST:
		m_d3dFilter[1] = D3DTEXF_POINT;
		m_d3dFilter[2] = D3DTEXF_POINT;
		break;
	case GL_NEAREST_MIPMAP_LINEAR:
		m_d3dFilter[1] = D3DTEXF_POINT;
		m_d3dFilter[2] = D3DTEXF_LINEAR;
		break;
	case GL_LINEAR_MIPMAP_NEAREST:
		m_d3dFilter[1] = D3DTEXF_LINEAR;
		m_d3dFilter[2] = D3DTEXF_POINT;
		break;
	case GL_LINEAR_MIPMAP_LINEAR:
	default:
		m_d3dFilter[1] = D3DTEXF_LINEAR;
		m_d3dFilter[2] = D3DTEXF_LINEAR;
		break;
	}
	m_glFilter[1] = mode;
}

static void D3DTex_LoadImage(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
	int targetIndex = UTIL_GLTextureTargettoInternalIndex( target );
	if (targetIndex < 0 || targetIndex >= D3D_TEXTARGET_MAX) {
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}
	int cubeFace = 0;
	if (targetIndex == D3D_TEXTARGET_CUBE)
		cubeFace = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB;
	
	int currentTMU = D3DState.TextureState.currentTMU;
	D3DTextureObject *pTexture = D3DState.TextureState.currentTexture[currentTMU][targetIndex];
	assert(pTexture != nullptr);

	if ( level == 0 ) 
	{
		D3DFORMAT d3dFormat;
		switch (internalformat) {
		case GL_INTENSITY:
		case GL_INTENSITY8:
			d3dFormat = D3DFMT_A8L8;
			break;
		case 1:
		case GL_LUMINANCE:
		case GL_LUMINANCE8:
			d3dFormat = D3DFMT_L8;
			break;
		case GL_ALPHA:
		case GL_ALPHA8:
			d3dFormat = D3DFMT_A8R8G8B8;
			break;
		case 2:
		case GL_LUMINANCE_ALPHA:
		case GL_LUMINANCE8_ALPHA8:
			d3dFormat = D3DFMT_A8L8;
			break;
		case GL_RGB5:
			d3dFormat = D3DFMT_X1R5G5B5;
			break;
		case GL_RGB5_A1:
			d3dFormat = D3DFMT_A1R5G5B5;
			break;
		case GL_RGBA4:
			d3dFormat = D3DFMT_A4R4G4B4;
			break;
		case 3:
		case GL_RGB:
		case GL_RGB8:
		case GL_BGR_EXT:
			d3dFormat = D3DFMT_X8R8G8B8;
			break;
		case 4:
		case GL_RGBA:
		case GL_RGBA8:
		case GL_BGRA_EXT:
			d3dFormat = D3DFMT_A8R8G8B8;
			break;
		default:
			logPrintf("WARNING: Texture internal format 0x%x is not supported\n", internalformat);
			D3DGlobal.lastError = E_INVALIDARG;
			return;
		}

		if (!cubeFace)
		{
			D3DState.TextureState.currentTexture[currentTMU][targetIndex]->FreeD3DTexture();
			HRESULT hr = D3DState.TextureState.currentTexture[currentTMU][targetIndex]->CreateD3DTexture( target, width, height, depth, !!border, d3dFormat, false );
			if (FAILED(hr)) {
				D3DGlobal.lastError = hr;
				return;
			}
		}

		HRESULT hr = pTexture->FillTextureLevel( cubeFace, 0, internalformat, width, height, depth, format, type, pixels );
		if (FAILED(hr)) {
			D3DGlobal.lastError = hr;
			return;
		}
		if (pixels) pTexture->CheckMipmapAutogen();
	}
	else if ( level == 1 )
	{
		if (!cubeFace) 
		{
			HRESULT hr = pTexture->RecreateD3DTexture( true );
			if (FAILED(hr)) {
				D3DGlobal.lastError = hr;
				return;
			}
		}
		HRESULT hr = pTexture->FillTextureLevel( cubeFace, 1, internalformat, width, height, depth, format, type, pixels );
		if (FAILED(hr)) {
			D3DGlobal.lastError = hr;
			return;
		}
	}
	else
	{
		HRESULT hr = pTexture->FillTextureLevel( cubeFace, level, internalformat, width, height, depth, format, type, pixels );
		if (FAILED(hr)) {
			D3DGlobal.lastError = hr;
			return;
		}
	}
}

static void D3DTex_LoadSubImage(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels)
{
	int targetIndex = UTIL_GLTextureTargettoInternalIndex( target );
	if (targetIndex < 0 || targetIndex >= D3D_TEXTARGET_MAX) {
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}
	int cubeFace = 0;
	if (targetIndex == D3D_TEXTARGET_CUBE)
		cubeFace = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB;

	int currentTMU = D3DState.TextureState.currentTMU;
	assert(D3DState.TextureState.currentTexture[currentTMU][targetIndex] != nullptr);
	assert(D3DState.TextureState.currentTexture[currentTMU][targetIndex]->GetD3DTexture() != nullptr);

	if (pixels)
	{
		HRESULT hr = D3DState.TextureState.currentTexture[currentTMU][targetIndex]->FillTextureSubLevel( cubeFace, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels );
		if (FAILED(hr)) {
			D3DGlobal.lastError = hr;
			return;
		}
		if (level == 0) {
			D3DState.TextureState.currentTexture[currentTMU][targetIndex]->CheckMipmapAutogen();
		}
	}
}

static void D3DTex_LoadCompressedImage(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *pixels)
{
	if (!D3DGlobal.supportsS3TC) {
		logPrintf("WARNING: S3TC texture compression is not supported\n");
		return;
	}
	if ((width % 4) || (height % 4)) {
		D3DGlobal.lastError = E_INVALID_OPERATION;
		return;
	}

	int targetIndex = UTIL_GLTextureTargettoInternalIndex( target );
	if (targetIndex < 0 || targetIndex >= D3D_TEXTARGET_MAX) {
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}
	int cubeFace = 0;
	if (targetIndex == D3D_TEXTARGET_CUBE)
		cubeFace = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB;
	
	int currentTMU = D3DState.TextureState.currentTMU;
	D3DTextureObject *pTexture = D3DState.TextureState.currentTexture[currentTMU][targetIndex];
	assert(pTexture != nullptr);

	if ( level == 0 ) 
	{
		D3DFORMAT d3dFormat;
		switch (internalformat) {
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			d3dFormat = D3DFMT_DXT1;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			d3dFormat = D3DFMT_DXT3;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			d3dFormat = D3DFMT_DXT5;
			break;
		default:
			logPrintf("WARNING: Compressed texture internal format 0x%x is not supported\n", internalformat);
			D3DGlobal.lastError = E_INVALIDARG;
			return;
		}

		if (!cubeFace)
		{
			D3DState.TextureState.currentTexture[currentTMU][targetIndex]->FreeD3DTexture();
			HRESULT hr = D3DState.TextureState.currentTexture[currentTMU][targetIndex]->CreateD3DTexture( target, width, height, depth, !!border, d3dFormat, false );
			if (FAILED(hr)) {
				D3DGlobal.lastError = hr;
				return;
			}
		}

		HRESULT hr = pTexture->FillCompressedTextureLevel( cubeFace, 0, internalformat, width, height, depth, imageSize, pixels );
		if (FAILED(hr)) {
			D3DGlobal.lastError = hr;
			return;
		}
		if (pixels) pTexture->CheckMipmapAutogen();
	}
	else if ( level == 1 )
	{
		if (!cubeFace) 
		{
			HRESULT hr = pTexture->RecreateD3DTexture( true );
			if (FAILED(hr)) {
				D3DGlobal.lastError = hr;
				return;
			}
		}
		HRESULT hr = pTexture->FillCompressedTextureLevel( cubeFace, 1, internalformat, width, height, depth, imageSize, pixels );
		if (FAILED(hr)) {
			D3DGlobal.lastError = hr;
			return;
		}
	}
	else
	{
		HRESULT hr = pTexture->FillCompressedTextureLevel( cubeFace, level, internalformat, width, height, depth, imageSize, pixels );
		if (FAILED(hr)) {
			D3DGlobal.lastError = hr;
			return;
		}
	}
}

static void D3DTex_LoadCompressedSubImage( GLenum /*target*/, GLint /*level*/, GLint xoffset, GLint yoffset, GLint /*zoffset*/, GLsizei width, GLsizei height, GLsizei /*depth*/, GLenum /*format*/, GLsizei /*imageSize*/, const GLvoid* /*pixels*/ )
{
	if (!D3DGlobal.supportsS3TC) {
		logPrintf("WARNING: S3TC texture compression is not supported\n");
		return;
	}
	if ((width % 4) || (height % 4) || (xoffset % 4) || (yoffset % 4)) {
		D3DGlobal.lastError = E_INVALID_OPERATION;
		return;
	}

	//!TODO
	logPrintf("WARNING: D3DTex_LoadCompressedSubImage\n");
}

static void D3DTex_CopySubImage( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height )
{
	if (D3DGlobal.skipCopyImage) {
		D3DGlobal.skipCopyImage--;
		return;
	}

	int targetIndex = UTIL_GLTextureTargettoInternalIndex( target );
	if (targetIndex < 0 || targetIndex >= D3D_TEXTARGET_MAX) {
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}
	int cubeFace = 0;
	if (targetIndex == D3D_TEXTARGET_CUBE)
		cubeFace = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB;

	int currentTMU = D3DState.TextureState.currentTMU;
	assert(D3DState.TextureState.currentTexture[currentTMU][targetIndex] != nullptr);
	assert(D3DState.TextureState.currentTexture[currentTMU][targetIndex]->GetD3DTexture() != nullptr);

	HRESULT hr = D3DState.TextureState.currentTexture[currentTMU][targetIndex]->CopyTextureSubLevel( cubeFace, level, xoffset, yoffset, x, y, width, height );
	if (FAILED(hr)) {
		D3DGlobal.lastError = hr;
		return;
	}
	if (level == 0) {
		D3DState.TextureState.currentTexture[currentTMU][targetIndex]->CheckMipmapAutogen();
	}
}

//=========================================
OPENGL_API void WINAPI glDeleteTextures( GLsizei n, const GLuint *textures )
{
	assert(D3DGlobal.pObjectBuffer != nullptr);
	HRESULT hr = D3DGlobal.pObjectBuffer->DeleteObjects( D3D_OBJECT_TYPE_TEXTURE, n, textures );
	if (FAILED(hr)) D3DGlobal.lastError = hr;
}
OPENGL_API void WINAPI glGenTextures( GLsizei n, GLuint *textures )
{
	assert(D3DGlobal.pObjectBuffer != nullptr);
	HRESULT hr = D3DGlobal.pObjectBuffer->GenObjects( D3D_OBJECT_TYPE_TEXTURE, n, textures );
	if (FAILED(hr)) D3DGlobal.lastError = hr;
}
OPENGL_API GLboolean WINAPI glIsTexture( GLuint texture )
{
	assert(D3DGlobal.pObjectBuffer != nullptr);
	return D3DGlobal.pObjectBuffer->IsObject( D3D_OBJECT_TYPE_TEXTURE, texture );
}
OPENGL_API GLboolean WINAPI glAreTexturesResident( GLsizei n, const GLuint *textures, GLboolean *residences )
{
	if (n <= 0) {
		D3DGlobal.lastError = E_INVALIDARG;
		return GL_FALSE;
	}

	//We assume all textures resident
	//FIXME: query Direct3D? how?
	for (int i = 0; i < n; ++i) {
		residences[i] = GL_FALSE;
		if (textures[i] <= 0) continue;
		D3DTextureObject *pTexture = (D3DTextureObject*)D3DGlobal.pObjectBuffer->GetObjectData( D3D_OBJECT_TYPE_TEXTURE, textures[i] );
		if (pTexture) {
			residences[i] = GL_TRUE;
		}
	}

	return GL_TRUE;
}
OPENGL_API void WINAPI glPrioritizeTextures( GLsizei n, const GLuint *textures, const GLclampf *priorities )
{
	if (n < 0) {
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}

	assert(D3DGlobal.pObjectBuffer != nullptr);
	for (int i = 0; i < n; ++i) {
		if (textures[i] <= 0) continue;
		D3DTextureObject *pTexture = (D3DTextureObject*)D3DGlobal.pObjectBuffer->GetObjectData( D3D_OBJECT_TYPE_TEXTURE, textures[i] );
		if (pTexture) {
			DWORD dwPriority = DWORD(priorities[i] * INT_MAX);
			pTexture->SetPriority( dwPriority );
			if (pTexture->GetD3DTexture()) pTexture->GetD3DTexture()->SetPriority( dwPriority );
		}
	}
}
OPENGL_API void WINAPI glBindTexture( GLenum target, GLuint texture )
{
	int targetIndex = UTIL_GLTextureTargettoInternalIndex( target );
	if (targetIndex < 0 || targetIndex >= D3D_TEXTARGET_MAX) {
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}

	int currentTMU = D3DState.TextureState.currentTMU;

	if (!texture) {
		if (D3DState.TextureState.currentTexture[currentTMU][targetIndex] != D3DGlobal.defaultTexture[targetIndex]) {
			D3DState.TextureState.currentTexture[currentTMU][targetIndex] = D3DGlobal.defaultTexture[targetIndex];
			D3DState.TextureState.textureChanged[currentTMU][targetIndex] = TRUE;
			D3DState.TextureState.textureStateChanged[currentTMU] = TRUE;
			D3DState.TextureState.textureSamplerStateChanged = TRUE;
		}
		D3DGlobal.lastError = S_OK;
		return;
	}
	
	assert(D3DGlobal.pObjectBuffer != nullptr);
	D3DTextureObject *pTexture = (D3DTextureObject*)D3DGlobal.pObjectBuffer->GetObjectData( D3D_OBJECT_TYPE_TEXTURE, texture );
	if (!pTexture) {
		D3DState.TextureState.currentTexture[currentTMU][targetIndex] = D3DGlobal.defaultTexture[targetIndex];
		D3DGlobal.lastError = E_OUTOFMEMORY;
		return;
	}
	if ((pTexture->GetD3DTexture() != nullptr) && (pTexture->GetTarget() != target)) {
		D3DState.TextureState.currentTexture[currentTMU][targetIndex] = D3DGlobal.defaultTexture[targetIndex];
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}
	if (D3DState.TextureState.currentTexture[currentTMU][targetIndex] != pTexture) {
		D3DState.TextureState.currentTexture[currentTMU][targetIndex] = pTexture;
		D3DState.TextureState.textureChanged[currentTMU][targetIndex] = TRUE;
		D3DState.TextureState.textureStateChanged[currentTMU] = TRUE;
		D3DState.TextureState.textureSamplerStateChanged = TRUE;
	}
}
OPENGL_API void WINAPI glTexImage1D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
	//refer 1d-textures to as 2d-textures with height = 1
	D3DTex_LoadImage( target, level, internalformat, width, 1, 1, border, format, type, pixels );
}
OPENGL_API void WINAPI glTexImage2D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
	D3DTex_LoadImage( target, level, internalformat, width, height, 1, border, format, type, pixels );
}
OPENGL_API void WINAPI glTexImage3D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
	D3DTex_LoadImage( target, level, internalformat, width, height, depth, border, format, type, pixels );
}
OPENGL_API void WINAPI glTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels )
{
	D3DTex_LoadSubImage( target, level, xoffset, 0, 0, width, 1, 1, format, type, pixels );
}
OPENGL_API void WINAPI glTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
	D3DTex_LoadSubImage( target, level, xoffset, yoffset, 0, width, height, 1, format, type, pixels );
}
OPENGL_API void WINAPI glTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels)
{
	D3DTex_LoadSubImage( target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels );
}
OPENGL_API void WINAPI glCompressedTexImage1D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *pixels )
{
	//refer 1d-textures to as 2d-textures with height = 1
	D3DTex_LoadCompressedImage( target, level, internalformat, width, 1, 1, border, imageSize, pixels );
}
OPENGL_API void WINAPI glCompressedTexImage2D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *pixels )
{
	D3DTex_LoadCompressedImage( target, level, internalformat, width, height, 1, border, imageSize, pixels );
}
OPENGL_API void WINAPI glCompressedTexImage3D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *pixels )
{
	D3DTex_LoadCompressedImage( target, level, internalformat, width, height, depth, border, imageSize, pixels );
}
OPENGL_API void WINAPI glCompressedTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *pixels )
{
	D3DTex_LoadCompressedSubImage( target, level, xoffset, 0, 0, width, 1, 1, format, imageSize, pixels );
}
OPENGL_API void WINAPI glCompressedTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *pixels )
{
	D3DTex_LoadCompressedSubImage( target, level, xoffset, yoffset, 0, width, height, 1, format, imageSize, pixels );
}
OPENGL_API void WINAPI glCompressedTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *pixels )
{
	D3DTex_LoadCompressedSubImage( target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, pixels );
}

OPENGL_API void WINAPI glCopyTexImage1D( GLenum target, GLint level, GLenum /*internalFormat*/, GLint x, GLint y, GLsizei width, GLint /*border*/ )
{
	//FIXME: format conversion! border!!
	D3DTex_CopySubImage( target, level, 0, 0, x, y, width, 1 );
}
OPENGL_API void WINAPI glCopyTexImage2D( GLenum target, GLint level, GLenum /*internalFormat*/, GLint x, GLint y, GLsizei width, GLsizei height, GLint /*border*/ )
{
	//FIXME: format conversion! border!!
	D3DTex_CopySubImage( target, level, 0, 0, x, y, width, height );
}
OPENGL_API void WINAPI glCopyTexImage3D( GLenum /*target*/, GLint /*level*/, GLenum /*internalFormat*/, GLint /*x*/, GLint /*y*/, GLint /*z*/, GLsizei /*width*/, GLsizei /*height*/, GLsizei /*depth*/, GLint /*border*/ )
{
	logPrintf("WARNING: unimplemented function - glCopyTexImage3D\n");
}
OPENGL_API void WINAPI glCopyTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width )
{
	D3DTex_CopySubImage( target, level, xoffset, 0, x, y, width, 1 );
}
OPENGL_API void WINAPI glCopyTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height )
{
	D3DTex_CopySubImage( target, level, xoffset, yoffset, x, y, width, height );
}
OPENGL_API void WINAPI glCopyTexSubImage3D( GLenum /*target*/, GLint /*level*/, GLint /*xoffset*/, GLint /*yoffset*/, GLint /*zoffset*/, GLint /*x*/, GLint /*y*/, GLint /*z*/, GLsizei /*width*/, GLsizei /*height*/, GLsizei /*depth*/ )
{
	logPrintf("WARNING: unimplemented function - glCopyTexSubImage3D\n");
}

OPENGL_API void WINAPI glGetTexImage( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels )
{
	int targetIndex = UTIL_GLTextureTargettoInternalIndex( target );
	if (targetIndex < 0 || targetIndex >= D3D_TEXTARGET_MAX) {
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}
	int cubeFace = 0;
	if (targetIndex == D3D_TEXTARGET_CUBE)
		cubeFace = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB;

	int currentTMU = D3DState.TextureState.currentTMU;
	D3DTextureObject *pTexture = D3DState.TextureState.currentTexture[currentTMU][targetIndex];
	if (!pTexture) {
		logPrintf("WARNING: glGetTexImage(0x%x, %i) - texture was not uploaded\n", target, level);
		return;
	}

	HRESULT hr = pTexture->GetTexImage( cubeFace, level, format, type, pixels );
	if (FAILED(hr)) D3DGlobal.lastError = hr;
}

OPENGL_API void WINAPI glGetCompressedTexImage( GLenum target, GLint level, GLvoid * /*img*/ )
{
	int targetIndex = UTIL_GLTextureTargettoInternalIndex( target );
	if (targetIndex < 0 || targetIndex >= D3D_TEXTARGET_MAX) {
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}
	int cubeFace = 0;
	if (targetIndex == D3D_TEXTARGET_CUBE)
		cubeFace = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB;

	int currentTMU = D3DState.TextureState.currentTMU;
	D3DTextureObject *pTexture = D3DState.TextureState.currentTexture[currentTMU][targetIndex];
	if (!pTexture) {
		logPrintf("WARNING: glGetCompressedTexImage(0x%x, %i) - texture was not uploaded\n", target, level);
		return;
	}

	if (!D3DGlobal.supportsS3TC) {
		logPrintf("WARNING: S3TC texture compression is not supported\n");
		return;
	}

	//!TODO
}

OPENGL_API void WINAPI glTexParameterfv( GLenum target, GLenum pname, const GLfloat *params )
{
	int targetIndex = UTIL_GLTextureTargettoInternalIndex( target );
	if (targetIndex < 0 || targetIndex >= D3D_TEXTARGET_MAX) {
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}

	int currentTMU = D3DState.TextureState.currentTMU;
	assert(D3DState.TextureState.currentTexture[currentTMU][targetIndex] != nullptr);

	switch (pname) {
	case GL_TEXTURE_WRAP_S:
	case GL_TEXTURE_WRAP_T:
	case GL_TEXTURE_WRAP_R_EXT:
		D3DState.TextureState.currentTexture[currentTMU][targetIndex]->SetAddressMode( pname, (GLenum)params[0] );
		break;
	case GL_TEXTURE_MAG_FILTER:
		D3DState.TextureState.currentTexture[currentTMU][targetIndex]->SetMagFilter( (GLenum)params[0] );
		break;
	case GL_TEXTURE_MIN_FILTER:
		D3DState.TextureState.currentTexture[currentTMU][targetIndex]->SetMinFilter( (GLenum)params[0] );
		break;
	case GL_TEXTURE_BORDER_COLOR:
		D3DState.TextureState.currentTexture[currentTMU][targetIndex]->SetBorderColor( D3DCOLOR_ARGB( QINDIEGL_CLAMP(params[3] * 255), QINDIEGL_CLAMP(params[0] * 255), QINDIEGL_CLAMP(params[1] * 255), QINDIEGL_CLAMP(params[2] * 255) ));
		break;
	case GL_TEXTURE_PRIORITY:
		if (D3DState.TextureState.currentTexture[currentTMU][targetIndex]) {
			DWORD dwPriority = DWORD(params[0] * INT_MAX);
			D3DState.TextureState.currentTexture[currentTMU][targetIndex]->SetPriority( dwPriority );
			if (D3DState.TextureState.currentTexture[currentTMU][targetIndex]->GetD3DTexture()) 
				D3DState.TextureState.currentTexture[currentTMU][targetIndex]->GetD3DTexture()->SetPriority( dwPriority );
		}
		break;
	case GL_GENERATE_MIPMAP_SGIS:
		D3DState.TextureState.currentTexture[currentTMU][targetIndex]->SetMipmapAutogen( (GLboolean)params[0] );
		break;
	case GL_TEXTURE_MAX_ANISOTROPY_EXT:
		D3DState.TextureState.currentTexture[currentTMU][targetIndex]->SetAnisotropy( (GLint)params[0] );
		break;
	case GL_TEXTURE_LOD_BIAS_EXT:
		D3DState.TextureState.currentTexture[currentTMU][targetIndex]->SetLodBias( params[0] );
		break;
	default:
		logPrintf("WARNING: glTexParameterfv - unknown parameter 0x%x (values %f %f %f %f)\n", pname, params[0],params[1],params[2],params[3]);
		break;
	}
	D3DState.TextureState.textureChanged[currentTMU][targetIndex] = TRUE;
	D3DState.TextureState.textureStateChanged[currentTMU] = TRUE;
	D3DState.TextureState.textureSamplerStateChanged = TRUE;
}

OPENGL_API void WINAPI glTexParameterf( GLenum target, GLenum pname, GLfloat param )
{
	glTexParameterfv( target, pname, &param );
}
OPENGL_API void WINAPI glTexParameteri( GLenum target, GLenum pname, GLint param )
{
	GLfloat fparam = (GLfloat)param;
	glTexParameterfv( target, pname, &fparam );
}
OPENGL_API void WINAPI glTexParameteriv( GLenum target, GLenum pname, const GLint *params )
{
	GLfloat fparams[4];
	fparams[0] = (GLfloat)params[0];
	fparams[1] = (GLfloat)params[1];
	fparams[2] = (GLfloat)params[2];
	fparams[3] = (GLfloat)params[3];
	glTexParameterfv( target, pname, fparams );	
}

OPENGL_API void WINAPI glGetTexParameterfv( GLenum target, GLenum pname, GLfloat *params )
{
	int targetIndex = UTIL_GLTextureTargettoInternalIndex( target );
	if (targetIndex < 0 || targetIndex >= D3D_TEXTARGET_MAX) {
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}

	int currentTMU = D3DState.TextureState.currentTMU;
	D3DTextureObject *pTexture = D3DState.TextureState.currentTexture[currentTMU][targetIndex];
	if (!pTexture) {
		params[0] = 0;
		return;
	}
	
	switch (pname) {
	case GL_TEXTURE_WRAP_S:
		params[0] = (GLfloat)pTexture->GetAddressMode(0);
		break;
	case GL_TEXTURE_WRAP_T:
		params[0] = (GLfloat)pTexture->GetAddressMode(1);
		break;
	case GL_TEXTURE_WRAP_R_EXT:
		params[0] = (GLfloat)pTexture->GetAddressMode(2);
		break;
	case GL_TEXTURE_MAG_FILTER:
		params[0] = (GLfloat)pTexture->GetFilter(0);
		break;
	case GL_TEXTURE_MIN_FILTER:
		params[0] = (GLfloat)pTexture->GetFilter(1);
		break;
	case GL_TEXTURE_BORDER_COLOR:
		params[0] = ((GLfloat)((pTexture->GetBorderColor() >> 16) & 0xFF) / 255.0f );
		params[1] = ((GLfloat)((pTexture->GetBorderColor() >> 8) & 0xFF) / 255.0f );
		params[2] = ((GLfloat)((pTexture->GetBorderColor()) & 0xFF) / 255.0f );
		params[3] = ((GLfloat)((pTexture->GetBorderColor()>> 24) & 0xFF) / 255.0f );
		break;
	case GL_TEXTURE_PRIORITY:
		params[0] = (GLfloat)pTexture->GetPriority() / (GLfloat)INT_MAX;
		break;
	case GL_TEXTURE_RESIDENT:
		params[0] = 1.0f;
		break;
	case GL_GENERATE_MIPMAP_SGIS:
		params[0] = (GLfloat)pTexture->GetMipmapAutogen();
		break;
	case GL_TEXTURE_MAX_ANISOTROPY_EXT:
		params[0] = (GLfloat)pTexture->GetAnisotropy();
		break;

	default:
		logPrintf("WARNING: glGetTexParameterfv - unknown parameter 0x%x\n", pname);
		break;
	}
}

OPENGL_API void WINAPI glGetTexParameteriv( GLenum target, GLenum pname, GLint *params )
{
	int targetIndex = UTIL_GLTextureTargettoInternalIndex( target );
	if (targetIndex < 0 || targetIndex >= D3D_TEXTARGET_MAX) {
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}

	int currentTMU = D3DState.TextureState.currentTMU;
	D3DTextureObject *pTexture = D3DState.TextureState.currentTexture[currentTMU][targetIndex];
	if (!pTexture) {
		params[0] = 0;
		return;
	}
	
	switch (pname) {
	case GL_TEXTURE_BORDER_COLOR:
		params[0] = (GLint)((GLfloat)((pTexture->GetBorderColor() >> 16) & 0xFF) * INT_MAX / 255.0f );
		params[1] = (GLint)((GLfloat)((pTexture->GetBorderColor() >> 8) & 0xFF) * INT_MAX / 255.0f );
		params[2] = (GLint)((GLfloat)((pTexture->GetBorderColor()) & 0xFF) * INT_MAX / 255.0f );
		params[3] = (GLint)((GLfloat)((pTexture->GetBorderColor() >> 24) & 0xFF) * INT_MAX / 255.0f );
		break;
	default:
		{
			GLfloat fparam;
			glGetTexParameterfv( target, pname, &fparam );
			params[0] = (GLint)fparam;
			break;
		}
	}
}

OPENGL_API void WINAPI glTexEnvfv( GLenum target, GLenum pname, const GLfloat *params )
{
	switch (target) 
	{
	default:
		{
			logPrintf("WARNING: glTexEnvfv - unknown target 0x%x\n", target);
			D3DGlobal.lastError = E_INVALID_ENUM;
			return;
		}
	case GL_TEXTURE_ENV:
		{
			switch (pname) 
			{
			case GL_TEXTURE_ENV_MODE:
				{
					GLenum newMode = (GLenum)params[0];
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].envMode != newMode) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].envMode = newMode;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}
				break;
			case GL_TEXTURE_ENV_COLOR:
				{
					D3DCOLOR envColor = D3DCOLOR_ARGB( QINDIEGL_CLAMP( params[3] * 255 ),
													  QINDIEGL_CLAMP( params[0] * 255 ),
													  QINDIEGL_CLAMP( params[1] * 255 ),
													  QINDIEGL_CLAMP( params[2] * 255 ));
					if (D3DState.TextureState.textureEnvColor != envColor) {
						D3DState.TextureState.textureEnvColor = envColor;
						D3DState_SetRenderState( D3DRS_TEXTUREFACTOR, envColor );
					}
					break;
				}
			case GL_COMBINE_RGB_ARB:
				{
					GLenum newMode = (GLenum)params[0];
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorOp != newMode) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorOp = newMode;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}
			case GL_COMBINE_ALPHA_ARB:
				{
					GLenum newMode = (GLenum)params[0];
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaOp != newMode) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaOp = newMode;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}
			case GL_SOURCE0_RGB_ARB:
				{
					GLenum newMode = (GLenum)params[0];
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorArg1 != newMode) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorArg1 = newMode;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}
			case GL_SOURCE1_RGB_ARB:
				{
					GLenum newMode = (GLenum)params[0];
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorArg2 != newMode) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorArg2 = newMode;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}
			case GL_SOURCE2_RGB_ARB:
				{
					GLenum newMode = (GLenum)params[0];
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorArg3 != newMode) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorArg3 = newMode;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}
			case GL_SOURCE0_ALPHA_ARB:
				{
					GLenum newMode = (GLenum)params[0];
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaArg1 != newMode) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaArg1 = newMode;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}
			case GL_SOURCE1_ALPHA_ARB:
				{
					GLenum newMode = (GLenum)params[0];
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaArg2 != newMode) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaArg2 = newMode;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}
			case GL_SOURCE2_ALPHA_ARB:
				{
					GLenum newMode = (GLenum)params[0];
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaArg3 != newMode) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaArg3 = newMode;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}
			case GL_OPERAND0_RGB_ARB:
				{
					GLenum newMode = (GLenum)params[0];
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorOperand1 != newMode) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorOperand1 = newMode;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}
			case GL_OPERAND1_RGB_ARB:
				{
					GLenum newMode = (GLenum)params[0];
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorOperand2 != newMode) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorOperand2 = newMode;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}
			case GL_OPERAND2_RGB_ARB:
				{
					GLenum newMode = (GLenum)params[0];
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorOperand3 != newMode) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorOperand3 = newMode;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}
			case GL_OPERAND0_ALPHA_ARB:
				{
					GLenum newMode = (GLenum)params[0];
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaOperand1 != newMode) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaOperand1 = newMode;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}
			case GL_OPERAND1_ALPHA_ARB:
				{
					GLenum newMode = (GLenum)params[0];
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaOperand2 != newMode) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaOperand2 = newMode;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}
			case GL_OPERAND2_ALPHA_ARB:
				{
					GLenum newMode = (GLenum)params[0];
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaOperand3 != newMode) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaOperand3 = newMode;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}
			case GL_RGB_SCALE_ARB:
				{
					DWORD scale = (GLint)params[0];
					if (scale >= 4) scale = 4;
					else if (scale >= 2) scale = 2;
					else scale = 1;
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorScale != scale) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorScale = scale;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}
			case GL_ALPHA_SCALE:
				{
					DWORD scale = (GLint)params[0];
					if (scale >= 4) scale = 4;
					else if (scale >= 2) scale = 2;
					else scale = 1;
					if (D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaScale != scale) {
						D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaScale = scale;
						D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
						D3DState.TextureState.textureSamplerStateChanged = TRUE;
					}
					break;
				}

			default:
				logPrintf("WARNING: glTexEnvfv(GL_TEXTURE_ENV) - unknown pname 0x%x (values %f %f %f %f)\n", pname, params[0],params[1],params[2],params[3]);
				break;
			}
			break;
		}
		case GL_TEXTURE_FILTER_CONTROL_EXT:
		{
			switch (pname) 
			{
			case GL_TEXTURE_LOD_BIAS_EXT:
				if (D3DState.TextureState.textureLodBias[D3DState.TextureState.currentTMU] != params[0]) {
					D3DState.TextureState.textureLodBias[D3DState.TextureState.currentTMU] = params[0];
					D3DState.TextureState.textureEnvModeChanged[D3DState.TextureState.currentTMU] = TRUE;
					D3DState.TextureState.textureSamplerStateChanged = TRUE;
				}
				break;
			default:
				logPrintf("WARNING: glTexEnvfv(GL_TEXTURE_FILTER_CONTROL_EXT) - unknown pname 0x%x (values %f %f %f %f)\n", pname, params[0],params[1],params[2],params[3]);
				break;
			}
			break;
		}
	}
}

OPENGL_API void WINAPI glTexEnvf( GLenum target, GLenum pname, GLfloat param )
{
	glTexEnvfv( target, pname, &param );
}

OPENGL_API void WINAPI glTexEnviv( GLenum target, GLenum pname, const GLint *params )
{
	if (target == GL_TEXTURE_ENV) {
		if (pname == GL_TEXTURE_ENV_COLOR) {
			D3DCOLOR envColor = D3DCOLOR_ARGB( QINDIEGL_CLAMP( (GLfloat)(params[3] / INT_MAX) * 255 ),
											  QINDIEGL_CLAMP( (GLfloat)(params[0] / INT_MAX) * 255 ),
											  QINDIEGL_CLAMP( (GLfloat)(params[1] / INT_MAX) * 255 ),
											  QINDIEGL_CLAMP( (GLfloat)(params[2] / INT_MAX) * 255 ));
			if (D3DState.TextureState.textureEnvColor != envColor) {
				D3DState.TextureState.textureEnvColor = envColor;
				D3DState_SetRenderState( D3DRS_TEXTUREFACTOR, envColor );
			}
			return;
		}
	}

	GLfloat fparams[4];
	fparams[0] = (GLfloat)params[0];
	fparams[1] = (GLfloat)params[1];
	fparams[2] = (GLfloat)params[2];
	fparams[3] = (GLfloat)params[3];
	glTexEnvfv( target, pname, fparams );
}

OPENGL_API void WINAPI glTexEnvi( GLenum target, GLenum pname, GLint param )
{
	glTexEnviv( target, pname, &param );
}

OPENGL_API void WINAPI glGetTexEnvfv( GLenum target, GLenum pname, GLfloat *params )
{
	switch (target) 
	{
	default:
		{
			logPrintf("WARNING: glGetTexEnvfv - unknown target 0x%x\n", target);
			D3DGlobal.lastError = E_INVALID_ENUM;
			return;
		}
	case GL_TEXTURE_ENV:
		{
			switch (pname) {
			case GL_TEXTURE_ENV_MODE:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].envMode;
				break;
			case GL_TEXTURE_ENV_COLOR:
				params[0] = ((GLfloat)((D3DState.TextureState.textureEnvColor >> 16) & 0xFF) / 255.0f );
				params[1] = ((GLfloat)((D3DState.TextureState.textureEnvColor >> 8) & 0xFF) / 255.0f );
				params[2] = ((GLfloat)((D3DState.TextureState.textureEnvColor) & 0xFF) / 255.0f );
				params[3] = ((GLfloat)((D3DState.TextureState.textureEnvColor >> 24) & 0xFF) / 255.0f );
				break;
			case GL_COMBINE_RGB_ARB:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorOp;
				break;
			case GL_COMBINE_ALPHA_ARB:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaOp;
				break;
			case GL_SOURCE0_RGB_ARB:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorArg1;
				break;
			case GL_SOURCE1_RGB_ARB:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorArg2;
				break;
			case GL_SOURCE2_RGB_ARB:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorArg3;
				break;
			case GL_SOURCE0_ALPHA_ARB:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaArg1;
				break;
			case GL_SOURCE1_ALPHA_ARB:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaArg2;
				break;
			case GL_SOURCE2_ALPHA_ARB:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaArg3;
				break;
			case GL_OPERAND0_RGB_ARB:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorOperand1;
				break;
			case GL_OPERAND1_RGB_ARB:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorOperand2;
				break;
			case GL_OPERAND2_RGB_ARB:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorOperand3;
				break;
			case GL_OPERAND0_ALPHA_ARB:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaOperand1;
				break;
			case GL_OPERAND1_ALPHA_ARB:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaOperand2;
				break;
			case GL_OPERAND2_ALPHA_ARB:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaOperand3;
				break;
			case GL_RGB_SCALE_ARB:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].colorScale;
				break;
			case GL_ALPHA_SCALE:
				params[0] = (GLfloat)D3DState.TextureState.TextureCombineState[D3DState.TextureState.currentTMU].alphaScale;
				break;
			default:
				logPrintf("WARNING: glGetTexEnvfv(GL_TEXTURE_ENV) - unknown pname 0x%x\n", pname);
				break;
			}
			break;
		}
	case GL_TEXTURE_FILTER_CONTROL_EXT:
		{
			switch (pname) {
			case GL_TEXTURE_LOD_BIAS_EXT:
				params[0] = D3DState.TextureState.textureLodBias[D3DState.TextureState.currentTMU];
				break;
			default:
				logPrintf("WARNING: glGetTexEnvfv(GL_TEXTURE_FILTER_CONTROL_EXT) - unknown pname 0x%x\n", pname);
				break;
			}
			break;
		}
	}
}

OPENGL_API void WINAPI glGetTexEnviv( GLenum target, GLenum pname, GLint *params )
{
	if (target != GL_TEXTURE_ENV) {
		logPrintf("WARNING: glGetTexEnviv - unknown target 0x%x\n", target);
		D3DGlobal.lastError = E_INVALID_ENUM;
		return;
	}

	switch (pname) {
	case GL_TEXTURE_ENV_COLOR:
		{
			params[0] = (GLint)((GLfloat)((D3DState.TextureState.textureEnvColor >> 16) & 0xFF) * INT_MAX / 255.0f );
			params[1] = (GLint)((GLfloat)((D3DState.TextureState.textureEnvColor >> 8) & 0xFF) * INT_MAX / 255.0f );
			params[2] = (GLint)((GLfloat)((D3DState.TextureState.textureEnvColor) & 0xFF) * INT_MAX / 255.0f );
			params[3] = (GLint)((GLfloat)((D3DState.TextureState.textureEnvColor >> 24) & 0xFF) * INT_MAX / 255.0f );
			break;
		}
	default:
		{
			GLfloat fparam;
			glGetTexEnvfv( target, pname, &fparam );
			params[0] = (GLint)fparam;
			break;
		}
	}
}

OPENGL_API void WINAPI glGetTexLevelParameterfv( GLenum target, GLint level, GLenum pname, GLfloat *params )
{
	D3DTextureObject *currentTexture( nullptr );

	int targetIndex = UTIL_GLTextureTargettoInternalIndex( target );
	if (targetIndex < 0 || targetIndex >= D3D_TEXTARGET_MAX) {
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}

	currentTexture = D3DState.TextureState.currentTexture[D3DState.TextureState.currentTMU][targetIndex];
	if (!currentTexture) currentTexture = D3DGlobal.defaultTexture[targetIndex];
		
	switch (pname) {
	case GL_TEXTURE_WIDTH:
		params[0] = (GLfloat)(currentTexture->GetWidth() >> level);
		break;
	case GL_TEXTURE_HEIGHT:
		params[0] = (GLfloat)(currentTexture->GetHeight() >> level);
		break;
	case GL_TEXTURE_DEPTH:
		params[0] = (GLfloat)(currentTexture->GetDepth() >> level);
		break;
	default:
		logPrintf("WARNING: unimplemented parameter in glGetTexLevelParameterfv - 0x%x\n", pname);
		break;
	}	
}

OPENGL_API void WINAPI glGetTexLevelParameteriv( GLenum target, GLint level, GLenum pname, GLint *params )
{
	GLfloat fparam;
	glGetTexLevelParameterfv( target, level, pname, &fparam );
	params[0] = (GLint)fparam;
}

OPENGL_API void WINAPI glActiveTexture( GLenum texture )
{
	int stageIndex = texture - GL_TEXTURE0_ARB;
	if (stageIndex < 0 || stageIndex >= D3DGlobal.maxActiveTMU) {
		logPrintf("WARNING: glActiveTexture - bad stage %i\n", stageIndex);
		D3DGlobal.lastError = E_INVALID_ENUM;
		return;
	}
	D3DState.TextureState.currentTMU = stageIndex;
	if (D3DState.TransformState.matrixMode == GL_TEXTURE)
		D3DState_SetMatrixMode();
}
OPENGL_API void WINAPI glClientActiveTexture( GLenum texture )
{
	int stageIndex = texture - GL_TEXTURE0_ARB;
	if (stageIndex < 0 || stageIndex >= D3DGlobal.maxActiveTMU) {
		logPrintf("WARNING: glClientActiveTexture - bad stage %i\n", stageIndex);
		D3DGlobal.lastError = E_INVALID_ENUM;
		return;
	}
	D3DState.ClientTextureState.currentClientTMU = stageIndex;
}
OPENGL_API void WINAPI glSelectTexture( GLenum texture )
{
	int stageIndex = texture - GL_TEXTURE0_SGIS;
	if (stageIndex < 0 || stageIndex >= D3DGlobal.maxActiveTMU) {
		logPrintf("WARNING: glSelectTextureSGIS - bad stage %i\n", stageIndex);
		D3DGlobal.lastError = E_INVALID_ENUM;
		return;
	}
	D3DState.TextureState.currentTMU = stageIndex;
	if (D3DState.TransformState.matrixMode == GL_TEXTURE)
		D3DState_SetMatrixMode();
}
