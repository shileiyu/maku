#ifndef MAKU_RENDER_OPENGL_CANVAS_H_
#define MAKU_RENDER_OPENGL_CANVAS_H_
#include "canvas.h"
#include <gl\GL.h>


namespace maku
{
namespace render
{
namespace opengl
{

#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_TEXTURE0 0x84C0
#define GL_VERTEX_ARRAY_BINDING 0x85B5
#define GL_CURRENT_PROGRAM 0x8B8D 
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_LINK_STATUS 0x8B82

class OpenGLCanvas:public InternalCanvas
{
    struct SimpleVertex
    {
        int x, y, z;
        float u, v;
    };
public:
    OpenGLCanvas(HGLRC hrc);

    ~OpenGLCanvas();

    void DrawPoint(const Point & p1, const Paint & paint);

    void DrawLine(const Point & p1, 
                            const Point & p2, 
                            const Paint & paint);

    void DrawRect(const Rect & rect, const Paint & paint);

    void DrawBitmap(Bitmap & bmp_info, const Rect & dest);

    void UpdateRect(RECT & rect);

    void SaveStatus();

    void RestoreStatus();

    HGLRC hrc()
    {
        return hrc_;
    }

    uint32_t GetWidth() 
    {
        return rect_.width();
    }

    uint32_t GetHeight() 
    {
        return rect_.height();
    }
private:
    void Prepare();

    void Create();  

    void CreateVertexAndUVBuffer();

    void SetColor(Float4 f);

    void UpdateVertexData(SimpleVertex * p, int n);

    void GetTextureWH(uint32_t & width, uint32_t & height);

    void CreateTexture(uint32_t width, uint32_t height);

    void UpdateTexture(Bitmap & bmp_info);

    void GetDllProcAddress();

    void GetAllProcAddress();

    bool CheckAllProcAddress();

    GLuint LoadShader();

    bool IsShaderDraw();
private:
    Rect rect_;
    GLuint vertex_array_;
    GLuint program_id_;
    GLuint texture_id_;
    GLuint matrix_id_;
    GLuint texture_name_;
    GLuint vertex_buffer_;
    GLuint uv_buffer_;
    GLuint color_buffer_;
    bool shader_flag_;
    HGLRC hrc_;
    GLint old_array_;
    GLint old_texture_;
    GLint old_program_;
    GLint old_viewport_[4];
    GLfloat vertex_data_[4][3];
    GLfloat color_data_[4][4];
    GLfloat uv_data_[4][2];
    Matrix ortho_;
};
}
}
}
#endif