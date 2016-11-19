/***************************************************************************
* Copyright(C) 2011-2016, Crystice Softworks.
* 
* This file is part of QindieGL source code.
* Please note that QindieGL is not driver, it's emulator.
* 
* QindieGL source code is free software; you can redistribute it and/or 
* modify it under the terms of the GNU General Public License as 
* published by the Free Software Foundation; either version 2 of 
* the License, or(at your option) any later version.
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
#include "d3d_pixels.hpp"

//==================================================================================
// Pixel operations
//==================================================================================

// D3DPixels_Unpack/D3DPixels_Pack
// Transfer source pixel data to destination
// The following is taken into account:
// - source pixel data type
// - source pixel format
// - current pixel store parameters
// - current pixel transfer and mapping(if any)

#define PIXEL_FLAG_BGR				0x1
#define PIXEL_FLAG_ALPHA_FIRST		0x2

static inline unsigned short D3D_ByteSwap16( unsigned short x ) 
{
	return(x>>8) |(x<<8);
}

static inline unsigned int D3D_ByteSwap32( unsigned int x ) 
{
	return(D3D_ByteSwap16(x&0xffff)<<16) |(D3D_ByteSwap16(x>>16));
}

inline void D3DPixels_ModifyA( GLubyte *a )
{
	float fA = QINDIEGL_MIN( 1.0f, QINDIEGL_MAX( 0.0f,((float)(*a) / 255.0f) * D3DState.ClientPixelStoreState.transferAlphaScale + D3DState.ClientPixelStoreState.transferAlphaBias ));

	if(D3DState.ClientPixelStoreState.transferMapColor)
		fA = D3DState.ClientPixelStoreState.pixelmapAtoA[(DWORD)(fA *(D3DState.ClientPixelStoreState.pixelmapSizeAtoA-1))];

	*a = static_cast<GLubyte>( QINDIEGL_CLAMP( fA * 255.0f ) );
}

inline void D3DPixels_ModifyBGR( GLubyte *bgr )
{
	float fR = QINDIEGL_MIN( 1.0f, QINDIEGL_MAX( 0.0f,((float)bgr[2] / 255.0f) * D3DState.ClientPixelStoreState.transferRedScale + D3DState.ClientPixelStoreState.transferRedBias ));
	float fG = QINDIEGL_MIN( 1.0f, QINDIEGL_MAX( 0.0f,((float)bgr[1] / 255.0f) * D3DState.ClientPixelStoreState.transferGreenScale + D3DState.ClientPixelStoreState.transferGreenBias ));
	float fB = QINDIEGL_MIN( 1.0f, QINDIEGL_MAX( 0.0f,((float)bgr[0] / 255.0f) * D3DState.ClientPixelStoreState.transferBlueScale + D3DState.ClientPixelStoreState.transferBlueBias ));

	if(D3DState.ClientPixelStoreState.transferMapColor) {
		fR = D3DState.ClientPixelStoreState.pixelmapRtoR[(DWORD)(fR *(D3DState.ClientPixelStoreState.pixelmapSizeRtoR-1))];
		fG = D3DState.ClientPixelStoreState.pixelmapGtoG[(DWORD)(fG *(D3DState.ClientPixelStoreState.pixelmapSizeGtoG-1))];
		fB = D3DState.ClientPixelStoreState.pixelmapBtoB[(DWORD)(fB *(D3DState.ClientPixelStoreState.pixelmapSizeBtoB-1))];
	}

	bgr[2] = static_cast<GLubyte>( QINDIEGL_CLAMP( fR * 255.0f ) );
	bgr[1] = static_cast<GLubyte>( QINDIEGL_CLAMP( fG * 255.0f ) );
	bgr[0] = static_cast<GLubyte>( QINDIEGL_CLAMP( fB * 255.0f ) );
}

inline void D3DPixels_ModifyBGRA( GLubyte *bgra )
{
	float fR = QINDIEGL_MIN( 1.0f, QINDIEGL_MAX( 0.0f,((float)bgra[2] / 255.0f) * D3DState.ClientPixelStoreState.transferRedScale + D3DState.ClientPixelStoreState.transferRedBias ));
	float fG = QINDIEGL_MIN( 1.0f, QINDIEGL_MAX( 0.0f,((float)bgra[1] / 255.0f) * D3DState.ClientPixelStoreState.transferGreenScale + D3DState.ClientPixelStoreState.transferGreenBias ));
	float fB = QINDIEGL_MIN( 1.0f, QINDIEGL_MAX( 0.0f,((float)bgra[0] / 255.0f) * D3DState.ClientPixelStoreState.transferBlueScale + D3DState.ClientPixelStoreState.transferBlueBias ));
	float fA = QINDIEGL_MIN( 1.0f, QINDIEGL_MAX( 0.0f,((float)bgra[3] / 255.0f) * D3DState.ClientPixelStoreState.transferAlphaScale + D3DState.ClientPixelStoreState.transferAlphaBias ));

	if(D3DState.ClientPixelStoreState.transferMapColor) {
		fR = D3DState.ClientPixelStoreState.pixelmapRtoR[(DWORD)(fR *(D3DState.ClientPixelStoreState.pixelmapSizeRtoR-1))];
		fG = D3DState.ClientPixelStoreState.pixelmapGtoG[(DWORD)(fG *(D3DState.ClientPixelStoreState.pixelmapSizeGtoG-1))];
		fB = D3DState.ClientPixelStoreState.pixelmapBtoB[(DWORD)(fB *(D3DState.ClientPixelStoreState.pixelmapSizeBtoB-1))];
		fA = D3DState.ClientPixelStoreState.pixelmapAtoA[(DWORD)(fA *(D3DState.ClientPixelStoreState.pixelmapSizeAtoA-1))];
	}

	bgra[2] = static_cast<GLubyte>( QINDIEGL_CLAMP( fR * 255.0f ) );
	bgra[1] = static_cast<GLubyte>( QINDIEGL_CLAMP( fG * 255.0f ) );
	bgra[0] = static_cast<GLubyte>( QINDIEGL_CLAMP( fB * 255.0f ) );
	bgra[3] = static_cast<GLubyte>( QINDIEGL_CLAMP( fA * 255.0f ) );
}

inline void D3DPixels_ModifyABGR( GLubyte *bgra )
{
	float fR = QINDIEGL_MIN( 1.0f, QINDIEGL_MAX( 0.0f,((float)bgra[3] / 255.0f) * D3DState.ClientPixelStoreState.transferRedScale + D3DState.ClientPixelStoreState.transferRedBias ));
	float fG = QINDIEGL_MIN( 1.0f, QINDIEGL_MAX( 0.0f,((float)bgra[2] / 255.0f) * D3DState.ClientPixelStoreState.transferGreenScale + D3DState.ClientPixelStoreState.transferGreenBias ));
	float fB = QINDIEGL_MIN( 1.0f, QINDIEGL_MAX( 0.0f,((float)bgra[1] / 255.0f) * D3DState.ClientPixelStoreState.transferBlueScale + D3DState.ClientPixelStoreState.transferBlueBias ));
	float fA = QINDIEGL_MIN( 1.0f, QINDIEGL_MAX( 0.0f,((float)bgra[0] / 255.0f) * D3DState.ClientPixelStoreState.transferAlphaScale + D3DState.ClientPixelStoreState.transferAlphaBias ));

	if(D3DState.ClientPixelStoreState.transferMapColor) {
		fR = D3DState.ClientPixelStoreState.pixelmapRtoR[(DWORD)(fR *(D3DState.ClientPixelStoreState.pixelmapSizeRtoR-1))];
		fG = D3DState.ClientPixelStoreState.pixelmapGtoG[(DWORD)(fG *(D3DState.ClientPixelStoreState.pixelmapSizeGtoG-1))];
		fB = D3DState.ClientPixelStoreState.pixelmapBtoB[(DWORD)(fB *(D3DState.ClientPixelStoreState.pixelmapSizeBtoB-1))];
		fA = D3DState.ClientPixelStoreState.pixelmapAtoA[(DWORD)(fA *(D3DState.ClientPixelStoreState.pixelmapSizeAtoA-1))];
	}

	bgra[3] = static_cast<GLubyte>( QINDIEGL_CLAMP( fR * 255.0f ) );
	bgra[2] = static_cast<GLubyte>( QINDIEGL_CLAMP( fG * 255.0f ) );
	bgra[1] = static_cast<GLubyte>( QINDIEGL_CLAMP( fB * 255.0f ) );
	bgra[0] = static_cast<GLubyte>( QINDIEGL_CLAMP( fA * 255.0f ) );
}

static void D3DPixels_UnpackPixel( int srcbytes, GLubyte *srcdata, int dstbytes, GLubyte *dstdata, eTexTypeInternal intfmt, DWORD channelMask, DWORD flags )
{
	DWORD bgr = flags & PIXEL_FLAG_BGR;
	DWORD alphaFirst = flags & PIXEL_FLAG_ALPHA_FIRST;

	switch(srcbytes) {
	case 1:
		{
			switch(dstbytes) {
			case 1:
				{
					dstdata[0] = srcdata[0];
					break;
				}
			case 2:
				{
					if(intfmt == D3D_TEXTYPE_X1R5G5B5) {
						GLushort *pixel =(GLushort*)dstdata;
						GLubyte comp =(srcdata[0] * 31) / 255;
						*pixel =(comp << 10) |(comp << 5) | comp;
					} else if(intfmt == D3D_TEXTYPE_A1R5G5B5) {
						GLushort *pixel =(GLushort*)dstdata;
						GLubyte comp =(srcdata[0] * 31) / 255;
						*pixel =(1 << 15) |(comp << 10) |(comp << 5) | comp;
					} else if(intfmt == D3D_TEXTYPE_A4R4G4B4) {
						GLushort *pixel =(GLushort*)dstdata;
						GLubyte comp =(srcdata[0] * 15) / 255;
						*pixel =(15 << 12) |(comp << 8) |(comp << 4) | comp;
					} else if(intfmt == D3D_TEXTYPE_INTENSITY) {
						dstdata[0] = srcdata[0];
						dstdata[1] = srcdata[0];
						D3DPixels_ModifyA( dstdata + 1 );
					} else {
						dstdata[0] = srcdata[0]; 
						dstdata[1] = 0xFF; 
					}
					break;
				}
			case 4:
				{
					if(intfmt == D3D_TEXTYPE_X24A8) {
						dstdata[0] = 255;
						dstdata[1] = 255;
						dstdata[2] = 255;
						dstdata[3] = srcdata[0];
						D3DPixels_ModifyA( dstdata + 3 );
					} else {
						dstdata[0] =(channelMask &(1<<2)) ? srcdata[0] : 0;
						dstdata[1] =(channelMask &(1<<1)) ? srcdata[0] : 0;
						dstdata[2] =(channelMask &(1<<0)) ? srcdata[0] : 0;
						dstdata[3] =(channelMask &(1<<3)) ? srcdata[0] : 255;
						D3DPixels_ModifyBGRA( dstdata );
					}
					break;
				}
			default:
				break;
			}
			break;
		}
	case 2:
		{
			switch(dstbytes) {
			case 1:
				{
					dstdata[0] = srcdata[alphaFirst ? 1 : 0]; 
					break;
				}
			case 2:
				{
					if(intfmt == D3D_TEXTYPE_X1R5G5B5) {
						GLushort *pixel =(GLushort*)dstdata;
						GLubyte comp =(srcdata[alphaFirst ? 1 : 0] * 31) / 255;
						*pixel =(comp << 10) |(comp << 5) | comp;
					} else if(intfmt == D3D_TEXTYPE_A1R5G5B5) {
						GLushort *pixel =(GLushort*)dstdata;
						GLubyte comp =(srcdata[alphaFirst ? 1 : 0] * 31) / 255;
						GLubyte comp2 =(srcdata[alphaFirst ? 0 : 1] > 127) ? 1 : 0;
						D3DPixels_ModifyA( &comp2 );
						*pixel =(comp2 << 15) |(comp << 10) |(comp << 5) | comp;
					} else if(intfmt == D3D_TEXTYPE_A4R4G4B4) {
						GLushort *pixel =(GLushort*)dstdata;
						GLubyte comp =(srcdata[alphaFirst ? 1 : 0] * 15) / 255;
						GLubyte comp2 =(srcdata[alphaFirst ? 0 : 1] * 15) / 255;
						D3DPixels_ModifyA( &comp2 );
						*pixel =(comp2 << 12) |(comp << 8) |(comp << 4) | comp;
					} else {
						dstdata[0] = srcdata[alphaFirst ? 1 : 0];
						dstdata[1] = srcdata[alphaFirst ? 0 : 1];
						D3DPixels_ModifyA( dstdata + 1 );
					}
					break;
				}
			case 4:
				{
					dstdata[0] = srcdata[alphaFirst ? 1 : 0];
					dstdata[1] = srcdata[alphaFirst ? 1 : 0];
					dstdata[2] = srcdata[alphaFirst ? 1 : 0];
					dstdata[3] = srcdata[alphaFirst ? 0 : 1];
					D3DPixels_ModifyBGRA( dstdata );
					break;
				}
			default:
				break;
			}
			break;
		}
	case 3:
	{
		switch(dstbytes) {
		case 1:
			{
				dstdata[0] =((int) srcdata[0] +(int) srcdata[1] +(int) srcdata[2]) / 3;
				break;
			}
		case 2:
			{
				if(intfmt == D3D_TEXTYPE_X1R5G5B5) {
					GLushort *pixel =(GLushort*)dstdata;
					GLubyte comp[3];
					comp[0] =(srcdata[bgr ? 0 : 2] * 31) / 255;
					comp[1] =(srcdata[1] * 31) / 255;
					comp[2] =(srcdata[bgr ? 2 : 0] * 31) / 255;
					D3DPixels_ModifyBGR( comp );
					*pixel =(comp[2] << 10) |(comp[1] << 5) | comp[0];
				} else if(intfmt == D3D_TEXTYPE_A1R5G5B5) {
					GLushort *pixel =(GLushort*)dstdata;
					GLubyte comp[3];
					comp[0] =(srcdata[bgr ? 0 : 2] * 31) / 255;
					comp[1] =(srcdata[1] * 31) / 255;
					comp[2] =(srcdata[bgr ? 2 : 0] * 31) / 255;
					D3DPixels_ModifyBGR( comp );
					*pixel =(1 << 15) |(comp[2] << 10) |(comp[1] << 5) | comp[0];
				} else if(intfmt == D3D_TEXTYPE_A4R4G4B4) {
					GLushort *pixel =(GLushort*)dstdata;
					GLubyte comp[3];
					comp[0] =(srcdata[bgr ? 0 : 2] * 15) / 255;
					comp[1] =(srcdata[1] * 15) / 255;
					comp[2] =(srcdata[bgr ? 2 : 0] * 15) / 255;
					D3DPixels_ModifyBGR( comp );
					*pixel =(15 << 12) |(comp[2] << 8) |(comp[1] << 4) | comp[0];
				} else if(intfmt == D3D_TEXTYPE_INTENSITY) {
					dstdata[0] =((int) srcdata[0] +(int) srcdata[1] +(int) srcdata[2]) / 3;
					dstdata[1] = dstdata[0]; 
					D3DPixels_ModifyA( dstdata + 1 );
				} else {
					dstdata[0] =((int) srcdata[0] +(int) srcdata[1] +(int) srcdata[2]) / 3;
					dstdata[1] = 0xFF; 
				}
				break;
			}
		case 4:
			{
				dstdata[0] = srcdata[bgr ? 0 : 2];
				dstdata[1] = srcdata[1];
				dstdata[2] = srcdata[bgr ? 2 : 0];
				dstdata[3] = 0xFF;
				D3DPixels_ModifyBGRA( dstdata );
				break;
			}
		default:
			break;
		}
		break;
	}
	case 4:
	{
		switch(dstbytes) {
		case 1:
			{
				dstdata[0] =((int) srcdata[alphaFirst ? 1 : 0] +(int) srcdata[alphaFirst ? 2 : 1] +(int) srcdata[alphaFirst ? 3 : 2]) / 3;
				break;
			}
		case 2:
			{
				if(intfmt == D3D_TEXTYPE_X1R5G5B5) {
					GLushort *pixel =(GLushort*)dstdata;
					GLubyte comp[3];
					comp[0] =(srcdata[alphaFirst ? 3 : 2] * 31) / 255;
					comp[1] =(srcdata[alphaFirst ? 2 : 1] * 31) / 255;
					comp[2] =(srcdata[alphaFirst ? 1 : 0] * 31) / 255;
					if(bgr) { comp[0] ^= comp[2] ^= comp[0] ^= comp[2]; }
					D3DPixels_ModifyBGR( comp );
					*pixel =(comp[2] << 10) |(comp[1] << 5) | comp[0];
				} else if(intfmt == D3D_TEXTYPE_A1R5G5B5) {
					GLushort *pixel =(GLushort*)dstdata;
					GLubyte comp[4];
					comp[0] =(srcdata[alphaFirst ? 3 : 2] * 31) / 255;
					comp[1] =(srcdata[alphaFirst ? 2 : 1] * 31) / 255;
					comp[2] =(srcdata[alphaFirst ? 1 : 0] * 31) / 255;
					comp[3] =(srcdata[alphaFirst ? 0 : 3] > 127) ? 1 : 0;
					if(bgr) { comp[0] ^= comp[2] ^= comp[0] ^= comp[2]; }
					D3DPixels_ModifyBGRA( comp );
					*pixel =(comp[3] << 15) |(comp[2] << 10) |(comp[1] << 5) | comp[0];
				} else if(intfmt == D3D_TEXTYPE_A4R4G4B4) {
					GLushort *pixel =(GLushort*)dstdata;
					GLubyte comp[4];
					comp[0] =(srcdata[alphaFirst ? 3 : 2] * 15) / 255;
					comp[1] =(srcdata[alphaFirst ? 2 : 1] * 15) / 255;
					comp[2] =(srcdata[alphaFirst ? 1 : 0] * 15) / 255;
					comp[3] =(srcdata[alphaFirst ? 0 : 3] * 15) / 255;
					if(bgr) { comp[0] ^= comp[2] ^= comp[0] ^= comp[2]; }
					D3DPixels_ModifyBGRA( comp );
					*pixel =(comp[3] << 12) |(comp[2] << 8) |(comp[1] << 4) | comp[0];
				} else {
					dstdata[0] =((int) srcdata[alphaFirst ? 1 : 0] +(int) srcdata[alphaFirst ? 2 : 1] +(int) srcdata[alphaFirst ? 3 : 2]) / 3;
					dstdata[1] = srcdata[alphaFirst ? 0 : 3]; 
				}
				break;
			}
		case 4:
			{
				dstdata[0] = srcdata[alphaFirst ? 3 : 2];
				dstdata[1] = srcdata[alphaFirst ? 2 : 1];
				dstdata[2] = srcdata[alphaFirst ? 1 : 0];
				dstdata[3] = srcdata[alphaFirst ? 0 : 3];
				if(bgr) { dstdata[0] ^= dstdata[2] ^= dstdata[0] ^= dstdata[2]; }
				D3DPixels_ModifyBGRA( dstdata );
				break;
			}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}

#pragma warning( disable: 4100 )	// unreferenced formal parameter

static void D3DPixels_PackPixel( int srcbytes, const GLubyte *srcdata, int dstbytes, GLubyte *dstdata, eTexTypeInternal intfmt, DWORD channelMask, DWORD flags )
{
	//!TODO

	DWORD bgr = flags & PIXEL_FLAG_BGR;
	DWORD alphaFirst = flags & PIXEL_FLAG_ALPHA_FIRST;

	switch(srcbytes) {
	case 1:
		{
			break;
		}
	case 2:
		{
			break;
		}
	case 4:
		{
			switch(dstbytes) {
			case 1:
				{
					dstdata[0] =(srcdata[0] + srcdata[1] + srcdata[2]) / 3;
					break;
				}
			case 2:
				{
					dstdata[alphaFirst ? 1 : 0] =(srcdata[0] + srcdata[1] + srcdata[2]) / 3;
					dstdata[alphaFirst ? 0 : 1] = srcdata[3];
					D3DPixels_ModifyA( dstdata +(alphaFirst ? 0 : 1) );
					break;
				}
			case 3:
				{
					dstdata[0] = srcdata[0];
					dstdata[1] = srcdata[1];
					dstdata[2] = srcdata[2];
					D3DPixels_ModifyBGR( dstdata );
					if(!bgr) { dstdata[0] ^= dstdata[2] ^= dstdata[0] ^= dstdata[2]; }
					break;
				}
			case 4:
				{
					dstdata[0] = srcdata[alphaFirst ? 3 : 0];
					dstdata[1] = srcdata[alphaFirst ? 0 : 1];
					dstdata[2] = srcdata[alphaFirst ? 1 : 2];
					dstdata[3] = srcdata[alphaFirst ? 2 : 3];
					if(alphaFirst) {
						D3DPixels_ModifyABGR( dstdata );
						if(!bgr) { dstdata[1] ^= dstdata[3] ^= dstdata[1] ^= dstdata[3]; }
					} else {
						D3DPixels_ModifyBGRA( dstdata );
						if(!bgr) { dstdata[0] ^= dstdata[2] ^= dstdata[0] ^= dstdata[2]; }
					}
					break;
				}
			default:
				break;
			}
			break;
		}
	default:
		break;
	}
}

static void D3DPixels_PackDepthStencil( int srcbytes, const GLuint *srcdata, int dstbytes, GLuint *dstdata, eTexTypeInternal intfmt, DWORD channelMask, DWORD flags )
{
	//TODO!
	// D3D cannot read depth-stencil surface, so we don't need this?

	// This function packs depthstencil into 1 or 2 GLuints
	// It depends on channelMask
	// If both depth and stencil are specified, depth value is the first and stencil value is the second

	int dstpixel = 0;

	if(channelMask &(1<<0)) {
		//pack depth
		switch(intfmt) {
		case D3D_TEXTYPE_D16:
			{
				assert( srcbytes == 2 );
				GLushort depth = *(GLushort*)srcdata;
				dstdata[dstpixel] =(GLuint)(((GLfloat)depth / USHRT_MAX) * UINT_MAX);
			}
			break;
		case D3D_TEXTYPE_D15S1:
			{
				assert( srcbytes == 2 );
				GLushort depth = *(GLushort*)srcdata >> 1;
				dstdata[dstpixel] =(GLuint)(((GLfloat)depth /(USHRT_MAX >> 1)) * UINT_MAX);
			}
			break;
		case D3D_TEXTYPE_D32:
			{
				assert( srcbytes == 4 );
				dstdata[dstpixel] = *srcdata;
			}
			break;
		case D3D_TEXTYPE_D24S8:
			{
				assert( srcbytes == 4 );
				dstdata[dstpixel] =(GLuint)(((GLfloat)(*srcdata >> 8) /(UINT_MAX >> 8)) * UINT_MAX);
			}
			break;
		default:
			//TODO!
			break;
		}
		++dstpixel;
	}

	if(channelMask &(1<<1)) {
		//pack stencil
		switch(intfmt) {
		case D3D_TEXTYPE_D16:
		case D3D_TEXTYPE_D32:
			{
				dstdata[dstpixel] = 0;
			}
			break;
		case D3D_TEXTYPE_D15S1:
			{
				assert( srcbytes == 2 );
				GLushort stencil = *(GLushort*)srcdata & 1;
				dstdata[dstpixel] = stencil ? UINT_MAX : 0;
			}
			break;
		case D3D_TEXTYPE_D24S8:
			{
				assert( srcbytes == 4 );
				GLubyte stencil = *(GLushort*)srcdata & 0xFF;
				dstdata[dstpixel] =(GLuint)(((GLfloat)stencil / 255) * UINT_MAX);
			}
			break;
		default:
			//TODO!
			break;
		}
	}
}

#pragma warning( default: 4100 )

template<typename T>
static GLubyte D3DPixels_DisassemblePackedPixel( ePixelPackageInternal pack_mode, int comp, T pixel )
{
	DWORD dwPixel =(DWORD)pixel;
	switch(pack_mode) {
	default:
		logPrintf("WARNING: unsupported packed pixel mode %i\n", pack_mode);
		return 0;
	case PP_TYPE_R3_G3_B2:
		switch(comp) {
		case 0:
			return(GLubyte)((((dwPixel >> 5) & 7) * 255) / 7);
		case 1:
			return(GLubyte)((((dwPixel >> 2) & 7) * 255) / 7);
		case 2:
			return(GLubyte)(((dwPixel & 3) * 255) / 3);
		default:
			return 255;
		}
	case PP_TYPE_R4_G4_B4_A4:
		switch(comp) {
		case 0:
			return(GLubyte)((((dwPixel >> 12) & 15) * 255) / 15);
		case 1:
			return(GLubyte)((((dwPixel >> 8) & 15) * 255) / 15);
		case 2:
			return(GLubyte)((((dwPixel >> 4) & 15) * 255) / 15);
		default:
			return(GLubyte)(((dwPixel & 15) * 255) / 15);
		}
	case PP_TYPE_R5_G5_B5_A1:
		switch(comp) {
		case 0:
			return(GLubyte)((((dwPixel >> 11) & 31) * 255) / 31);
		case 1:
			return(GLubyte)((((dwPixel >> 6) & 31) * 255) / 31);
		case 2:
			return(GLubyte)((((dwPixel >> 1) & 31) * 255) / 31);
		default:
			return((dwPixel & 1) ? 255 : 0);
		}
	case PP_TYPE_R8_G8_B8_A8:
		switch(comp) {
		case 0:
			return(GLubyte)((dwPixel >> 24) & 255);
		case 1:
			return(GLubyte)((dwPixel >> 16) & 255);
		case 2:
			return(GLubyte)((dwPixel >> 8) & 255);
		default:
			return(GLubyte)(dwPixel & 255);
		}
	case PP_TYPE_R10_G10_B10_A2:
		switch(comp) {
		case 0:
			return(GLubyte)(((dwPixel >> 22) & 1023) >> 2);
		case 1:
			return(GLubyte)(((dwPixel >> 12) & 1023) >> 2);
		case 2:
			return(GLubyte)(((dwPixel >> 2) & 1023) >> 2);
		default:
			return(GLubyte)(((dwPixel & 3) * 255) / 3);
		}
	}
}

template<typename T>
static T D3DPixels_AssemblePackedPixel( ePixelPackageInternal pack_mode, int num_comp, GLubyte *pixel )
{
	DWORD dwPixel =(DWORD)pixel;
	switch(pack_mode) {
	default:
		logPrintf("WARNING: unsupported packed pixel mode %i\n", pack_mode);
		return 0;
	case PP_TYPE_R3_G3_B2:
		switch(num_comp) {
		default:
		case 1:
		case 2:
			dwPixel =(((pixel[0] * 7 / 255) & 7) << 5) |
					 (((pixel[0] * 7 / 255) & 7) << 2) |
					 (((pixel[0] * 3 / 255) & 3) << 0);
			break;
		case 3:
		case 4:
			dwPixel =(((pixel[0] * 7 / 255) & 7) << 5) |
					 (((pixel[1] * 7 / 255) & 7) << 2) |
					 (((pixel[2] * 3 / 255) & 3) << 0);
			break;
		}
		break;
	case PP_TYPE_R4_G4_B4_A4:
		switch(num_comp) {
		default:
		case 1:
			dwPixel =(((pixel[0] * 15 / 255) & 15) << 12) |
					 (((pixel[0] * 15 / 255) & 15) << 8) |
					 (((pixel[0] * 15 / 255) & 15) << 4) |
					   15;
			break;
		case 2:
			dwPixel =(((pixel[0] * 15 / 255) & 15) << 12) |
					 (((pixel[0] * 15 / 255) & 15) << 8) |
					 (((pixel[0] * 15 / 255) & 15) << 4) |
					 (((pixel[1] * 15 / 255) & 15) << 0);
			break;
		case 3:
			dwPixel =(((pixel[0] * 15 / 255) & 15) << 12) |
					 (((pixel[1] * 15 / 255) & 15) << 8) |
					 (((pixel[2] * 15 / 255) & 15) << 4) |
					   15;
			break;
		case 4:
			dwPixel =(((pixel[0] * 15 / 255) & 15) << 12) |
					 (((pixel[1] * 15 / 255) & 15) << 8) |
					 (((pixel[2] * 15 / 255) & 15) << 4) |
					 (((pixel[3] * 15 / 255) & 15) << 0);
			break;
		}
		break;
	case PP_TYPE_R5_G5_B5_A1:
		switch(num_comp) {
		default:
		case 1:
			dwPixel =(((pixel[0] * 31 / 255) & 31) << 11) |
					 (((pixel[0] * 31 / 255) & 31) << 6) |
					 (((pixel[0] * 31 / 255) & 31) << 1) |
					   1;
			break;
		case 2:
			dwPixel =(((pixel[0] * 31 / 255) & 31) << 11) |
					 (((pixel[0] * 31 / 255) & 31) << 6) |
					 (((pixel[0] * 31 / 255) & 31) << 1) |
					  (pixel[1] > 0 ? 1 : 0);
			break;
		case 3:
			dwPixel =(((pixel[0] * 31 / 255) & 31) << 11) |
					 (((pixel[1] * 31 / 255) & 31) << 6) |
					 (((pixel[2] * 31 / 255) & 31) << 1) |
					   1;
			break;
		case 4:
			dwPixel =(((pixel[0] * 31 / 255) & 31) << 11) |
					 (((pixel[1] * 31 / 255) & 31) << 6) |
					 (((pixel[2] * 31 / 255) & 31) << 1) |
					  (pixel[3] > 0 ? 1 : 0);
			break;
		}
		break;
	case PP_TYPE_R8_G8_B8_A8:
		switch(num_comp) {
		default:
		case 1:
			dwPixel =(pixel[0] << 24) |
					 (pixel[0] << 16) |
					 (pixel[0] << 8) |
					  255;
			break;
		case 2:
			dwPixel =(pixel[0] << 24) |
					 (pixel[0] << 16) |
					 (pixel[0] << 8) |
					  pixel[1];
			break;
		case 3:
			dwPixel =(pixel[0] << 24) |
					 (pixel[1] << 16) |
					 (pixel[2] << 8) |
					  255;
			break;
		case 4:
			dwPixel =(pixel[0] << 24) |
					 (pixel[1] << 16) |
					 (pixel[2] << 8) |
					  pixel[3];
			break;
		}
		break;
	case PP_TYPE_R10_G10_B10_A2:
		switch(num_comp) {
		default:
		case 1:
			dwPixel =(((pixel[0] * 1023 / 255) & 1023) << 22) |
					 (((pixel[0] * 1023 / 255) & 1023) << 12) |
					 (((pixel[0] * 1023 / 255) & 1023) << 2) |
					   3;
			break;
		case 2:
			dwPixel =(((pixel[0] * 1023 / 255) & 1023) << 22) |
					 (((pixel[0] * 1023 / 255) & 1023) << 12) |
					 (((pixel[0] * 1023 / 255) & 1023) << 2) |
					 (((pixel[1] * 3 / 255) & 3) << 0);
			break;
		case 3:
			dwPixel =(((pixel[0] * 1023 / 255) & 1023) << 22) |
					 (((pixel[1] * 1023 / 255) & 1023) << 12) |
					 (((pixel[2] * 1023 / 255) & 1023) << 2) |
					  3;
			break;
		case 4:
			dwPixel =(((pixel[0] * 1023 / 255) & 1023) << 22) |
					 (((pixel[1] * 1023 / 255) & 1023) << 12) |
					 (((pixel[2] * 1023 / 255) & 1023) << 2) |
					 (((pixel[3] * 3 / 255) & 3) << 0);
			break;
		}
		break;
	}
	return(T)dwPixel;
}

#pragma warning( disable: 4127 ) // conditional expression is constant
#pragma warning( disable: 4244 ) // conversion, possible loss of data

template<typename T> 
static void D3DPixels_UnpackInternal( ePixelPackageInternal pack_mode, int width, int height, int depth, int hpitch, int vpitch, GLubyte *dstbytes, int dstpixelsize, eTexTypeInternal intfmt, DWORD channelMask, DWORD flags, bool flipVertical, int fmtpixelsize, int realpixelsize, const T *pixels )
{
	GLubyte ubpixel[4];

	int row_length = width*realpixelsize;
	if(D3DState.ClientPixelStoreState.unpackRowLength > 0)
		row_length = D3DState.ClientPixelStoreState.unpackRowLength*realpixelsize;
	if(D3DState.ClientPixelStoreState.unpackAlignment > 0) {
		int alignment = D3DState.ClientPixelStoreState.unpackAlignment - 1;
		row_length =(row_length + alignment) & ~alignment;
	}

	int image_height = height*width*realpixelsize;
	if(D3DState.ClientPixelStoreState.unpackImageHeight > 0)
		image_height = D3DState.ClientPixelStoreState.unpackImageHeight*width*realpixelsize;
	if(sizeof(pixels[0]) < D3DState.ClientPixelStoreState.unpackAlignment)
		image_height =(int)(ceil((float)image_height*sizeof(pixels[0])/(float)D3DState.ClientPixelStoreState.unpackAlignment) * D3DState.ClientPixelStoreState.unpackAlignment / sizeof(pixels[0]));

	const T *in = pixels + D3DState.ClientPixelStoreState.unpackSkipPixels * realpixelsize;
	in += D3DState.ClientPixelStoreState.unpackSkipRows * row_length;
	in += D3DState.ClientPixelStoreState.unpackSkipImages * image_height;
	
	GLubyte *sliceptr = dstbytes;
	for( int i = 0; i < depth; ++i ) {
		GLubyte *rowptr = sliceptr;
		for( int j = 0; j < height; ++j ) {
			for( int k = 0; k < width; ++k ) {
				const T *ppixel = in + i*image_height +(flipVertical ?(height - j - 1) : j)*row_length + k*realpixelsize;

				if(pack_mode == PP_TYPE_UNPACKED) {
					for( int l = 0; l < fmtpixelsize; ++l ) {
						T pixeldata = ppixel[l];
						if(D3DState.ClientPixelStoreState.unpackSwapBytes) {
							if(sizeof(T) == 4)
								pixeldata = static_cast<T>( D3D_ByteSwap32(pixeldata) );
							else if(sizeof(T) == 2)
								pixeldata = static_cast<T>( D3D_ByteSwap16(pixeldata) );
						}
						ubpixel[l] = static_cast<GLubyte>( QINDIEGL_CLAMP(((GLfloat)pixeldata / std::numeric_limits<T>::max()) * 255) );
					}
				} else {
					T pixeldata = *ppixel;
					if(D3DState.ClientPixelStoreState.unpackSwapBytes) {
						if(sizeof(T) == 4)
						pixeldata = static_cast<T>( D3D_ByteSwap32(pixeldata) );
					else if(sizeof(T) == 2)
						pixeldata = static_cast<T>( D3D_ByteSwap16(pixeldata) );
					}
					for( int l = 0; l < fmtpixelsize; ++l ) {
						ubpixel[l] = D3DPixels_DisassemblePackedPixel<T>( pack_mode, l, pixeldata );
					}
				}
				D3DPixels_UnpackPixel( fmtpixelsize, ubpixel, dstpixelsize, rowptr + k*dstpixelsize, intfmt, channelMask, flags );
			}
			rowptr += hpitch;
		}
		sliceptr += vpitch;
	}
}

static void D3DPixels_UnpackInternal( int width, int height, int depth, int hpitch, int vpitch, GLubyte *dstbytes, int dstpixelsize, eTexTypeInternal intfmt, DWORD channelMask, DWORD flags, bool flipVertical, int pixelsize, const GLfloat *pixels )
{
	GLubyte ubpixel[4];

	int row_length = width*pixelsize;
	if(D3DState.ClientPixelStoreState.unpackRowLength > 0)
		row_length = D3DState.ClientPixelStoreState.unpackRowLength*pixelsize;
	if(D3DState.ClientPixelStoreState.unpackAlignment > 0) {
		int alignment = D3DState.ClientPixelStoreState.unpackAlignment - 1;
		row_length =(row_length + alignment) & ~alignment;
	}

	int image_height = height*width*pixelsize;
	if(D3DState.ClientPixelStoreState.unpackImageHeight > 0)
		image_height = D3DState.ClientPixelStoreState.unpackImageHeight*width*pixelsize;
	if(D3DState.ClientPixelStoreState.unpackAlignment > 0) {
		int alignment = D3DState.ClientPixelStoreState.unpackAlignment - 1;
		image_height =(image_height + alignment) & ~alignment;
	}

	const GLfloat *in = pixels + D3DState.ClientPixelStoreState.unpackSkipPixels * pixelsize;
	in += D3DState.ClientPixelStoreState.unpackSkipRows * row_length;
	in += D3DState.ClientPixelStoreState.unpackSkipImages * image_height;

	GLubyte *sliceptr = dstbytes;
	for( int i = 0; i < depth; ++i ) {
		GLubyte *rowptr = sliceptr;
		for( int j = 0; j < height; ++j ) {
			for( int k = 0; k < width; ++k ) {
				const GLfloat *ppixel = in + i*image_height +(flipVertical ?(height - j - 1) : j)*row_length + k*pixelsize;
				for( int l = 0; l < pixelsize; ++l ) {
					ubpixel[l] = static_cast<GLubyte>( QINDIEGL_CLAMP(ppixel[l] * 255) );
				}
				D3DPixels_UnpackPixel( pixelsize, ubpixel, dstpixelsize, rowptr + k*dstpixelsize, intfmt, channelMask, flags );
			}
			rowptr += hpitch;
		}
		sliceptr += vpitch;
	}
}

template<typename T> 
static void D3DPixels_PackInternal( ePixelPackageInternal pack_mode, int width, int height, int depth, int hpitch, int vpitch, const GLubyte *srcbytes, int srcpixelsize, eTexTypeInternal intfmt, DWORD channelMask, DWORD flags, bool flipVertical, int fmtpixelsize, int realpixelsize, T *pixels )
{
	GLubyte ubpixel[4];
	GLuint uipixel[2];

	int row_length = width*realpixelsize;
	if(D3DState.ClientPixelStoreState.packRowLength > 0)
		row_length = D3DState.ClientPixelStoreState.packRowLength*realpixelsize;
	if(D3DState.ClientPixelStoreState.packAlignment > 0) {
		int alignment = D3DState.ClientPixelStoreState.packAlignment - 1;
		row_length =(row_length + alignment) & ~alignment;
	}

	int image_height = height*width*realpixelsize;
	if(D3DState.ClientPixelStoreState.packImageHeight > 0)
		image_height = D3DState.ClientPixelStoreState.packImageHeight*width*realpixelsize;
	if(D3DState.ClientPixelStoreState.packAlignment > 0) {
		int alignment = D3DState.ClientPixelStoreState.packAlignment - 1;
		image_height =(image_height + alignment) & ~alignment;
	}

	T *out = pixels + D3DState.ClientPixelStoreState.packSkipPixels * realpixelsize;
	out += D3DState.ClientPixelStoreState.packSkipRows * row_length;
	out += D3DState.ClientPixelStoreState.packSkipImages * image_height;

	const GLubyte *sliceptr = srcbytes;
	for( int i = 0; i < depth; ++i ) {
		const GLubyte *rowptr = sliceptr;
		for( int j = 0; j < height; ++j ) {
			for( int k = 0; k < width; ++k ) {
				T *ppixel = out + i*image_height +(flipVertical ?(height - j - 1) : j)*row_length + k*realpixelsize;
				if(intfmt & D3D_TEXTYPE_DEPTHSTENCIL_MASK) {
					D3DPixels_PackDepthStencil( srcpixelsize,(GLuint*)(rowptr + k*srcpixelsize), fmtpixelsize, uipixel, intfmt, channelMask, flags );
					for( int l = 0; l < fmtpixelsize; ++l ) {
						ppixel[l] =(T)(uipixel[l] /(GLfloat)std::numeric_limits<T>::max());
					}
				} else {
					D3DPixels_PackPixel( srcpixelsize, rowptr + k*srcpixelsize, fmtpixelsize, ubpixel, intfmt, channelMask, flags );
					if(pack_mode == PP_TYPE_UNPACKED) {
						for( int l = 0; l < fmtpixelsize; ++l ) {
							ppixel[l] =(T)(((float)ubpixel[l] / 255.0f) * std::numeric_limits<T>::max());
							if(D3DState.ClientPixelStoreState.packSwapBytes) {
								if(sizeof(T) == 4)
									ppixel[l] = static_cast<T>( D3D_ByteSwap32(ppixel[l]) );
								else if(sizeof(T) == 2)
									ppixel[l] = static_cast<T>( D3D_ByteSwap16(ppixel[l]) );
							}
						}
					} else {
						*ppixel = D3DPixels_AssemblePackedPixel<T>( pack_mode, fmtpixelsize, ubpixel );
						if(D3DState.ClientPixelStoreState.packSwapBytes) {
							if(sizeof(T) == 4)
								*ppixel = static_cast<T>( D3D_ByteSwap32(*ppixel) );
							else if(sizeof(T) == 2)
								*ppixel = static_cast<T>( D3D_ByteSwap16(*ppixel) );
						}
					}
				}
			}
			rowptr += hpitch;
		}
		sliceptr += vpitch;
	}
}

#pragma warning( default: 4127 )
#pragma warning( default: 4244 )

static void D3DPixels_PackInternal( int width, int height, int depth, int hpitch, int vpitch, const GLubyte *srcbytes, int srcpixelsize, eTexTypeInternal intfmt, DWORD channelMask, DWORD flags, bool flipVertical, int fmtpixelsize, int realpixelsize, GLfloat *pixels )
{
	GLubyte ubpixel[4];
	GLuint uipixel[2];

	int row_length = width*realpixelsize;
	if(D3DState.ClientPixelStoreState.packRowLength > 0)
		row_length = D3DState.ClientPixelStoreState.packRowLength*realpixelsize;
	if(D3DState.ClientPixelStoreState.packAlignment > 0) {
		int alignment = D3DState.ClientPixelStoreState.packAlignment - 1;
		row_length =(row_length + alignment) & ~alignment;
	}

	int image_height = height*width*realpixelsize;
	if(D3DState.ClientPixelStoreState.packImageHeight > 0)
		image_height = D3DState.ClientPixelStoreState.packImageHeight*width*realpixelsize;
	if(D3DState.ClientPixelStoreState.packAlignment > 0) {
		int alignment = D3DState.ClientPixelStoreState.packAlignment - 1;
		image_height =(image_height + alignment) & ~alignment;
	}

	GLfloat *out = pixels + D3DState.ClientPixelStoreState.packSkipPixels * realpixelsize;
	out += D3DState.ClientPixelStoreState.packSkipRows * row_length;
	out += D3DState.ClientPixelStoreState.packSkipImages * image_height;

	const GLubyte *sliceptr = srcbytes;
	for( int i = 0; i < depth; ++i ) {
		const GLubyte *rowptr = sliceptr;
		for( int j = 0; j < height; ++j ) {
			for( int k = 0; k < width; ++k ) {
				GLfloat *ppixel = out + i*image_height +(flipVertical ?(height - j - 1) : j)*row_length + k*realpixelsize;
				if(intfmt & D3D_TEXTYPE_DEPTHSTENCIL_MASK) {
					D3DPixels_PackDepthStencil( srcpixelsize,(GLuint*)(rowptr + k*srcpixelsize), fmtpixelsize, uipixel, intfmt, channelMask, flags );
					for( int l = 0; l < fmtpixelsize; ++l ) {
						ppixel[l] = uipixel[l] /(GLfloat)UINT_MAX;
					}
				} else {
					D3DPixels_PackPixel( srcpixelsize, rowptr + k*srcpixelsize, fmtpixelsize, ubpixel, intfmt, channelMask, flags );
					for( int l = 0; l < fmtpixelsize; ++l ) {
						ppixel[l] = ubpixel[l] / 255.0f;
					}
				}
			}
			rowptr += hpitch;
		}
		sliceptr += vpitch;
	}
}

HRESULT D3DPixels_GetPixelInfo( GLenum format, int &pixelSize, DWORD &channelMask, DWORD &flags )
{
	channelMask = 0xf;
	flags = 0;

	switch(format) {
	case GL_RED:
		channelMask = (1 << 0);
		pixelSize = 1;
		break;
	case GL_GREEN:
		channelMask = (1 << 1);
		pixelSize = 1;
		break;
	case GL_BLUE:
		channelMask = (1 << 2);
		pixelSize = 1;
		break;
	case GL_ALPHA:
		channelMask = (1 << 3);
		pixelSize = 1;
		break;
	case GL_LUMINANCE:
	case GL_COLOR_INDEX:
		channelMask = 0x7;	//exclude alpha channel
		pixelSize = 1;
		break;
	case GL_INTENSITY:
		pixelSize = 1;
		break;
	case GL_LUMINANCE_ALPHA:
		pixelSize = 2;
		break;
	case GL_RGB:
		pixelSize = 3;
		break;
	case GL_BGR_EXT:
		flags |= PIXEL_FLAG_BGR;
		pixelSize = 3;
		break;
	case GL_RGBA:
		pixelSize = 4;
		break;
	case GL_BGRA_EXT:
		flags |= PIXEL_FLAG_BGR;
		pixelSize = 4;
		break;
	case GL_ABGR_EXT:
		flags |=(PIXEL_FLAG_BGR|PIXEL_FLAG_ALPHA_FIRST);
		pixelSize = 4;
		break;
	case GL_DEPTH_COMPONENT:
		channelMask = (1 << 0);
		pixelSize = 1;
		break;
	case GL_STENCIL_INDEX:
		channelMask = (1 << 1);
		pixelSize = 1;
		break;
	default:
		logPrintf("WARNING: Texture format 0x%x is not supported\n", format);
		return E_INVALIDARG;
	}

	return S_OK;
}

HRESULT D3DPixels_Unpack( int width, int height, int depth, int hpitch, int vpitch, GLubyte *dstbytes, int dstpixelsize, eTexTypeInternal intfmt, bool flipVertical, GLenum format, GLenum type, const GLvoid *pixels )
{
	DWORD flags = 0;
	int srcPixelSize;
	DWORD channelMask;

	HRESULT hr = D3DPixels_GetPixelInfo( format, srcPixelSize, channelMask, flags );
	if(FAILED(hr))
		return hr;

	switch(type) {
	case GL_UNSIGNED_BYTE:
		D3DPixels_UnpackInternal<GLubyte>( PP_TYPE_UNPACKED, width, height, depth, hpitch, vpitch, dstbytes, dstpixelsize, intfmt, channelMask, flags, flipVertical, srcPixelSize, srcPixelSize,(const GLubyte*)pixels);
		break;
	case GL_BYTE:
		D3DPixels_UnpackInternal<GLbyte>( PP_TYPE_UNPACKED, width, height, depth, hpitch, vpitch, dstbytes, dstpixelsize, intfmt, channelMask, flags, flipVertical, srcPixelSize, srcPixelSize,(const GLbyte*)pixels);
		break;
	case GL_UNSIGNED_SHORT:
		D3DPixels_UnpackInternal<GLushort>( PP_TYPE_UNPACKED, width, height, depth, hpitch, vpitch, dstbytes, dstpixelsize, intfmt, channelMask, flags, flipVertical, srcPixelSize, srcPixelSize,(const GLushort*)pixels);
		break;
	case GL_SHORT:
		D3DPixels_UnpackInternal<GLshort>( PP_TYPE_UNPACKED, width, height, depth, hpitch, vpitch, dstbytes, dstpixelsize, intfmt, channelMask, flags, flipVertical, srcPixelSize, srcPixelSize,(const GLshort*)pixels);
		break;
	case GL_UNSIGNED_INT:
		D3DPixels_UnpackInternal<GLuint>( PP_TYPE_UNPACKED, width, height, depth, hpitch, vpitch, dstbytes, dstpixelsize, intfmt, channelMask, flags, flipVertical, srcPixelSize, srcPixelSize,(const GLuint*)pixels);
		break;
	case GL_INT:
		D3DPixels_UnpackInternal<GLint>( PP_TYPE_UNPACKED, width, height, depth, hpitch, vpitch, dstbytes, dstpixelsize, intfmt, channelMask, flags, flipVertical, srcPixelSize, srcPixelSize,(const GLint*)pixels);
		break;
	case GL_FLOAT:
		D3DPixels_UnpackInternal( width, height, depth, hpitch, vpitch, dstbytes, dstpixelsize, intfmt, channelMask, flags, flipVertical, srcPixelSize,(const GLfloat*)pixels);
		break;
	case GL_UNSIGNED_BYTE_3_3_2_EXT:
		D3DPixels_UnpackInternal<GLubyte>( PP_TYPE_R3_G3_B2, width, height, depth, hpitch, vpitch, dstbytes, dstpixelsize, intfmt, channelMask, flags, flipVertical, srcPixelSize, 1,(const GLubyte*)pixels);
		break;
	case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
		D3DPixels_UnpackInternal<GLushort>( PP_TYPE_R5_G5_B5_A1, width, height, depth, hpitch, vpitch, dstbytes, dstpixelsize, intfmt, channelMask, flags, flipVertical, srcPixelSize, 1,(const GLushort*)pixels);
		break;
	case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
		D3DPixels_UnpackInternal<GLushort>( PP_TYPE_R4_G4_B4_A4, width, height, depth, hpitch, vpitch, dstbytes, dstpixelsize, intfmt, channelMask, flags, flipVertical, srcPixelSize, 1,(const GLushort*)pixels);
		break;
	case GL_UNSIGNED_INT_8_8_8_8_EXT:
		D3DPixels_UnpackInternal<GLuint>( PP_TYPE_R8_G8_B8_A8, width, height, depth, hpitch, vpitch, dstbytes, dstpixelsize, intfmt, channelMask, flags, flipVertical, srcPixelSize, 1,(const GLuint*)pixels);
		break;
	case GL_UNSIGNED_INT_10_10_10_2_EXT:
		D3DPixels_UnpackInternal<GLuint>( PP_TYPE_R10_G10_B10_A2, width, height, depth, hpitch, vpitch, dstbytes, dstpixelsize, intfmt, channelMask, flags, flipVertical, srcPixelSize, 1,(const GLuint*)pixels);
		break;
	default:
		logPrintf("WARNING: Texture data type 0x%x is not supported\n", type);
		return E_INVALIDARG;
	}

	return S_OK;
}

HRESULT D3DPixels_Pack( int width, int height, int depth, int hpitch, int vpitch, const GLubyte *srcbytes, int srcpixelsize, eTexTypeInternal intfmt, bool flipVertical, GLenum format, GLenum type, GLvoid *pixels )
{
	DWORD flags = 0;
	int dstPixelSize;
	DWORD channelMask;

	HRESULT hr = D3DPixels_GetPixelInfo( format, dstPixelSize, channelMask, flags );
	if(FAILED(hr))
		return hr;

	switch(type) {
	case GL_UNSIGNED_BYTE:
		D3DPixels_PackInternal<GLubyte>( PP_TYPE_UNPACKED, width, height, depth, hpitch, vpitch, srcbytes, srcpixelsize, intfmt, channelMask, flags, flipVertical, dstPixelSize, dstPixelSize,(GLubyte*)pixels);
		break;
	case GL_BYTE:
		D3DPixels_PackInternal<GLbyte>( PP_TYPE_UNPACKED, width, height, depth, hpitch, vpitch, srcbytes, srcpixelsize, intfmt, channelMask, flags, flipVertical, dstPixelSize, dstPixelSize,(GLbyte*)pixels);
		break;
	case GL_UNSIGNED_SHORT:
		D3DPixels_PackInternal<GLushort>( PP_TYPE_UNPACKED, width, height, depth, hpitch, vpitch, srcbytes, srcpixelsize, intfmt, channelMask, flags, flipVertical, dstPixelSize, dstPixelSize,(GLushort*)pixels);
		break;
	case GL_SHORT:
		D3DPixels_PackInternal<GLshort>( PP_TYPE_UNPACKED, width, height, depth, hpitch, vpitch, srcbytes, srcpixelsize, intfmt, channelMask, flags, flipVertical, dstPixelSize, dstPixelSize,(GLshort*)pixels);
		break;
	case GL_UNSIGNED_INT:
		D3DPixels_PackInternal<GLuint>( PP_TYPE_UNPACKED, width, height, depth, hpitch, vpitch, srcbytes, srcpixelsize, intfmt, channelMask, flags, flipVertical, dstPixelSize, dstPixelSize,(GLuint*)pixels);
		break;
	case GL_INT:
		D3DPixels_PackInternal<GLint>( PP_TYPE_UNPACKED, width, height, depth, hpitch, vpitch, srcbytes, srcpixelsize, intfmt, channelMask, flags, flipVertical, dstPixelSize, dstPixelSize,(GLint*)pixels);
		break;
	case GL_FLOAT:
		D3DPixels_PackInternal( width, height, depth, hpitch, vpitch, srcbytes, srcpixelsize, intfmt, channelMask, flags, flipVertical, dstPixelSize, dstPixelSize,(GLfloat*)pixels);
		break;
	case GL_UNSIGNED_BYTE_3_3_2_EXT:
		D3DPixels_PackInternal<GLubyte>( PP_TYPE_R3_G3_B2, width, height, depth, hpitch, vpitch, srcbytes, srcpixelsize, intfmt, channelMask, flags, flipVertical, dstPixelSize, 1,(GLubyte*)pixels);
		break;
	case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
		D3DPixels_PackInternal<GLushort>( PP_TYPE_R5_G5_B5_A1, width, height, depth, hpitch, vpitch, srcbytes, srcpixelsize, intfmt, channelMask, flags, flipVertical, dstPixelSize, 1,(GLushort*)pixels);
		break;
	case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
		D3DPixels_PackInternal<GLushort>( PP_TYPE_R4_G4_B4_A4, width, height, depth, hpitch, vpitch, srcbytes, srcpixelsize, intfmt, channelMask, flags, flipVertical, dstPixelSize, 1,(GLushort*)pixels);
		break;
	case GL_UNSIGNED_INT_8_8_8_8_EXT:
		D3DPixels_PackInternal<GLuint>( PP_TYPE_R8_G8_B8_A8, width, height, depth, hpitch, vpitch, srcbytes, srcpixelsize, intfmt, channelMask, flags, flipVertical, dstPixelSize, 1,(GLuint*)pixels);
		break;
	case GL_UNSIGNED_INT_10_10_10_2_EXT:
		D3DPixels_PackInternal<GLuint>( PP_TYPE_R10_G10_B10_A2, width, height, depth, hpitch, vpitch, srcbytes, srcpixelsize, intfmt, channelMask, flags, flipVertical, dstPixelSize, 1,(GLuint*)pixels);
		break;
	default:
		logPrintf("WARNING: Texture data type 0x%x is not supported\n", type);
		return E_INVALIDARG;
	}

	return S_OK;
}

//=================================

OPENGL_API void WINAPI glReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels )
{
	if(!D3DGlobal.pDevice) {
		D3DGlobal.lastError = E_FAIL;
		return;
	}

	HRESULT hr;
	LPDIRECT3DSURFACE9 lpRenderTarget = nullptr;

	if( format == GL_DEPTH_COMPONENT || format == GL_STENCIL_INDEX ) 
	{
		static bool msgPrinted = false;
		if(!msgPrinted) {
			logPrintf("WARNING: glReadPixels: reading depth-stencil is not supported\n");
			msgPrinted = true;
		}
		for(int i = 0; i < height; ++i) {
			for(int j = 0; j < width; ++j) {
				switch(type) {
				case GL_UNSIGNED_BYTE:
					*((GLubyte*)pixels + i*height + j) = 0;
					break;
				case GL_BYTE:
					*((GLbyte*)pixels + i*height + j) = 0;
					break;
				case GL_UNSIGNED_SHORT:
					*((GLushort*)pixels + i*height + j) = 0;
					break;
				case GL_SHORT:
					*((GLshort*)pixels + i*height + j) = 0;
					break;
				case GL_UNSIGNED_INT:
					*((GLuint*)pixels + i*height + j) = 0;
					break;
				case GL_INT:
					*((GLint*)pixels + i*height + j) = 0;
					break;
				case GL_FLOAT:
					*((GLfloat*)pixels + i*height + j) = 0;
					break;
				default:
					logPrintf("WARNING: Texture data type 0x%x is not supported\n", type);
					D3DGlobal.lastError = E_INVALIDARG;
					break;
				}
			}
		}
		return;
	/*	D3DSURFACE_DESC desc;
		LPDIRECT3DSURFACE9 lpDS;
		LPDIRECT3DSURFACE9 lpDS2;

		hr = D3DGlobal.pDevice->GetDepthStencilSurface( &lpRenderTarget );
		if(FAILED(hr)) {
			D3DGlobal.lastError = hr;
			logPrintf("WARNING: glReadPixels: GetDepthStencilSurface failed with error '%s'\n", DXGetErrorString9(hr));
			return;
		} 
		hr = lpRenderTarget->GetDesc(&desc);
		if(FAILED(hr)) {
			D3DGlobal.lastError = hr;
			logPrintf("WARNING: glReadPixels: GetDesc failed with error '%s'\n", DXGetErrorString9(hr));
			return;
		}

		hr = D3DGlobal.pDevice->CreateDepthStencilSurface( desc.Width, 
														desc.Height, 
														desc.Format, 
														D3DMULTISAMPLE_NONE, 
														0, FALSE, 
														&lpDS, nullptr );
		if(FAILED(hr)) {
			D3DGlobal.lastError = hr;
			logPrintf("WARNING: glReadPixels: CreateDepthStencilSurface failed with error '%s'\n", DXGetErrorString9(hr));
			return;
		}

		hr = D3DGlobal.pDevice->StretchRect( lpRenderTarget, nullptr, lpDS, nullptr, D3DTEXF_NONE );
		if(FAILED(hr)) {
			D3DGlobal.lastError = hr;
			logPrintf("WARNING: glReadPixels: StretchRect failed with error '%s'\n", DXGetErrorString9(hr));
			return;
		}

		lpRenderTarget->Release();

		//create offscreen surface that will hold an antialiased screen image
		hr = D3DGlobal.pDevice->CreateOffscreenPlainSurface( desc.Width, 
															 desc.Height, 
															 D3DFMT_D16_LOCKABLE, 
															 D3DPOOL_SYSTEMMEM, 
															 &lpDS2, nullptr );
		if(FAILED(hr)) {
			D3DGlobal.lastError = hr;
			logPrintf("WARNING: glReadPixels: CreateOffscreenPlainSurface failed with error '%s'\n", DXGetErrorString9(hr));
			return;
		}

		hr = D3DGlobal.pDevice->GetRenderTargetData( lpDS, lpDS2 );
		if(FAILED(hr)) {
			D3DGlobal.lastError = hr;
			logPrintf("WARNING: glReadPixels: GetRenderTargetData failed with error '%s'\n", DXGetErrorString9(hr));
			return;
		}

		lpDS->Release();
		lpDS2->Release();

		logPrintf("WARNING: glReadPixels: reading depth will be ok\n");
		return;*/
	} else {
		D3DSURFACE_DESC desc;

		hr = D3DGlobal.pDevice->GetRenderTarget( 0, &lpRenderTarget );
		if(FAILED(hr)) {
			D3DGlobal.lastError = hr;
			logPrintf("WARNING: glReadPixels: GetRenderTarget failed with error '%s'\n", DXGetErrorString9(hr));
			return;
		} 

		if(!D3DGlobal.pSystemMemRT) 
		{
			// because we don't have a lockable backbuffer we instead copy it off to an image surface
			// this will also handle translation between different backbuffer formats
			hr = lpRenderTarget->GetDesc(&desc);
			if(FAILED(hr)) {
				D3DGlobal.lastError = hr;
				logPrintf("WARNING: glReadPixels: GetDesc failed with error '%s'\n", DXGetErrorString9(hr));
				return;
			}

			//create offscreen surface that will hold an antialiased screen image
			hr = D3DGlobal.pDevice->CreateOffscreenPlainSurface( desc.Width, 
																 desc.Height, 
																 desc.Format, 
																 D3DPOOL_SYSTEMMEM, 
																 &D3DGlobal.pSystemMemRT, nullptr );
			if(FAILED(hr)) {
				D3DGlobal.lastError = hr;
				logPrintf("WARNING: glReadPixels: CreateOffscreenPlainSurface failed with error '%s'\n", DXGetErrorString9(hr));
				return;
			}

			if(desc.MultiSampleType != D3DMULTISAMPLE_NONE) {
				if(!D3DGlobal.pSystemMemFB) {
					hr = D3DGlobal.pDevice->CreateRenderTarget( desc.Width, 
																desc.Height, 
																desc.Format, 
																D3DMULTISAMPLE_NONE, 
																0, FALSE, 
																&D3DGlobal.pSystemMemFB, nullptr );
					if(FAILED(hr)) {
						D3DGlobal.lastError = hr;
						logPrintf("WARNING: glReadPixels: CreateRenderTarget failed with error '%s'\n", DXGetErrorString9(hr));
						return;
					}
				}
			}
		}

		if(D3DGlobal.pSystemMemFB) {
			hr = D3DGlobal.pDevice->StretchRect( lpRenderTarget, nullptr, D3DGlobal.pSystemMemFB, nullptr, D3DTEXF_NONE );
			if(FAILED(hr)) {
				D3DGlobal.lastError = hr;
				logPrintf("WARNING: glReadPixels: StretchRect failed with error '%s'\n", DXGetErrorString9(hr));
				return;
			}
			lpRenderTarget->Release();
			lpRenderTarget = D3DGlobal.pSystemMemFB;
		}

		hr = D3DGlobal.pDevice->GetRenderTargetData( lpRenderTarget, D3DGlobal.pSystemMemRT );
		if(FAILED(hr)) {
			D3DGlobal.lastError = hr;
			logPrintf("WARNING: glReadPixels: GetRenderTargetData failed with error '%s'\n", DXGetErrorString9(hr));
			return;
		}
	}

	//Lock a portion of surface
	D3DLOCKED_RECT lockrect;
	RECT srcrect;
	y = D3DGlobal.hCurrentMode.Height -(height + y);
	srcrect.left = x;
	srcrect.right = x + width;
	srcrect.top = y;
	srcrect.bottom = y + height;

	hr = D3DGlobal.pSystemMemRT->LockRect( &lockrect, &srcrect, D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY );
	if(FAILED(hr)) {
		D3DGlobal.lastError = hr;
		logPrintf("WARNING: glReadPixels: LockRect failed with error '%s'\n", DXGetErrorString9(hr));
		return;
	}

	//Pack pixels to destination
	const GLubyte *srcdata =(GLubyte*)lockrect.pBits;
	hr = D3DPixels_Pack( width, height, 1, lockrect.Pitch, 0, srcdata, 4, D3D_TEXTYPE_GENERIC, true, format, type, pixels );
	if(FAILED(hr)) {
		D3DGlobal.lastError = hr;
	}

	if( format == GL_DEPTH_COMPONENT )
	{
		logPrintf("glReadPixels(GL_DEPTH_COMPONENT): %i %i %i %i\n", x,y,width,height);
		logPrintf("glReadPixels(GL_DEPTH_COMPONENT): %f\n", *(GLfloat*)pixels);
	}
	
	//And we are done
	D3DGlobal.pSystemMemRT->UnlockRect();

	if(lpRenderTarget != D3DGlobal.pSystemMemFB)
		lpRenderTarget->Release();
}

OPENGL_API void WINAPI glCopyPixels( GLint, GLint, GLsizei, GLsizei, GLenum )
{
	logPrintf("WARNING: glCopyPixels is not supported\n");
}

OPENGL_API void glDrawPixels( GLsizei, GLsizei, GLenum, GLenum, const GLvoid* )
{
	logPrintf("WARNING: glDrawPixels is not supported\n");
}

OPENGL_API void WINAPI glGetPixelMapfv( GLenum map, GLfloat *values )
{
	GLfloat *mapPointer;
	DWORD mapSize;

	switch(map) {
	case GL_PIXEL_MAP_R_TO_R:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapRtoR;
		mapSize = D3DState.ClientPixelStoreState.pixelmapSizeRtoR;
		break;
	case GL_PIXEL_MAP_G_TO_G:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapGtoG;
		mapSize = D3DState.ClientPixelStoreState.pixelmapSizeGtoG;
		break;
	case GL_PIXEL_MAP_B_TO_B:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapBtoB;
		mapSize = D3DState.ClientPixelStoreState.pixelmapSizeBtoB;
		break;
	case GL_PIXEL_MAP_A_TO_A:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapAtoA;
		mapSize = D3DState.ClientPixelStoreState.pixelmapSizeAtoA;
		break;
	default:
		logPrintf("WARNING: glGetPixelMapfv supports only RGBA maps(bad map 0x%x)\n", map);
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}

	memcpy( values, mapPointer, mapSize * sizeof(GLfloat) );
}

OPENGL_API void WINAPI glGetPixelMapuiv( GLenum map, GLuint *values )
{
	GLfloat *mapPointer;
	DWORD mapSize;

	switch(map) {
	case GL_PIXEL_MAP_R_TO_R:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapRtoR;
		mapSize = D3DState.ClientPixelStoreState.pixelmapSizeRtoR;
		break;
	case GL_PIXEL_MAP_G_TO_G:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapGtoG;
		mapSize = D3DState.ClientPixelStoreState.pixelmapSizeGtoG;
		break;
	case GL_PIXEL_MAP_B_TO_B:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapBtoB;
		mapSize = D3DState.ClientPixelStoreState.pixelmapSizeBtoB;
		break;
	case GL_PIXEL_MAP_A_TO_A:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapAtoA;
		mapSize = D3DState.ClientPixelStoreState.pixelmapSizeAtoA;
		break;
	default:
		logPrintf("WARNING: glGetPixelMapuiv supports only RGBA maps(bad map 0x%x)\n", map);
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}

	for(DWORD i = 0; i < mapSize; ++i)
		values[i] =(GLuint)(mapPointer[i] * UINT_MAX);
}
OPENGL_API void WINAPI glGetPixelMapusv( GLenum map, GLushort *values )
{
	GLfloat *mapPointer;
	DWORD mapSize;

	switch(map) {
	case GL_PIXEL_MAP_R_TO_R:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapRtoR;
		mapSize = D3DState.ClientPixelStoreState.pixelmapSizeRtoR;
		break;
	case GL_PIXEL_MAP_G_TO_G:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapGtoG;
		mapSize = D3DState.ClientPixelStoreState.pixelmapSizeGtoG;
		break;
	case GL_PIXEL_MAP_B_TO_B:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapBtoB;
		mapSize = D3DState.ClientPixelStoreState.pixelmapSizeBtoB;
		break;
	case GL_PIXEL_MAP_A_TO_A:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapAtoA;
		mapSize = D3DState.ClientPixelStoreState.pixelmapSizeAtoA;
		break;
	default:
		logPrintf("WARNING: glGetPixelMapusv supports only RGBA maps(bad map 0x%x)\n", map);
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}

	for (DWORD i = 0; i < mapSize; ++i)
		values[i] = static_cast<GLushort>(mapPointer[i] * USHRT_MAX);
}

OPENGL_API void WINAPI glPixelMapfv( GLenum map, GLsizei mapsize, const GLfloat *values )
{
	GLfloat *mapPointer;
	DWORD *mapSizePointer;

	switch(map) {
	case GL_PIXEL_MAP_R_TO_R:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapRtoR;
		mapSizePointer = &D3DState.ClientPixelStoreState.pixelmapSizeRtoR;
		break;
	case GL_PIXEL_MAP_G_TO_G:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapGtoG;
		mapSizePointer = &D3DState.ClientPixelStoreState.pixelmapSizeGtoG;
		break;
	case GL_PIXEL_MAP_B_TO_B:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapBtoB;
		mapSizePointer = &D3DState.ClientPixelStoreState.pixelmapSizeBtoB;
		break;
	case GL_PIXEL_MAP_A_TO_A:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapAtoA;
		mapSizePointer = &D3DState.ClientPixelStoreState.pixelmapSizeAtoA;
		break;
	default:
		logPrintf("WARNING: glPixelMapfv supports only RGBA maps(bad map 0x%x)\n", map);
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}

	if(mapsize > IMPL_MAX_PIXEL_MAP_TABLE) {
		logPrintf("WARNING: glPixelMapfv truncated map size %i to %i\n", mapsize, IMPL_MAX_PIXEL_MAP_TABLE);
		mapsize = IMPL_MAX_PIXEL_MAP_TABLE;
	}

	*mapSizePointer = mapsize;
	memcpy( mapPointer, values, mapsize * sizeof(GLfloat) );
}

OPENGL_API void WINAPI glPixelMapuiv( GLenum map, GLsizei mapsize, const GLuint *values )
{
	GLfloat *mapPointer;
	DWORD *mapSizePointer;

	switch(map) {
	case GL_PIXEL_MAP_R_TO_R:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapRtoR;
		mapSizePointer = &D3DState.ClientPixelStoreState.pixelmapSizeRtoR;
		break;
	case GL_PIXEL_MAP_G_TO_G:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapGtoG;
		mapSizePointer = &D3DState.ClientPixelStoreState.pixelmapSizeGtoG;
		break;
	case GL_PIXEL_MAP_B_TO_B:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapBtoB;
		mapSizePointer = &D3DState.ClientPixelStoreState.pixelmapSizeBtoB;
		break;
	case GL_PIXEL_MAP_A_TO_A:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapAtoA;
		mapSizePointer = &D3DState.ClientPixelStoreState.pixelmapSizeAtoA;
		break;
	default:
		logPrintf("WARNING: glPixelMapuiv supports only RGBA maps(bad map 0x%x)\n", map);
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}

	if(mapsize > IMPL_MAX_PIXEL_MAP_TABLE) {
		logPrintf("WARNING: glPixelMapuiv truncated map size %i to %i\n", mapsize, IMPL_MAX_PIXEL_MAP_TABLE);
		mapsize = IMPL_MAX_PIXEL_MAP_TABLE;
	}

	*mapSizePointer = mapsize;
	for(int i = 0; i < mapsize; ++i)
		mapPointer[i] = static_cast<GLfloat>(values[i] / UINT_MAX);
}

OPENGL_API void WINAPI glPixelMapusv( GLenum map, GLsizei mapsize, const GLushort *values )
{
	GLfloat *mapPointer;
	DWORD *mapSizePointer;

	switch(map) {
	case GL_PIXEL_MAP_R_TO_R:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapRtoR;
		mapSizePointer = &D3DState.ClientPixelStoreState.pixelmapSizeRtoR;
		break;
	case GL_PIXEL_MAP_G_TO_G:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapGtoG;
		mapSizePointer = &D3DState.ClientPixelStoreState.pixelmapSizeGtoG;
		break;
	case GL_PIXEL_MAP_B_TO_B:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapBtoB;
		mapSizePointer = &D3DState.ClientPixelStoreState.pixelmapSizeBtoB;
		break;
	case GL_PIXEL_MAP_A_TO_A:
		mapPointer = D3DState.ClientPixelStoreState.pixelmapAtoA;
		mapSizePointer = &D3DState.ClientPixelStoreState.pixelmapSizeAtoA;
		break;
	default:
		logPrintf("WARNING: glPixelMapusv supports only RGBA maps(bad map 0x%x)\n", map);
		D3DGlobal.lastError = E_INVALIDARG;
		return;
	}

	if(mapsize > IMPL_MAX_PIXEL_MAP_TABLE) {
		logPrintf("WARNING: glPixelMapusv truncated map size %i to %i\n", mapsize, IMPL_MAX_PIXEL_MAP_TABLE);
		mapsize = IMPL_MAX_PIXEL_MAP_TABLE;
	}

	*mapSizePointer = mapsize;
	for(int i = 0; i < mapsize; ++i)
		mapPointer[i] = static_cast<GLfloat>(values[i] / USHRT_MAX);
}

OPENGL_API void WINAPI glPixelStorei( GLenum pname, GLint param )
{
	switch(pname) {
	case GL_UNPACK_SWAP_BYTES:
		D3DState.ClientPixelStoreState.unpackSwapBytes =(param > 0) ? GL_TRUE : GL_FALSE;
		break;
	case GL_UNPACK_LSB_FIRST:
		D3DState.ClientPixelStoreState.unpackLSBfirst =(param > 0) ? GL_TRUE : GL_FALSE;
		break;
	case GL_UNPACK_ROW_LENGTH:
		D3DState.ClientPixelStoreState.unpackRowLength = param;
		break;
	case GL_UNPACK_IMAGE_HEIGHT:
		D3DState.ClientPixelStoreState.unpackImageHeight = param;
		break;
	case GL_UNPACK_SKIP_ROWS:
		D3DState.ClientPixelStoreState.unpackSkipRows = param;
		break;
	case GL_UNPACK_SKIP_IMAGES:
		D3DState.ClientPixelStoreState.unpackSkipImages = param;
		break;
	case GL_UNPACK_SKIP_PIXELS:
		D3DState.ClientPixelStoreState.unpackSkipPixels = param;
		break;
	case GL_UNPACK_ALIGNMENT:
		D3DState.ClientPixelStoreState.unpackAlignment = param;
		if(param != 1 && param != 2 && param != 4 && param != 8) {
			logPrintf("WARNING: glPixelStore - invalid alignment %i\n", param);
			D3DState.ClientPixelStoreState.unpackAlignment = 1;
		}
		break;

	case GL_PACK_SWAP_BYTES:
		D3DState.ClientPixelStoreState.packSwapBytes =(param > 0) ? GL_TRUE : GL_FALSE;
		break;
	case GL_PACK_LSB_FIRST:
		D3DState.ClientPixelStoreState.packLSBfirst =(param > 0) ? GL_TRUE : GL_FALSE;
		break;
	case GL_PACK_ROW_LENGTH:
		D3DState.ClientPixelStoreState.packRowLength = param;
		break;
	case GL_PACK_IMAGE_HEIGHT:
		D3DState.ClientPixelStoreState.packImageHeight = param;
		break;
	case GL_PACK_SKIP_ROWS:
		D3DState.ClientPixelStoreState.packSkipRows = param;
		break;
	case GL_PACK_SKIP_IMAGES:
		D3DState.ClientPixelStoreState.packSkipImages = param;
		break;
	case GL_PACK_SKIP_PIXELS:
		D3DState.ClientPixelStoreState.packSkipPixels = param;
		break;
	case GL_PACK_ALIGNMENT:
		D3DState.ClientPixelStoreState.packAlignment = param;
		if(param != 1 && param != 2 && param != 4 && param != 8) {
			logPrintf("WARNING: glPixelStore - invalid alignment %i\n", param);
			D3DState.ClientPixelStoreState.packAlignment = 1;
		}
		break;

	default:
		logPrintf("WARNING: glPixelStore - invalid pname 0x%x\n", pname);
		D3DGlobal.lastError = E_INVALIDARG;
		break;
	}
}
OPENGL_API void WINAPI glPixelStoref( GLenum pname, GLfloat param )
{
	glPixelStorei( pname,(GLint)param );
}

OPENGL_API void WINAPI glPixelTransferi( GLenum pname, GLint param )
{
	switch(pname) {
	case GL_MAP_COLOR:
		D3DState.ClientPixelStoreState.transferMapColor =(param > 0) ? GL_TRUE : GL_FALSE;
		break;
	case GL_RED_SCALE:
		D3DState.ClientPixelStoreState.transferRedScale =(GLfloat)param;
		break;
	case GL_RED_BIAS:
		D3DState.ClientPixelStoreState.transferRedBias =(GLfloat)param;
		break;
	case GL_GREEN_SCALE:
		D3DState.ClientPixelStoreState.transferGreenScale =(GLfloat)param;
		break;
	case GL_GREEN_BIAS:
		D3DState.ClientPixelStoreState.transferGreenBias =(GLfloat)param;
		break;
	case GL_BLUE_SCALE:
		D3DState.ClientPixelStoreState.transferBlueScale =(GLfloat)param;
		break;
	case GL_BLUE_BIAS:
		D3DState.ClientPixelStoreState.transferBlueBias =(GLfloat)param;
		break;
	case GL_ALPHA_SCALE:
		D3DState.ClientPixelStoreState.transferAlphaScale =(GLfloat)param;
		break;
	case GL_ALPHA_BIAS:
		D3DState.ClientPixelStoreState.transferAlphaBias =(GLfloat)param;
		break;

	case GL_MAP_STENCIL:
	case GL_INDEX_SHIFT:
	case GL_INDEX_OFFSET:
	case GL_DEPTH_SCALE:
	case GL_DEPTH_BIAS:
		//Ignore for now
		break;

	default:
		logPrintf("WARNING: glPixelTransferi - invalid pname 0x%x\n", pname);
		D3DGlobal.lastError = E_INVALIDARG;
		break;
	}
}
OPENGL_API void WINAPI glPixelTransferf( GLenum pname, GLfloat param )
{
	switch(pname) {
	case GL_MAP_COLOR:
		D3DState.ClientPixelStoreState.transferMapColor =(param > 0) ? GL_TRUE : GL_FALSE;
		break;
	case GL_RED_SCALE:
		D3DState.ClientPixelStoreState.transferRedScale = param;
		break;
	case GL_RED_BIAS:
		D3DState.ClientPixelStoreState.transferRedBias = param;
		break;
	case GL_GREEN_SCALE:
		D3DState.ClientPixelStoreState.transferGreenScale = param;
		break;
	case GL_GREEN_BIAS:
		D3DState.ClientPixelStoreState.transferGreenBias = param;
		break;
	case GL_BLUE_SCALE:
		D3DState.ClientPixelStoreState.transferBlueScale = param;
		break;
	case GL_BLUE_BIAS:
		D3DState.ClientPixelStoreState.transferBlueBias = param;
		break;
	case GL_ALPHA_SCALE:
		D3DState.ClientPixelStoreState.transferAlphaScale = param;
		break;
	case GL_ALPHA_BIAS:
		D3DState.ClientPixelStoreState.transferAlphaBias = param;
		break;

	case GL_MAP_STENCIL:
	case GL_INDEX_SHIFT:
	case GL_INDEX_OFFSET:
	case GL_DEPTH_SCALE:
	case GL_DEPTH_BIAS:
		//Ignore for now
		break;

	default:
		logPrintf("WARNING: glPixelTransferf - invalid pname 0x%x\n", pname);
		D3DGlobal.lastError = E_INVALIDARG;
		break;
	}
}

OPENGL_API void WINAPI glPixelZoom( GLfloat, GLfloat )
{
	logPrintf("WARNING: glPixelZoom is not supported\n");
}

OPENGL_API void WINAPI glRasterPos2d( GLdouble, GLdouble )
{
	logPrintf("WARNING: glRasterPos2d is not supported\n");
}
OPENGL_API void WINAPI glRasterPos2dv( const GLdouble* )
{
	logPrintf("WARNING: glRasterPos2dv is not supported\n");
}
OPENGL_API void WINAPI glRasterPos2f( GLfloat, GLfloat )
{
	logPrintf("WARNING: glRasterPos2f is not supported\n");
}
OPENGL_API void WINAPI glRasterPos2fv( const GLfloat* )
{
	logPrintf("WARNING: glRasterPos2fv is not supported\n");
}
OPENGL_API void WINAPI glRasterPos2i( GLint, GLint )
{
	logPrintf("WARNING: glRasterPos2i is not supported\n");
}
OPENGL_API void WINAPI glRasterPos2iv( const GLint* )
{
	logPrintf("WARNING: glRasterPos2iv is not supported\n");
}
OPENGL_API void WINAPI glRasterPos2s( GLshort, GLshort )
{
	logPrintf("WARNING: glRasterPos2s is not supported\n");
}
OPENGL_API void WINAPI glRasterPos2sv( const GLshort* )
{
	logPrintf("WARNING: glRasterPos2sv is not supported\n");
}
OPENGL_API void WINAPI glRasterPos3d( GLdouble, GLdouble, GLdouble )
{
	logPrintf("WARNING: glRasterPos3d is not supported\n");
}
OPENGL_API void WINAPI glRasterPos3dv( const GLdouble* )
{
	logPrintf("WARNING: glRasterPos3dv is not supported\n");
}
OPENGL_API void WINAPI glRasterPos3f( GLfloat, GLfloat, GLfloat )
{
	logPrintf("WARNING: glRasterPos3f is not supported\n");
}
OPENGL_API void WINAPI glRasterPos3fv( const GLfloat* )
{
	logPrintf("WARNING: glRasterPos3fv is not supported\n");
}
OPENGL_API void WINAPI glRasterPos3i( GLint, GLint, GLint )
{
	logPrintf("WARNING: glRasterPos3i is not supported\n");
}
OPENGL_API void WINAPI glRasterPos3iv( const GLint* )
{
	logPrintf("WARNING: glRasterPos3iv is not supported\n");
}
OPENGL_API void WINAPI glRasterPos3s( GLshort, GLshort, GLshort )
{
	logPrintf("WARNING: glRasterPos3s is not supported\n");
}
OPENGL_API void WINAPI glRasterPos3sv( const GLshort* )
{
	logPrintf("WARNING: glRasterPos3sv is not supported\n");
}
OPENGL_API void WINAPI glRasterPos4d( GLdouble, GLdouble, GLdouble, GLdouble )
{
	logPrintf("WARNING: glRasterPos4d is not supported\n");
}
OPENGL_API void WINAPI glRasterPos4dv( const GLdouble* )
{
	logPrintf("WARNING: glRasterPos4dv is not supported\n");
}
OPENGL_API void WINAPI glRasterPos4f( GLfloat, GLfloat, GLfloat, GLfloat )
{
	logPrintf("WARNING: glRasterPos4f is not supported\n");
}
OPENGL_API void WINAPI glRasterPos4fv( const GLfloat* )
{
	logPrintf("WARNING: glRasterPos4fv is not supported\n");
}
OPENGL_API void WINAPI glRasterPos4i( GLint, GLint, GLint, GLint )
{
	logPrintf("WARNING: glRasterPos4i is not supported\n");
}
OPENGL_API void WINAPI glRasterPos4iv( const GLint* )
{
	logPrintf("WARNING: glRasterPos4iv is not supported\n");
}
OPENGL_API void WINAPI glRasterPos4s( GLshort, GLshort, GLshort, GLshort )
{
	logPrintf("WARNING: glRasterPos4s is not supported\n");
}
OPENGL_API void WINAPI glRasterPos4sv( const GLshort* )
{
	logPrintf("WARNING: glRasterPos4sv is not supported\n");
}
OPENGL_API void WINAPI glBitmap( GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, const GLubyte* )
{
	logPrintf("WARNING: glBitmap is not supported\n");
}
