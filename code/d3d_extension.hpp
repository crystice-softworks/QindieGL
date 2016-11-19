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
#ifndef QINDIEGL_D3D_EXTENSION_H
#define QINDIEGL_D3D_EXTENSION_H

extern void D3DExtension_BuildExtensionsString();

extern OPENGL_API void WINAPI glActiveTexture(GLenum texture);
extern OPENGL_API void WINAPI glClientActiveTexture(GLenum texture);
extern OPENGL_API void WINAPI glMultiTexCoord1s( GLenum target, GLshort s );
extern OPENGL_API void WINAPI glMultiTexCoord1i( GLenum target, GLint s );
extern OPENGL_API void WINAPI glMultiTexCoord1f( GLenum target, GLfloat s );
extern OPENGL_API void WINAPI glMultiTexCoord1d( GLenum target, GLdouble s );
extern OPENGL_API void WINAPI glMultiTexCoord2s( GLenum target, GLshort s, GLshort t );
extern OPENGL_API void WINAPI glMultiTexCoord2i( GLenum target, GLint s, GLint t );
extern OPENGL_API void WINAPI glMultiTexCoord2f( GLenum target, GLfloat s, GLfloat t );
extern OPENGL_API void WINAPI glMultiTexCoord2d( GLenum target, GLdouble s, GLdouble t );
extern OPENGL_API void WINAPI glMultiTexCoord3s( GLenum target, GLshort s, GLshort t, GLshort r );
extern OPENGL_API void WINAPI glMultiTexCoord3i( GLenum target, GLint s, GLint t, GLint r );
extern OPENGL_API void WINAPI glMultiTexCoord3f( GLenum target, GLfloat s, GLfloat t, GLfloat r );
extern OPENGL_API void WINAPI glMultiTexCoord3d( GLenum target, GLdouble s, GLdouble t, GLdouble r );
extern OPENGL_API void WINAPI glMultiTexCoord4s( GLenum target, GLshort s, GLshort t, GLshort r, GLshort q );
extern OPENGL_API void WINAPI glMultiTexCoord4i( GLenum target, GLint s, GLint t, GLint r, GLint q );
extern OPENGL_API void WINAPI glMultiTexCoord4f( GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q );
extern OPENGL_API void WINAPI glMultiTexCoord4d( GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q );
extern OPENGL_API void WINAPI glMultiTexCoord1sv( GLenum target, const GLshort *v );
extern OPENGL_API void WINAPI glMultiTexCoord1iv( GLenum target, const GLint *v );
extern OPENGL_API void WINAPI glMultiTexCoord1fv( GLenum target, const GLfloat *v );
extern OPENGL_API void WINAPI glMultiTexCoord1dv( GLenum target, const GLdouble *v );
extern OPENGL_API void WINAPI glMultiTexCoord2sv( GLenum target, const GLshort *v );
extern OPENGL_API void WINAPI glMultiTexCoord2iv( GLenum target, const GLint *v );
extern OPENGL_API void WINAPI glMultiTexCoord2fv( GLenum target, const GLfloat *v );
extern OPENGL_API void WINAPI glMultiTexCoord2dv( GLenum target, const GLdouble *v );
extern OPENGL_API void WINAPI glMultiTexCoord3sv( GLenum target, const GLshort *v );
extern OPENGL_API void WINAPI glMultiTexCoord3iv( GLenum target, const GLint *v );
extern OPENGL_API void WINAPI glMultiTexCoord3fv( GLenum target, const GLfloat *v );
extern OPENGL_API void WINAPI glMultiTexCoord3dv( GLenum target, const GLdouble *v );
extern OPENGL_API void WINAPI glMultiTexCoord4sv( GLenum target, const GLshort *v );
extern OPENGL_API void WINAPI glMultiTexCoord4iv( GLenum target, const GLint *v );
extern OPENGL_API void WINAPI glMultiTexCoord4fv( GLenum target, const GLfloat *v );
extern OPENGL_API void WINAPI glMultiTexCoord4dv( GLenum target, const GLdouble *v );
extern OPENGL_API void WINAPI WINAPI glActiveStencilFace(GLenum face);
extern OPENGL_API void WINAPI WINAPI glDeleteTextures(GLsizei n, const GLuint *textures);
extern OPENGL_API void WINAPI WINAPI glGenTextures(GLsizei n, GLuint *textures);
extern OPENGL_API GLboolean WINAPI glIsTexture(GLuint texture);
extern OPENGL_API void WINAPI glBindTexture(GLenum target, GLuint texture);
extern OPENGL_API GLboolean WINAPI glAreTexturesResident(GLsizei n,  const GLuint *textures,  GLboolean *residences);
extern OPENGL_API void WINAPI glPrioritizeTextures(GLsizei n,  const GLuint *textures,  const GLclampf *priorities);
extern OPENGL_API void WINAPI glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
extern OPENGL_API void WINAPI glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
extern OPENGL_API void WINAPI glCopyTexImage3D(GLenum target,  GLint level,  GLenum internalFormat,  GLint x,  GLint y,  GLint z,  GLsizei width,  GLsizei height,  GLsizei depth,  GLint border);
extern OPENGL_API void WINAPI glCopyTexSubImage3D(GLenum target,  GLint level,  GLint xoffset,  GLint yoffset,  GLint zoffset,  GLint x,  GLint y,  GLint z,  GLsizei width,  GLsizei height,  GLsizei depth);
extern OPENGL_API BOOL WINAPI wglSwapInterval(int interval);
extern OPENGL_API int WINAPI wglGetSwapInterval();
extern OPENGL_API void WINAPI glSelectTexture(GLenum texture);
extern OPENGL_API void WINAPI glMTexCoord2f( GLenum target, GLfloat s, GLfloat t );
extern OPENGL_API void WINAPI glMTexCoord2fv( GLenum target, const GLfloat *v );
extern OPENGL_API void WINAPI glLoadTransposeMatrixf(const GLfloat *m);
extern OPENGL_API void WINAPI glLoadTransposeMatrixd(const GLdouble *m);
extern OPENGL_API void WINAPI glMultTransposeMatrixf(const GLfloat *m);
extern OPENGL_API void WINAPI glMultTransposeMatrixd(const GLdouble *m);
extern OPENGL_API void WINAPI glFogCoordd( GLdouble coord );
extern OPENGL_API void WINAPI glFogCoordf( GLfloat coord );
extern OPENGL_API void WINAPI glFogCoorddv( GLdouble *coord );
extern OPENGL_API void WINAPI glFogCoordfv( GLfloat *coord );
extern OPENGL_API void WINAPI glFogCoordPointer(GLenum type,  GLsizei stride,  const GLvoid *pointer);
extern OPENGL_API void WINAPI glSecondaryColor3b(GLbyte red,  GLbyte green,  GLbyte blue);
extern OPENGL_API void WINAPI glSecondaryColor3bv(const GLbyte *v);
extern OPENGL_API void WINAPI glSecondaryColor3d(GLdouble red,  GLdouble green,  GLdouble blue);
extern OPENGL_API void WINAPI glSecondaryColor3dv(const GLdouble *v);
extern OPENGL_API void WINAPI glSecondaryColor3f(GLfloat red,  GLfloat green,  GLfloat blue);
extern OPENGL_API void WINAPI glSecondaryColor3fv(const GLfloat *v);
extern OPENGL_API void WINAPI glSecondaryColor3i(GLint red,  GLint green,  GLint blue);
extern OPENGL_API void WINAPI glSecondaryColor3iv(const GLint *v);
extern OPENGL_API void WINAPI glSecondaryColor3s(GLshort red,  GLshort green,  GLshort blue);
extern OPENGL_API void WINAPI glSecondaryColor3sv(const GLshort *v);
extern OPENGL_API void WINAPI glSecondaryColor3ub(GLubyte red,  GLubyte green,  GLubyte blue);
extern OPENGL_API void WINAPI glSecondaryColor3ubv(const GLubyte *v);
extern OPENGL_API void WINAPI glSecondaryColor3ui(GLuint red,  GLuint green,  GLuint blue);
extern OPENGL_API void WINAPI glSecondaryColor3uiv(const GLuint *v);
extern OPENGL_API void WINAPI glSecondaryColor3us(GLushort red,  GLushort green,  GLushort blue);
extern OPENGL_API void WINAPI glSecondaryColor3usv(const GLushort *v);
extern OPENGL_API void WINAPI glSecondaryColorPointer(GLint size,  GLenum type,  GLsizei stride,  const GLvoid *pointer);
extern OPENGL_API void WINAPI glDrawRangeElements( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices );
extern OPENGL_API void WINAPI glMultiDrawArrays( GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount );
extern OPENGL_API void WINAPI glMultiDrawElements( GLenum mode, GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount );
extern OPENGL_API void WINAPI glLockArrays( GLint first, GLsizei count );
extern OPENGL_API void WINAPI glUnlockArrays();
extern OPENGL_API void WINAPI glBlendColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );
extern OPENGL_API void WINAPI glBlendEquation( GLenum mode );
extern OPENGL_API void WINAPI glCompressedTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *pixels);
extern OPENGL_API void WINAPI glCompressedTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *pixels);
extern OPENGL_API void WINAPI glCompressedTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *pixels);
extern OPENGL_API void WINAPI glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *pixels);
extern OPENGL_API void WINAPI glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *pixels);
extern OPENGL_API void WINAPI glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *pixels);
extern OPENGL_API void WINAPI glGetCompressedTexImage(GLenum target, GLint level, GLvoid *img);

#endif //QINDIEGL_D3D_EXTENSION_H