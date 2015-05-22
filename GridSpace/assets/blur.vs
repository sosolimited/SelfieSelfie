#include "common.glsl"

uniform float uTextureIndex;

attribute vec4  ciPosition;
attribute vec2  ciTexCoord0;

varying vec2 vTexCoord;

void main()
{
	gl_Position = ciPosition; // we specify position in NDC
	vTexCoord = offsetTextureCoordinate( ciTexCoord0, uTextureIndex );
}
