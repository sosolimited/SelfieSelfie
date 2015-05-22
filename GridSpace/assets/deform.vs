precision mediump float;

uniform mat4 ciModelViewProjection;
uniform lowp sampler2D uVideo;
uniform float uFrameIndex;

attribute vec3  ciPosition;
attribute vec3	ciNormal;
attribute vec2  ciTexCoord0;

varying vec2 vTexCoord;
varying vec4 vColor;

#include "common.glsl"

void main()
{
	vec2 tc = offsetTextureCoordinate( ciTexCoord0, uFrameIndex );
	vec4 color = texture2D( uVideo, tc );
	float offset = clamp( length(color.rgb), 0.0, 1.0 );
	offset = mix( -4.0, 1.0, offset );
	gl_Position = ciModelViewProjection * vec4(ciPosition + ciNormal * offset, 1.0);
	vTexCoord = tc;
	vColor = color;
}
