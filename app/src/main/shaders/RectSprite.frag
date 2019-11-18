#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 v_texCoord;
layout (location = 1) in vec4 v_color;

layout (binding = 1) uniform sampler2D tex;

layout (location = 0) out vec4 fragColor;

void main()
{
	fragColor = texture(tex, v_texCoord) * v_color;
}