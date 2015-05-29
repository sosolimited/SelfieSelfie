precision mediump float;

uniform mat4 ciModelViewProjection;
uniform lowp sampler2D uVideo;
uniform float uFrameIndex;

attribute vec3  ciPosition;
attribute vec2  ciTexCoord0;
attribute float FrameIndex;

varying vec2 vTexCoord;

#include "common.glsl"

void main()
{
	float index = wrappedIndex(uFrameIndex, FrameIndex);
	vTexCoord = offsetTextureCoordinate( ciTexCoord0, index );
	gl_Position = ciModelViewProjection * vec4(ciPosition, 1.0);
}
