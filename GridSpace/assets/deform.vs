precision mediump float;

uniform mat4 ciModelViewProjection;
uniform lowp sampler2D uVideo;

attribute vec3  ciPosition;
attribute vec3	ciNormal;
attribute vec2  ciTexCoord0;

varying vec2 vTexCoord;

const vec2 cellDimensions = vec2( 480, 320 );
const vec2 gridSize = vec2( 8, 8 );

vec2 getTextureOffset( float index )
{
	float column = mod(index, gridSize.x);
	float row = floor(index / gridSize.x);

	return vec2(column, row) / gridSize;
}

void main()
{
	float offset = clamp( length(texture2D( uVideo, ciTexCoord0 ).rgb), 0.0, 1.0 );
	offset = 0.0; // mix( -4.0, 0.0, offset );
	gl_Position = ciModelViewProjection * vec4(ciPosition + ciNormal * offset, 1.0);

	vTexCoord = (ciTexCoord0 / gridSize) + getTextureOffset( 13.0 );
}
