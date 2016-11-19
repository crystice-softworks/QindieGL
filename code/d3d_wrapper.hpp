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
#ifndef QINDIEGL_D3D_WRAPPER_H
#define QINDIEGL_D3D_WRAPPER_H

#include <cstdio>
#include <cassert>
#include <limits>
#include <time.h>
#include <direct.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

#define NO_GL_PROTOTYPES
#include "gl_headers/gl.h"
#include "gl_headers/glext.h"

#define OPENGL_API

#define WRAPPER_GL_VENDOR_STRING			"Crystice Softworks (Microsoft Direct3D 9.0c)"
#define WRAPPER_GL_VERSION_STRING			"1.1"
#define WRAPPER_GL_SHORT_NAME_STRING		"QindieGL"
#define WRAPPER_GL_WRAPPER_NAME_STRING		"QindieGL by Crystice Softworks"
#define WRAPPER_GL_WRAPPER_VERSION_STRING	"1.0"

#define WRAPPER_REGISTRY_PATH				"SOFTWARE\\Crystice Softworks\\QindieGL"
#define WRAPPER_REGISTRY_CFG_EXT			"Config"
#define WRAPPER_REGISTRY_SUB_EXT			"Extensions"

#define MAX_D3D_TMU							8

#define E_STACK_OVERFLOW					0x8000000A
#define E_STACK_UNDERFLOW					0x8000000B
#define E_INVALID_ENUM						0x8000000C
#define E_INVALID_OPERATION					0x8000000D

#define GL_SELECTED_TEXTURE_SGIS			0x835B
#define GL_MAX_TEXTURES_SGIS				0x835D
#define GL_TEXTURE0_SGIS					0x835E

#define GL_WRAPPER_NAME_CHS					0x1FA0
#define GL_WRAPPER_VERSION_CHS				0x1FA1

#define QINDIEGL_CLAMP(i)					(int) ((((i) > 255) ? 255 : (((i) < 0) ? 0 : (i))))
#define QINDIEGL_MAX(a,b)					((a) > (b) ? (a) : (b))
#define QINDIEGL_MIN(a,b)					((a) < (b) ? (a) : (b))



extern void logPrintf( const char *fmt, ... );

#endif //QINDIEGL_D3D_WRAPPER_H