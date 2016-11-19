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
#ifndef QINDIEGL_D3D_PIXELS_H
#define QINDIEGL_D3D_PIXELS_H

extern HRESULT D3DPixels_Unpack( int width, int height, int depth, int hpitch, int vpitch, GLubyte *dstbytes, int dstpixelsize, eTexTypeInternal intfmt, bool flipVertical, GLenum format, GLenum type, const GLvoid *pixels );
extern HRESULT D3DPixels_Pack( int width, int height, int depth, int hpitch, int vpitch, const GLubyte *srcbytes, int srcpixelsize, eTexTypeInternal intfmt, bool flipVertical, GLenum format, GLenum type, GLvoid *pixels );

#endif //QINDIEGL_D3D_PIXELS_H