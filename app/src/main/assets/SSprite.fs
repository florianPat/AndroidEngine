#version 100

precision mediump float;

varying vec2 v_texCoord;
varying vec4 v_color;

uniform sampler2D tex;

void main()
{
	gl_FragColor = texture2D(tex, v_texCoord) * v_color;
}