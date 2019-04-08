#version 100

attribute vec2 position;

varying vec4 v_color;

uniform mat4 u_mvp;
uniform vec4 u_color;

void main()
{
	v_color = u_color;
	gl_Position = u_mvp * vec4(position, 1.0, 1.0);
}