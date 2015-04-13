#include "opengl_canvas.h"
#include "opengl_common.h"
#include "opengl_shader.h"
#include "utils.h"

namespace maku
{
namespace render
{
namespace opengl
{
#define GETADDRESS(x) T##x = (x##_T)TwglGetProcAddress(#x)
#define CHECKADDR(x)   if(!(x)) return false;
//
glDeleteTextures_T TglDeleteTextures = NULL;
glGetString_T TglGetString = NULL;
glGenTextures_T TglGenTextures = NULL;
glBindTexture_T TglBindTexture = NULL;
glTexParameteri_T TglTexParameteri = NULL;
glTexImage2D_T TglTexImage2D = NULL;
glViewport_T TglViewport = NULL;
glEnable_T TglEnable = NULL;
glDisable_T TglDisable = NULL;
glGetTexLevelParameteriv_T TglGetTexLevelParameteriv= NULL;
glTexSubImage2D_T TglTexSubImage2D = NULL;
glPushAttrib_T TglPushAttrib = NULL;
glPushClientAttrib_T TglPushClientAttrib = NULL;
glPopClientAttrib_T TglPopClientAttrib = NULL;
glPopAttrib_T TglPopAttrib = NULL;
glBlendFunc_T TglBlendFunc = NULL;
glGetIntegerv_T TglGetIntegerv = NULL;
glMatrixMode_T TglMatrixMode = NULL;
glPushMatrix_T TglPushMatrix = NULL;
glPopMatrix_T TglPopMatrix = NULL;
glLoadIdentity_T TglLoadIdentity = NULL;
glOrtho_T TglOrtho = NULL;
glDrawArrays_T TglDrawArrays = NULL;
glBegin_T TglBegin= NULL;
glColor3f_T TglColor3f = NULL;
glTexCoord2f_T TglTexCoord2f = NULL;
glVertex3f_T TglVertex3f = NULL;
glEnd_T TglEnd = NULL;
glColor4f_T TglColor4f = NULL;
wglGetProcAddress_T TwglGetProcAddress = NULL;
//
glGenVertexArrays_T TglGenVertexArrays = NULL;
glGenBuffers_T TglGenBuffers = NULL;
glDeleteBuffers_T TglDeleteBuffers = NULL;
glBindBuffer_T TglBindBuffer = NULL;
glDeleteVertexArrays_T TglDeleteVertexArrays = NULL;
glBindVertexArray_T TglBindVertexArray = NULL;
glGetUniformLocation_T TglGetUniformLocation = NULL;
glUniformMatrix4fv_T  TglUniformMatrix4fv = NULL;
glBufferData_T TglBufferData = NULL;
glUseProgram_T TglUseProgram = NULL;
glActiveTexture_T TglActiveTexture = NULL;
glUniform1i_T TglUniform1i = NULL;
glEnableVertexAttribArray_T TglEnableVertexAttribArray = NULL;
glDisableVertexAttribArray_T TglDisableVertexAttribArray = NULL;
glVertexAttribPointer_T TglVertexAttribPointer = NULL;
//
glCreateShader_T TglCreateShader = NULL;
glShaderSource_T TglShaderSource = NULL;
glCompileShader_T TglCompileShader = NULL;
glGetShaderiv_T TglGetShaderiv = NULL;
glCreateProgram_T TglCreateProgram = NULL;
glDeleteProgram_T TglDeleteProgram = NULL;
glAttachShader_T TglAttachShader = NULL;
glLinkProgram_T TglLinkProgram = NULL;
glGetProgramiv_T TglGetProgramiv = NULL;
glDeleteShader_T TglDeleteShader = NULL;

OpenGLCanvas::OpenGLCanvas(HGLRC hrc)
{
    vertex_array_ = 0;
    program_id_ = 0;
    texture_id_ = 0;
    texture_name_ = 0;
    matrix_id_ = 0;
    old_array_ = 0;
    old_program_ = 0;
    old_texture_ = 0;
    vertex_buffer_ = 0;
    uv_buffer_ = 0;
    color_buffer_ = 0;
    hrc_ = hrc;
    ZeroMemory(vertex_data_, sizeof(vertex_data_));
    ZeroMemory(uv_data_, sizeof(uv_data_));
    ZeroMemory(color_data_, sizeof(color_data_));
    ZeroMemory(old_viewport_, 4 * sizeof(GLint));
    ZeroMemory(&ortho_, sizeof(ortho_));
    ZeroMemory(&rect_, sizeof(rect_));
    uv_data_[1][0] = uv_data_[2][1] = uv_data_[3][0] = uv_data_[3][1] = 1.0f;
    GetDllProcAddress();
    shader_flag_ = IsShaderDraw();
}

OpenGLCanvas::~OpenGLCanvas()
{
    //delete buffer
    if (0 != texture_name_)
        TglDeleteTextures(1, &texture_name_);
    if(vertex_buffer_)
        TglDeleteBuffers(1, &vertex_buffer_);
    if(uv_buffer_)
        TglDeleteBuffers(1, &uv_buffer_);
    if(color_buffer_)
        TglDeleteBuffers(1, &color_buffer_);
    if(0 != vertex_array_)
        TglDeleteVertexArrays(1, &vertex_array_);
    if(0 != program_id_)
        TglDeleteProgram(program_id_);
}

void OpenGLCanvas::GetDllProcAddress()
{
    HMODULE gl_mod = ::GetModuleHandle(L"opengl32.dll");

    TglDeleteTextures = (glDeleteTextures_T)GetProcAddress(gl_mod, "glDeleteTextures");
    TglGetString = (glGetString_T)GetProcAddress(gl_mod, "glGetString");
    TglGenTextures = (glGenTextures_T)GetProcAddress(gl_mod, "glGenTextures");
    TglBindTexture = (glBindTexture_T)GetProcAddress(gl_mod, "glBindTexture");
    TglTexParameteri = (glTexParameteri_T)GetProcAddress(gl_mod, "glTexParameteri");
    TglTexImage2D = (glTexImage2D_T)GetProcAddress(gl_mod, "glTexImage2D");
    TglViewport = (glViewport_T)GetProcAddress(gl_mod, "glViewport");
    TglEnable = (glEnable_T)GetProcAddress(gl_mod, "glEnable");
    TglDisable = (glDisable_T)GetProcAddress(gl_mod, "glDisable");
    TglGetTexLevelParameteriv= (glGetTexLevelParameteriv_T)GetProcAddress(gl_mod, "glGetTexLevelParameteriv");
    TglTexSubImage2D = (glTexSubImage2D_T)GetProcAddress(gl_mod, "glTexSubImage2D");
    TglPushAttrib = (glPushAttrib_T)GetProcAddress(gl_mod, "glPushAttrib");
    TglPushClientAttrib = (glPushClientAttrib_T)GetProcAddress(gl_mod, "glPushClientAttrib");
    TglPopClientAttrib = (glPopClientAttrib_T)GetProcAddress(gl_mod, "glPopClientAttrib");
    TglPopAttrib = (glPopAttrib_T)GetProcAddress(gl_mod, "glPopAttrib");
    TglBlendFunc = (glBlendFunc_T)GetProcAddress(gl_mod, "glBlendFunc");
    TglGetIntegerv = (glGetIntegerv_T)GetProcAddress(gl_mod, "glGetIntegerv");
    TglMatrixMode = (glMatrixMode_T)GetProcAddress(gl_mod, "glMatrixMode");
    TglPushMatrix = (glPushMatrix_T)GetProcAddress(gl_mod, "glPushMatrix");
    TglPopMatrix = (glPopMatrix_T)GetProcAddress(gl_mod, "glPopMatrix");
    TglLoadIdentity = (glLoadIdentity_T)GetProcAddress(gl_mod, "glLoadIdentity");
    TglOrtho = (glOrtho_T)GetProcAddress(gl_mod, "glOrtho");
    TglDrawArrays = (glDrawArrays_T)GetProcAddress(gl_mod, "glDrawArrays");
    TglBegin= (glBegin_T)GetProcAddress(gl_mod, "glBegin");
    TglColor3f = (glColor3f_T)GetProcAddress(gl_mod, "glColor3f");
    TglTexCoord2f = (glTexCoord2f_T)GetProcAddress(gl_mod, "glTexCoord2f");
    TglVertex3f = (glVertex3f_T)GetProcAddress(gl_mod, "glVertex3f");
    TglEnd = (glEnd_T)GetProcAddress(gl_mod, "glEnd");
    TglColor4f = (glColor4f_T)GetProcAddress(gl_mod, "glColor4f");
    TwglGetProcAddress = (wglGetProcAddress_T)GetProcAddress(gl_mod, "wglGetProcAddress");
}

void OpenGLCanvas::GetAllProcAddress()
{
    GETADDRESS(glGenVertexArrays);
    GETADDRESS(glGenBuffers);
    GETADDRESS(glDeleteBuffers);
    GETADDRESS(glBindBuffer);
    GETADDRESS(glBindVertexArray);
    GETADDRESS(glDeleteVertexArrays);
    GETADDRESS(glGetUniformLocation);
    GETADDRESS(glUniformMatrix4fv);
    GETADDRESS(glBufferData);
    GETADDRESS(glUseProgram);
    GETADDRESS(glActiveTexture);
    GETADDRESS(glUniform1i);
    GETADDRESS(glEnableVertexAttribArray);
    GETADDRESS(glDisableVertexAttribArray);
    GETADDRESS(glVertexAttribPointer);
    GETADDRESS(glCreateShader);
    GETADDRESS(glShaderSource);
    GETADDRESS(glCompileShader);
    GETADDRESS(glGetShaderiv);
    GETADDRESS(glCreateProgram);
    GETADDRESS(glDeleteProgram);
    GETADDRESS(glAttachShader);
    GETADDRESS(glLinkProgram);
    GETADDRESS(glGetProgramiv);
    GETADDRESS(glDeleteShader);
}

bool OpenGLCanvas::CheckAllProcAddress()
{
    CHECKADDR(TglGenVertexArrays);
    CHECKADDR(TglGenBuffers);
    CHECKADDR(TglDeleteBuffers);
    CHECKADDR(TglBindBuffer);
    CHECKADDR(TglBindVertexArray);
    CHECKADDR(TglDeleteVertexArrays);
    CHECKADDR(TglGetUniformLocation);
    CHECKADDR(TglUniformMatrix4fv);
    CHECKADDR(TglBufferData);
    CHECKADDR(TglUseProgram);
    CHECKADDR(TglActiveTexture);
    CHECKADDR(TglUniform1i);
    CHECKADDR(TglEnableVertexAttribArray);   
    CHECKADDR(TglDisableVertexAttribArray);  
    CHECKADDR(TglVertexAttribPointer);  
    CHECKADDR(TglCreateShader);  
    CHECKADDR(TglShaderSource);  
    CHECKADDR(TglCompileShader);  
    CHECKADDR(TglGetShaderiv);  
    CHECKADDR(TglCreateProgram);  
    CHECKADDR(TglDeleteProgram);
    CHECKADDR(TglAttachShader);  
    CHECKADDR(TglLinkProgram);  
    CHECKADDR(TglGetProgramiv);  
    CHECKADDR(TglDeleteShader);    
    return true;
}

GLuint OpenGLCanvas::LoadShader()
{
    // Create the shaders
    GLuint VertexShaderID = TglCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = TglCreateShader(GL_FRAGMENT_SHADER);
    // Read the Vertex Shader code from the file
    std::string VertexShaderCode(vertex_shader);

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode(fragment_shader);

    // Compile Vertex Shader
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    TglShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    TglCompileShader(VertexShaderID);

    // Compile Fragment Shader
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    TglShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    TglCompileShader(FragmentShaderID);

    // Link the program
    GLuint ProgramID = TglCreateProgram();
    TglAttachShader(ProgramID, VertexShaderID);
    TglAttachShader(ProgramID, FragmentShaderID);
    TglLinkProgram(ProgramID);

    TglDeleteShader(VertexShaderID);
    TglDeleteShader(FragmentShaderID);
    return ProgramID;
}

bool OpenGLCanvas::IsShaderDraw()
{
    if(TglGetString == 0)
        return false;

    int main_version = 0;
    bool result = false;
    const char* str = (const char*)TglGetString(GL_VERSION);
    if(NULL != str)
        sscanf_s(str, "%d", &main_version);
    if(main_version >= 3)
        result = true;
    return result;
}

void OpenGLCanvas::CreateTexture(uint32_t width, uint32_t height)
{
    //gen empty texture
    TglGenTextures(1, &texture_name_);
    TglBindTexture(GL_TEXTURE_2D, texture_name_);
    TglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    TglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    TglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    TglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    TglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
}

void OpenGLCanvas::SetColor(Float4 f)
{
    for (int i = 0; i < 4; ++i)
    {
        color_data_[i][0] = f.x;
        color_data_[i][1] = f.y;
        color_data_[i][2] = f.z;
        color_data_[i][3] = f.w;
    }

    TglBindBuffer(GL_ARRAY_BUFFER, color_buffer_);
    TglBufferData(GL_ARRAY_BUFFER, sizeof(color_data_), color_data_, GL_STATIC_DRAW);

    TglEnableVertexAttribArray(2);
    TglBindBuffer(GL_ARRAY_BUFFER, color_buffer_);
    TglVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

void OpenGLCanvas::CreateVertexAndUVBuffer()
{
    TglGenVertexArrays(1, &vertex_array_);
    TglBindVertexArray(vertex_array_);
    //color buffer
    TglGenBuffers(1, &color_buffer_);
    //vertex
    TglGenBuffers(1, &vertex_buffer_);
    TglBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    TglBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data_), vertex_data_, GL_STATIC_DRAW);
    //uv
    TglGenBuffers(1, &uv_buffer_);
    TglBindBuffer(GL_ARRAY_BUFFER, uv_buffer_);
    TglBufferData(GL_ARRAY_BUFFER, sizeof(uv_data_), uv_data_, GL_STATIC_DRAW);
    //vertex
    TglEnableVertexAttribArray(0);
    TglBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    TglVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    //uv
    TglEnableVertexAttribArray(1);
    TglBindBuffer(GL_ARRAY_BUFFER, uv_buffer_);
    TglVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

void OpenGLCanvas::UpdateRect(RECT & rect)
{
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    if (width != rect_.width() || height != rect_.height())
    {
        TransRect(rect, rect_);
        ortho_ = MatrixOrthographicOffCenterLH((float)rect_.left(),
            (float)rect_.right(), (float)rect_.bottom(),
            (float)rect_.top(), 0.0f, 100.0f);

        if (0 != texture_name_)
            TglDeleteTextures(1, &texture_name_);
        texture_name_ = 0;
    }
}

void OpenGLCanvas::Prepare()
{
    if (0 == program_id_)
        Create();
    if(0 == texture_name_)
        CreateTexture(rect_.width(), rect_.height());

    TglViewport(rect_.left(), rect_.top(), rect_.width(), rect_.height());
    if (shader_flag_)
    {
        //set status
        TglBindVertexArray(vertex_array_);                
        TglActiveTexture(GL_TEXTURE0);
        TglEnable(GL_TEXTURE_2D);
        TglBindTexture(GL_TEXTURE_2D, texture_name_);

        TglUseProgram(program_id_);
        TglUniform1i(texture_id_, 0);
    }
}

void OpenGLCanvas::GetTextureWH(uint32_t & width, uint32_t & height)
{
    GLint temp;
    TglGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &temp);
    width = temp;
    TglGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &temp);
    height = temp;
}

void OpenGLCanvas::SaveStatus()
{
    //save status
    TglPushAttrib(GL_ALL_ATTRIB_BITS);                
    TglPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    //set status
    TglEnable(GL_BLEND);
    TglEnable(GL_ALPHA_TEST);
    TglDisable(GL_CULL_FACE);
    TglDisable(GL_SCISSOR_TEST);
    TglDisable(GL_DEPTH_TEST);
    TglEnable(GL_TEXTURE_2D);
    TglEnable(GL_TEXTURE);
    TglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    TglGetIntegerv(GL_VIEWPORT, old_viewport_);
    if (shader_flag_)
    {
        TglGetIntegerv(GL_VERTEX_ARRAY_BINDING, &old_array_);
        TglGetIntegerv(GL_TEXTURE_BINDING_2D, &old_texture_);
        TglGetIntegerv(GL_CURRENT_PROGRAM, &old_program_);
    }
    else
    {
        //push matrix
        TglMatrixMode(GL_PROJECTION);
        TglPushMatrix();
        TglLoadIdentity();
        TglOrtho(0.0, rect_.width() - 1.0, 0.0, rect_.height() - 1.0, -1.0, 1.0);
        TglMatrixMode(GL_MODELVIEW);
        TglPushMatrix();
        TglLoadIdentity();
    }
//prepare
    Prepare();
}

void OpenGLCanvas::RestoreStatus()
{
    if (shader_flag_)
    {
        TglBindVertexArray(old_array_);
        TglBindTexture(GL_TEXTURE_2D, old_texture_);
        TglUseProgram(old_program_);
    } 
    else
    {
        //pop matrix
        TglMatrixMode(GL_MODELVIEW);
        TglPopMatrix();
        TglMatrixMode(GL_PROJECTION);
        TglPopMatrix();
        //TglMatrixMode(GL_MODELVIEW);
    }
    //restore status 
    TglViewport(old_viewport_[0], old_viewport_[1],
                old_viewport_[2], old_viewport_[3]);
    TglPopClientAttrib();
    TglPopAttrib();
}

void OpenGLCanvas::Create()
{
    //prepare
    if (shader_flag_)
    {
        GetAllProcAddress();
        if (!CheckAllProcAddress())
        {
            shader_flag_ = false;
            return;
        } 
        program_id_ = LoadShader();
        texture_id_ = TglGetUniformLocation(program_id_, "myTextureSampler");  
        matrix_id_ = TglGetUniformLocation(program_id_, "MVP");
        CreateVertexAndUVBuffer();
    }
}

void OpenGLCanvas::UpdateVertexData(SimpleVertex * p, int n)
{
    for (int i = 0; i < n; ++i)
    {
        vertex_data_[i][0] = static_cast<GLfloat>(p[i].x);
        vertex_data_[i][1] = static_cast<GLfloat>(p[i].y);
        vertex_data_[i][2] = static_cast<GLfloat>(p[i].z);
        uv_data_[i][0] = static_cast<GLfloat>(p[i].u);
        uv_data_[i][1] = static_cast<GLfloat>(p[i].v);
    };
    TglBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    TglBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data_), vertex_data_, GL_STATIC_DRAW);
    TglBindBuffer(GL_ARRAY_BUFFER, uv_buffer_);
    TglBufferData(GL_ARRAY_BUFFER, sizeof(uv_data_), uv_data_, GL_STATIC_DRAW);

    TglEnableVertexAttribArray(0);
    TglBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    TglVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

void OpenGLCanvas::UpdateTexture(Bitmap & bmp_info)
{
    TglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bmp_info.width,
        bmp_info.height, GL_RGBA, GL_UNSIGNED_BYTE, bmp_info.bits);
}

void OpenGLCanvas::DrawBitmap(Bitmap & bmp_info, const Rect & dest)
{
    //
    uint32_t width = 0;
    uint32_t height = 0;
    GetTextureWH(width, height);
    //create or update
    if (bmp_info.width > width || bmp_info.height > height)
    {
        TglDeleteTextures(1, &texture_name_);
        CreateTexture(bmp_info.width, bmp_info.height);
    }

    GetTextureWH(width, height);
    //update data
    UpdateTexture(bmp_info);
    float max_u = (float)(bmp_info.width) / width;
    float max_v = (float)(bmp_info.height) / height;
    //draw
    if (shader_flag_)
    {
        //vertex data
        SimpleVertex p[4] = 
        {
            {rect_.left() + dest.left(), rect_.top() + dest.top(), 0, 0, 0},
            {rect_.left() + dest.right(), rect_.top() + dest.top(), 0, max_u, 0},
            {rect_.left() + dest.left(), rect_.top() + dest.bottom(), 0, 0, max_v},
            {rect_.left() + dest.right(), rect_.top() + dest.bottom(), 0, max_u, max_v},
        };
        UpdateVertexData(p, 4);
        //set color
        Float4 color;
        color.x = 0.0f;
        color.y = 0.0f;
        color.z = 0.0f;
        color.w = 0.0f;
        SetColor(color);
        //draw
        TglUniformMatrix4fv(matrix_id_, 1, GL_FALSE, &ortho_.m0.x); 
        TglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    } 
    else
    {//may be error
        TglBegin(GL_QUADS);
        TglColor3f(1.0, 1.0, 1.0);
        TglTexCoord2f(0.0, max_v); 
        TglVertex3f((float)(rect_.left() + dest.left()), 
                    (float)(rect_.top() + dest.top()), 0.0);

        TglTexCoord2f(max_u, max_v); 
        TglVertex3f((float)(rect_.left() + dest.right()), 
                     (float)(rect_.top() + dest.top()), 0.0);
        TglTexCoord2f(max_u, 0.0); 
        TglVertex3f((float)(rect_.left() + dest.left()), 
                    (float)(rect_.top() + dest.bottom()), 0.0);
        TglTexCoord2f(0.0, 0.0); 
        TglVertex3f((float)(rect_.left() + dest.right()), 
                     (float)(rect_.top() + dest.bottom()), 0.0);
        TglEnd();
    }
}

void OpenGLCanvas::DrawLine(const Point & p1, 
                                                  const Point & p2, 
                                                  const Paint & paint)
{
    TglBindTexture(GL_TEXTURE_2D, 0);
    if (shader_flag_)
    {
        //update vertex data
        SimpleVertex p[2] = 
        {
            {rect_.left() + p1.x(), rect_.top() + p1.y(), 0, 0, 0},
            {rect_.left() + p2.x(), rect_.top() + p2.y(), 0, 0, 0},
        };
        UpdateVertexData(p, 2);
        //color buffer
        Float4 color;
        TransFloat(paint.color, color);
        SetColor(color);
        //
        TglUniformMatrix4fv(matrix_id_, 1, GL_FALSE, &ortho_.m0.x);
        TglDrawArrays(GL_LINES, 0, 2);
    } 
    else
    {
        Float4 color;
        TransFloat(paint.color, color);
        TglBegin(GL_LINES);
        TglColor4f(color.x, color.y, color.z, color.w);
        TglVertex3f((float)p1.x(), (float)p1.y(), 0.0);
        TglVertex3f((float)p2.x(), (float)p2.y(), 0.0);
        TglEnd();
    }
}

void OpenGLCanvas::DrawPoint(const Point & p1, const Paint & paint)
{
    TglBindTexture(GL_TEXTURE_2D, 0);
    if (shader_flag_)
    {
        //update vertex data
        SimpleVertex p = {rect_.left() + p1.x(), rect_.top() + p1.y(), 0, 0, 0};

        UpdateVertexData(&p, 1);
        //color buffer
        Float4 color;
        TransFloat(paint.color, color);
        SetColor(color);
        //
        TglUniformMatrix4fv(matrix_id_, 1, GL_FALSE, &ortho_.m0.x);
        TglDrawArrays(GL_POINT, 0, 1);
    }
    else
    {
        Float4 color;
        TransFloat(paint.color, color);
        TglBegin(GL_POINT);
        TglColor4f(color.x, color.y, color.z, color.w);
        TglVertex3f((float)p1.x(), (float)p1.y(), 0.0);
        TglEnd();
    }
}

void OpenGLCanvas::DrawRect(const Rect & rect, const Paint & paint)
{
    //update vertex data
    SimpleVertex p[4] = 
    {
        {rect_.left() + rect.left(), rect_.top() + rect.top(), 0, 0, 0},
        {rect_.left() + rect.right(), rect_.top() + rect.top(), 0, 0, 0},
        {rect_.left() + rect.left(), rect_.top() + rect.bottom(), 0, 0, 0},
        {rect_.left() + rect.right(), rect_.top() + rect.bottom(), 0, 0, 0},
    };
    //draw
    TglBindTexture(GL_TEXTURE_2D, 0);
    if (shader_flag_)
    {
        UpdateVertexData(p, 4);
        //color buffer
        Float4 color;
        TransFloat(paint.color, color);
        SetColor(color);
        //
        TglUniformMatrix4fv(matrix_id_, 1, GL_FALSE, &ortho_.m0.x);
        TglDrawArrays(GL_LINE_LOOP, 0, 4);
    } 
    else
    {
        Float4 color;
        TransFloat(paint.color, color);
        TglBegin(GL_LINE_LOOP);
        TglColor4f(color.x, color.y, color.z, color.w);
        TglVertex3f((float)p[0].x, (float)p[0].y, 0.0);
        TglVertex3f((float)p[1].x, (float)p[1].y, 0.0);
        TglVertex3f((float)p[2].x, (float)p[2].y, 0.0);
        TglVertex3f((float)p[3].x, (float)p[3].y, 0.0);
        TglEnd();
    }
}
}
}
}