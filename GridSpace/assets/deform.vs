precision mediump float;

uniform mat4 ciModelViewProjection;
uniform lowp sampler2D uVideo;
uniform float uFrameIndex;

attribute vec3  ciPosition;
attribute vec3	ciNormal;
attribute vec2  ciTexCoord0;

varying vec2 vTexCoord;

#include "common.glsl"

void main()
{
	vec2 tc = offsetTextureCoordinate( ciTexCoord0, uFrameIndex );
	float offset = clamp( length(texture2D( uVideo, tc ).rgb), 0.0, 1.0 );
	offset = 0.0; // mix( -4.0, 1.0, offset );
	gl_Position = ciModelViewProjection * vec4(ciPosition + ciNormal * offset, 1.0);
	vTexCoord = tc;
}
