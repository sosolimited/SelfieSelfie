precision mediump float;

uniform mat4						ciModelViewProjection;
uniform mat3            ciNormalMatrix;
uniform lowp sampler2D	uBlurredTexture;
uniform float						uCurrentFrame;

attribute vec3  ciPosition;
attribute vec3	ciNormal;
attribute vec2  ciTexCoord0; // coord for vertex offset and interpolation
attribute vec2	ciTexCoord1; // coord for vertex color to enable flat shading on ES2
attribute float	DeformScaling;
attribute float FrameOffset;
attribute float ColorWeight;
attribute float DeformFrameOffset;

varying vec2 vTexCoord;
varying vec4 vColor;
varying float vColorWeight;
varying vec3 vNormal;

const vec3 kLightDirection = vec3( 0, 1, 0 );

///
/// Formerly in "common.glsl"
///
uniform vec2 uGridSize;

vec2 getTextureOffset( float index )
{
  float column = mod(index, uGridSize.x);
  float row = floor(index / uGridSize.x);

  return vec2(column, row) / uGridSize;
}

vec2 offsetTextureCoordinate( vec2 coord, float index )
{
  return (coord / uGridSize) + getTextureOffset( index );
}

float wrappedIndex( float baseIndex, float offset )
{
  return floor(mod(baseIndex - offset, uGridSize.x * uGridSize.y));
}

///
/// end "common.glsl"
///

vec3 calcOffset( vec2 tex_coord )
{
	float t = clamp( length(texture2D(uBlurredTexture, tex_coord).rgb), 0.0, 1.0 );
	float offset = mix( -3.0, 0.0, t );
	return ciNormal * offset;
}

void main()
{
	float index = wrappedIndex(uCurrentFrame, 0.0 );
  float deform_index = wrappedIndex(uCurrentFrame, 0.0 );
	vec2 offset_coord = offsetTextureCoordinate( ciTexCoord0, deform_index );
  vec2 frag_color_coord = offsetTextureCoordinate( ciTexCoord0, index );
	vec2 color_coord = offsetTextureCoordinate( ciTexCoord1, index );

	gl_Position = ciModelViewProjection * vec4(ciPosition + calcOffset(offset_coord), 1.0);
	vColor = vec4( clamp( texture2D( uBlurredTexture, color_coord ).rgb * 1.3, 0.0, 1.0 ), 1.0 );
	vTexCoord = frag_color_coord;
	vColorWeight = ColorWeight;

  vNormal = ciNormal;
}
