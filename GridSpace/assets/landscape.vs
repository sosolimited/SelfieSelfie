precision mediump float;

uniform mat4						ciModelViewProjection;
uniform lowp sampler2D	uBlurredTexture;
uniform float						uCurrentFrame;

attribute vec3  ciPosition;
attribute vec3	ciNormal;
attribute vec2  ciTexCoord0; // coord for vertex offset and interpolation
attribute vec2	ciTexCoord1; // coord for vertex color to enable flat shading on ES2
attribute float	DeformScaling;
attribute float FrameOffset;

varying vec2 vTexCoord;
varying vec4 vColor;

#include "common.glsl"

vec3 calcOffset( vec2 tex_coord )
{
	float t = clamp( length(texture2D(uBlurredTexture, tex_coord).rgb), 0.0, 1.0 );
	float offset = mix( -5.0, 0.0, t ); // note that normal can be > 1.0.
	return ciNormal * DeformScaling * offset;
}

void main()
{
	float index = wrappedIndex(uCurrentFrame, FrameOffset);
	vec2 offset_coord = offsetTextureCoordinate( ciTexCoord0, index );
	vec2 color_coord = offsetTextureCoordinate( ciTexCoord1, index );

	gl_Position = ciModelViewProjection * vec4(ciPosition + calcOffset(offset_coord), 1.0);
	vColor = vec4( clamp( texture2D( uBlurredTexture, color_coord ).rgb * 1.3, 0.0, 1.0 ), 1.0 );
	vTexCoord = ciTexCoord0;
}
