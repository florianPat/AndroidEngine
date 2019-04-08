#version 100

attribute vec2 position;
attribute vec2 texCoord;

varying vec2 v_texCoord;
varying vec4 v_color;

uniform mat4 u_mvp;
uniform vec4 u_color;

void main()
{
	v_texCoord = texCoord;
	v_color = u_color;
	gl_Position = u_mvp * vec4(position, 0.0, 1.0);
}