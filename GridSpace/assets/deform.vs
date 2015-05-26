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

const vec3 kLightDirection = vec3( 0, 1, 0 );

#include "common.glsl"

float hemisphereMix( vec3 normal )
{
	vec3 light_dir = normalize( kLightDirection );
	float cos_theta = dot(normal, light_dir);
	float a = cos_theta * 0.5 + 0.5;

	return a;
}

vec3 calcOffset( vec2 tex_coord )
{
	float t = clamp( length(texture2D(uVideo, tex_coord).rgb), 0.0, 1.0 );
	float offset = mix( -5.0, 0.0, t ); // note that normal can be > 1.0.
	return ciNormal * offset;
}

void main()
{
	float index = wrappedIndex(uFrameIndex, FrameIndex);
	vec2 offset_coord = offsetTextureCoordinate( ciTexCoord0, index );
	vec2 color_coord = offsetTextureCoordinate( ciTexCoord1, index );

	gl_Position = ciModelViewProjection * vec4(ciPosition + calcOffset(offset_coord), 1.0);
	vColor = vec4( clamp( texture2D( uVideo, color_coord ).rgb * 1.3, 0.0, 1.0 ), 1.0 );

	// irregularity of grid causes incorrect normals due to squash and stretch.
	// visually, they produce good enough results
	float texture_step = 1.0 / 64.0;
	float spatial_step = mix( 0.5, 2.0, FrameIndex );
	vec3 a = spatial_step * vec3(-1, 0, 0 ) + calcOffset( offsetTextureCoordinate( ciTexCoord1 - vec2( texture_step, 0 ), index ) );
	vec3 c = spatial_step * vec3( 1, 0, 0 ) + calcOffset( offsetTextureCoordinate( ciTexCoord1 + vec2( texture_step, 0 ), index ) );
	vec3 b = spatial_step * vec3( 0, 0, 1 ) + calcOffset( offsetTextureCoordinate( ciTexCoord1 + vec2( 0, texture_step ), index ) );
	vec3 d = spatial_step * vec3( 0, 0,-1 ) + calcOffset( offsetTextureCoordinate( ciTexCoord1 - vec2( 0, texture_step ), index ) );
	vec3 ac = c - a;
	vec3 bd = d - b;
	vec3 normal = normalize( cross( ac, bd ) );

	float shading = hemisphereMix(normal);
	vColor.rgb *= vec3(shading);
}
