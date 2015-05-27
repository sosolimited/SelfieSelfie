
precision highp float;

const vec2 cellDimensions = vec2( 480, 320 );
const vec2 gridSize = vec2( 8, 8 );
// Size of a single texel in an input camera image.
const vec2 texelSize = vec2( 1.0 ) / (cellDimensions * vec2(4));

vec2 getTextureOffset( float index )
{
	float column = mod(index, gridSize.x);
	float row = floor(index / gridSize.x);

	return vec2(column, row) / gridSize;
}

vec2 offsetTextureCoordinate( vec2 coord, float index )
{
	return (coord / gridSize) + getTextureOffset( index );
}

float wrappedIndex( float baseIndex, float offset )
{
	return floor(mod(baseIndex - offset, 64.0));
}
