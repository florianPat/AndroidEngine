#version 100

attribute vec2 position;
attribute vec2 texCoord;
attribute vec4 color;
attribute vec4 mvMatrixSclRot;
attribute vec2 mvMatrixPos;

varying vec4 v_color;

uniform mat4 u_proj;

void main()
{
	v_color = color;

	mat4 mv = mat4( vec4(mvMatrixSclRot.x, mvMatrixSclRot.y, 0, 0 ),
    	                vec4(mvMatrixSclRot.z, mvMatrixSclRot.w, 0, 0 ),
    	                vec4(0, 0, 1.0, 0),
    	                vec4(mvMatrixPos.x, mvMatrixPos.y, 0, 1.0) );
    mat4 mvp = u_proj * mv;
    gl_Position = mvp * vec4(position, 0.0, 1.0);
}