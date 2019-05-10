#version 100

attribute vec2 position;
attribute vec2 texCoord;
attribute vec4 color;
attribute vec4 mvMatrixSclRot;
attribute vec2 mvMatrixPos;

varying vec2 v_texCoord;
varying vec4 v_color;

uniform mat4 u_proj;

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

void main()
{
	v_texCoord = texCoord;
	v_color = color;

	mat4 mv = mat4( vec4(mvMatrixSclRot.x, mvMatrixSclRot.y, 0, 0 ),
	                vec4(mvMatrixSclRot.z, mvMatrixSclRot.w, 0, 0 ),
	                vec4(0, 0, 1.0, 0),
	                vec4(mvMatrixPos.x, mvMatrixPos.y, 0, 1.0) );
	mat4 mvp = u_proj * mv;
	gl_Position = mvp * vec4(position, 0.0, 1.0);
}