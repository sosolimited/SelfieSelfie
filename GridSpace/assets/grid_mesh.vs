precision mediump float;

uniform mat4 ciModelViewProjection;
uniform lowp sampler2D uVideo;
uniform float uFrameIndex;

attribute vec4  ciPosition;
attribute vec2  ciTexCoord0;
attribute vec3	PositionOffset;
attribute float FrameIndex;

varying vec4 vColor;

#include "common.glsl"

void main()
{
	float index = wrappedIndex(uFrameIndex, FrameIndex);
	vec2 color_coord = offsetTextureCoordinate(ciTexCoord0, index);
	vec4 color = texture2D(uVideo, color_coord);

	vec3 offset = PositionOffset * mix( 0.1, 1.0, length(color.rgb) / 3.0 );
	gl_Position = ciModelViewProjection * (ciPosition + vec4(offset, 0.0));
	vColor = color * (1.0 - FrameIndex); // fade out a bit in distance
}
