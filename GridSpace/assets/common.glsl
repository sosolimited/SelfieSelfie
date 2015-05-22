
precision highp float;

const vec2 cellDimensions = vec2( 480, 320 );
const vec2 gridSize = vec2( 8, 8 );
const vec2 texelSize = vec2( 1.0 ) / (cellDimensions * gridSize);

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
