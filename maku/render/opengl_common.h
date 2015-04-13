#ifndef MAKU_RENDER_OPENGL_COMMON_H_
#define MAKU_RENDER_OPENGL_COMMON_H_
#include <ncore/ncore.h>
#include <gl\GL.h>

namespace maku
{

namespace render
{

namespace opengl
{
//hook api
typedef BOOL (WINAPI* SwapBuffers_T)(HDC);
typedef BOOL  (WINAPI* wglDeleteContext_T) (HGLRC hrc);
typedef BOOL  (WINAPI* wglMakeCurrent_T) (HDC, HGLRC);
//load api
typedef void (WINAPI * glDeleteTextures_T) (GLsizei n, const GLuint *textures);
typedef const GLubyte * (WINAPI * glGetString_T) (GLenum name);
typedef void (WINAPI * glGenTextures_T) (GLsizei n, GLuint *textures);
typedef void (WINAPI * glBindTexture_T) (GLenum target, GLuint texture);
typedef void (WINAPI * glTexParameteri_T)(GLenum target, GLenum pname, GLint param);
typedef void (WINAPI * glTexImage2D_T) (GLenum target,
    GLint level, GLint internalformat,
    GLsizei width, GLsizei height, GLint border, 
    GLenum format, GLenum type, const GLvoid *pixels);
typedef void (WINAPI * glViewport_T) (GLint x, GLint y,
    GLsizei width, GLsizei height);
typedef void (WINAPI * glEnable_T) (GLenum cap);
typedef void (WINAPI * glDisable_T) (GLenum cap);
typedef void (WINAPI * glGetTexLevelParameteriv_T) (GLenum target, 
    GLint level, GLenum pname, GLint *params);
typedef void (WINAPI * glTexSubImage2D_T) (GLenum target, 
    GLint level, GLint xoffset, GLint yoffset, 
    GLsizei width, GLsizei height, GLenum format, 
    GLenum type, const GLvoid *pixels);
typedef void (WINAPI * glPushAttrib_T) (GLbitfield mask);
typedef void (WINAPI * glPushClientAttrib_T) (GLbitfield mask);
typedef void (WINAPI * glPopClientAttrib_T) (void);
typedef void (WINAPI * glPopAttrib_T) (void);
typedef void (WINAPI * glBlendFunc_T) (GLenum sfactor, GLenum dfactor);
typedef void (WINAPI * glGetIntegerv_T) (GLenum pname, GLint *params);
typedef void (WINAPI * glMatrixMode_T) (GLenum mode);
typedef void (WINAPI * glPushMatrix_T) (void);
typedef void (WINAPI * glPopMatrix_T) (void);
typedef void (WINAPI * glLoadIdentity_T) (void);
typedef void (WINAPI * glOrtho_T) (
    GLdouble left, 
    GLdouble right, GLdouble bottom, 
    GLdouble top, GLdouble zNear, GLdouble zFar);
typedef void (WINAPI * glDrawArrays_T) (GLenum mode, GLint first, GLsizei count);
typedef void (WINAPI * glBegin_T) (GLenum mode);
typedef void (WINAPI * glColor3f_T) (GLfloat red, GLfloat green, GLfloat blue);
typedef void (WINAPI * glTexCoord2f_T) (GLfloat s, GLfloat t);
typedef void (WINAPI * glVertex3f_T) (GLfloat x, GLfloat y, GLfloat z);
typedef void (WINAPI * glEnd_T) (void);
typedef void (WINAPI * glColor4f_T) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef PROC  (WINAPI * wglGetProcAddress_T) (LPCSTR);
//3.0版本api
typedef void (WINAPI*glGenVertexArrays_T) (	GLsizei  n, GLuint * arrays);
typedef void (WINAPI*glBindVertexArray_T) (GLuint array);
typedef void (WINAPI*glBindBuffer_T) (GLenum target, GLuint buffer);
typedef void (WINAPI*glGenBuffers_T) (GLsizei n, GLuint * buffers);
typedef GLint (WINAPI*glGetUniformLocation_T) (GLuint program, 
             const char* name);
typedef void (WINAPI* glUniformMatrix4fv_T)(GLint location, 
    GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (WINAPI*glBufferData_T) (GLenum target, int  size,
    const GLvoid* data, GLenum  usage);
typedef void (WINAPI* glDeleteBuffers_T)(GLsizei 	n,
      const GLuint * buffers);
typedef void (WINAPI* glDeleteVertexArrays_T)(GLsizei  n,
       const GLuint * arrays);
typedef void (WINAPI*glUseProgram_T) (GLuint program);
typedef void (WINAPI*glActiveTexture_T) (GLenum texture);
typedef void (WINAPI*glUniform1i_T) (GLint location, GLint v0);
typedef void (WINAPI*glEnableVertexAttribArray_T) (GLuint index);
typedef void (WINAPI*glDisableVertexAttribArray_T) (GLuint  index);
typedef void (WINAPI*glVertexAttribPointer_T) (GLuint index,
    GLint  size, GLenum type, GLboolean normalized,
    GLsizei  	stride, const GLvoid* pointer);
typedef GLuint (WINAPI*glCreateShader_T) (GLenum shaderType);
typedef void (WINAPI*glShaderSource_T) (GLuint shader, GLsizei count,
    const char** 	string, const GLint* 	length);
typedef void (WINAPI*glCompileShader_T) (GLuint shader);
typedef void (WINAPI*glGetShaderiv_T) (GLuint	shader,
    GLenum pname, GLint* params);
typedef GLuint (WINAPI*glCreateProgram_T) (void);
typedef void (WINAPI * glDeleteProgram_T)(GLuint program);
typedef void (WINAPI*glAttachShader_T) (GLuint program, GLuint shader);
typedef void (WINAPI*glLinkProgram_T) (GLuint program);
typedef void (WINAPI*glGetProgramiv_T) (GLuint program,
    GLenum pname, GLint*	params);
typedef void (WINAPI*glDeleteShader_T) (GLuint shader);
//
BOOL WINAPI FSwapBuffers(HDC dc);
HGLRC WINAPI FwglCreateContext (HDC hdc);
BOOL  WINAPI FwglMakeCurrent(HDC dc, HGLRC hrc);
}
}
}
#endif