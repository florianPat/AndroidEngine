#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec4 color;
layout (location = 3) in vec4 mvMatrixSclRot;
layout (location = 4) in vec2 mvMatrixPos;

layout (std140, binding = 0) uniform u {
    mat4 proj;
} uniforms;

layout (location = 0) out vec2 v_texCoord;
layout (location = 1) out vec4 v_color;

/*
mat4 identity()
{
    return mat4(1.0);
}

mat4 translate(vec2 trans)
{
    mat4 result = identity();

    result[3][0] = trans.x;
    result[3][1] = trans.y;

    return result;
}

mat4 scale(vec2 scl)
{
    mat4 result = identity();

    result[0][0] = scl.x;
    result[1][1] = scl.y;

    return result;
}

mat4 rotate(float angle)
{
    float cosA = cos(angle);
    float sinA = sin(angle);

    mat4 result = identity();

    result[0][0] = cosA;
    result[1][0] = sinA;
    result[0][1] = -sinA;
    result[1][1] = cosA;

    return result;
}
*/

void main()
{
	v_texCoord = texCoord;
	v_color = color;

	mat4 mv = mat4( vec4(mvMatrixSclRot.x, mvMatrixSclRot.y, 0, 0 ),
	                vec4(mvMatrixSclRot.z, mvMatrixSclRot.w, 0, 0 ),
	                vec4(0, 0, 1.0, 0),
	                vec4(mvMatrixPos.x, mvMatrixPos.y, 0, 1.0) );
	mat4 mvp = uniforms.proj * mv;
	gl_Position = mvp * vec4(position, 0.0, 1.0);
}