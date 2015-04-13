#ifndef MAKU_RENDER_OPENGL_SHADER_H_
#define MAKU_RENDER_OPENGL_SHADER_H_

namespace maku
{
namespace render
{


//vertex shader
#define SHADER_DECLARE(name, ver , sl) \
    const char name[] = "#version "###ver##" core\n"###sl

SHADER_DECLARE(vertex_shader, 330, 
// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec4 vertexColor;
// Output data ; will be interpolated for each fragment.
out vec2 UV;
out vec4 fragmentColor;
uniform mat4 MVP;
void main()
{
    gl_Position = MVP*vec4(vertexPosition_modelspace, 1);
 	UV = vertexUV;
    fragmentColor = vertexColor;
}
);

//fragment shader
#define SHADER_FRAGMENT(name, ver , sl) \
    const char name[] = "#version "###ver##" core\n"###sl

SHADER_FRAGMENT(fragment_shader, 330,
    in vec2 UV;
//int fragment color
in vec4 fragmentColor;
// Ouput data
out vec4 color;
// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

void main()
{
    //
    color =  texture( myTextureSampler, UV ).bgra + fragmentColor;
}
);

}
}


#endif