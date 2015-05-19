precision mediump float;

uniform mat4 ciModelViewProjection;
uniform lowp sampler2D uVideo;

attribute vec3  ciPosition;
attribute vec3	ciNormal;
attribute vec2  ciTexCoord0;

varying vec2 vTexCoord;

void main()
{
	float offset = clamp( length(texture2D( uVideo, ciTexCoord0 ).rgb), 0.0, 1.0 );
	offset = mix( -4.0, 0.0, offset );
	gl_Position = ciModelViewProjection * vec4(ciPosition + ciNormal * offset, 1.0);

	vTexCoord = ciTexCoord0;
}
