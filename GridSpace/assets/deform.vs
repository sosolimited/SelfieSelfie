precision mediump float;

uniform mat4 ciModelViewProjection;
uniform lowp sampler2D uVideo;
uniform float uFrameIndex;

attribute vec3  ciPosition;
attribute vec3	ciNormal;
attribute vec2  ciTexCoord0; // coord for vertex offset
attribute vec2	ciTexCoord1; // coord for vertex color
attribute float FrameIndex;

varying vec2 vTexCoord;
varying vec4 vColor;

#include "common.glsl"

void main()
{
	float index = wrappedIndex(uFrameIndex, FrameIndex);
	vec2 offset_coord = offsetTextureCoordinate( ciTexCoord0, index );
	vec2 color_coord = offsetTextureCoordinate( ciTexCoord1, index );
	float offset = clamp( length(texture2D(uVideo, offset_coord).rgb), 0.0, 1.0 );
	offset = mix( -4.0, 1.0, offset ); // note that normal can be > 1.0.

	gl_Position = ciModelViewProjection * vec4(ciPosition + ciNormal * offset, 1.0);
	vColor = texture2D( uVideo, color_coord );
}
