precision mediump float;

uniform mat4 ciModelViewProjection;

attribute vec3 ciPosition;
attribute vec3 PreviousPosition;
attribute float Size;
attribute vec2 ciTexCoord0;

varying vec2 vTexCoord;

void main()
{
	gl_Position = ciModelViewProjection * vec4(ciPosition, 1.0);
	gl_PointSize = Size;
}
