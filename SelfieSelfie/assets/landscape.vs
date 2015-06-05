precision mediump float;

uniform mat4						ciModelViewProjection;
uniform mat3            ciNormalMatrix;
uniform lowp sampler2D	uBlurredTexture;
uniform float						uCurrentFrame;

attribute vec3  ciPosition;
attribute vec3	ciNormal;
attribute vec2  ciTexCoord0; // coord for vertex offset and interpolation
attribute vec2	ciTexCoord1; // coord for vertex color to enable flat shading on ES2
attribute vec2  ciTexCoord2;

attribute float FrameOffset;
attribute float ColorWeight;

attribute float	DeformScaling;
attribute float DeformFrameOffset;

varying vec2 vTexCoord;
varying vec4 vColor;
varying float vColorWeight;
varying vec3 vNormal;

const vec3 kLightDirection = vec3( 0, 1, 0 );

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

vec3 calcOffset( vec2 tex_coord )
{
	float t = clamp( length(texture2D(uBlurredTexture, tex_coord).rgb), 0.0, 1.0 );
	float offset = mix( 0.6, 0.0, t );
	return ciNormal * DeformScaling * offset;
}

void main()
{
	float index = wrappedIndex(uCurrentFrame, FrameOffset );
  float deform_index = wrappedIndex(uCurrentFrame, DeformFrameOffset );
  vec2 frag_color_coord = offsetTextureCoordinate( ciTexCoord0, index );
	vec2 flat_color_coord = offsetTextureCoordinate( ciTexCoord1, index );
  vec2 offset_coord = offsetTextureCoordinate( ciTexCoord2, deform_index );

	gl_Position = ciModelViewProjection * vec4(ciPosition + calcOffset(offset_coord), 1.0);
	vColor = vec4( clamp( texture2D( uBlurredTexture, flat_color_coord ).rgb * 1.3, 0.0, 1.0 ), 1.0 );
	vTexCoord = frag_color_coord;
	vColorWeight = ColorWeight;

  vNormal = ciNormal;
}
