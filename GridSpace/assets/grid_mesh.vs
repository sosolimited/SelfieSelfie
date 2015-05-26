precision mediump float;

uniform mat4 ciModelViewProjection;
uniform lowp sampler2D uVideo;
uniform float uFrameIndex;

attribute vec4  ciPosition;
attribute vec2  ciTexCoord0;
attribute float FrameIndex;

varying vec4 vColor;

#include "common.glsl"

void main()
{
	float index = wrappedIndex(uFrameIndex, FrameIndex);
	vec2 color_coord = offsetTextureCoordinate(ciTexCoord0, index);
	vec4 color = texture2D(uVideo, color_coord);

	gl_Position = ciModelViewProjection * ciPosition;
	vColor = color * (1.0 - FrameIndex); // fade out a bit in distance
}
