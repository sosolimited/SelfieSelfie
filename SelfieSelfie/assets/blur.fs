#version 100
#extension GL_OES_EGL_image_external : require

precision highp float;

const vec2 cellDimensions = vec2( 640, 480 );
const vec2 gridSize = vec2( 8, 8 );
// Size of a single texel in an input camera image.
const vec2 texelSize = vec2( 1.0 ) / (cellDimensions);

uniform samplerExternalOES	uSampler;

varying vec2 vTexCoord;

void main( void )
{
	vec4 sum = vec4( 0.0 );
	sum += texture2D( uSampler, vTexCoord + -10.0 * texelSize ) * 0.009167927656011385;
	sum += texture2D( uSampler, vTexCoord +  -9.0 * texelSize ) * 0.014053461291849008;
	sum += texture2D( uSampler, vTexCoord +  -8.0 * texelSize ) * 0.020595286319257878;
	sum += texture2D( uSampler, vTexCoord +  -7.0 * texelSize ) * 0.028855245532226279;
	sum += texture2D( uSampler, vTexCoord +  -6.0 * texelSize ) * 0.038650411513543079;
	sum += texture2D( uSampler, vTexCoord +  -5.0 * texelSize ) * 0.049494378859311142;
	sum += texture2D( uSampler, vTexCoord +  -4.0 * texelSize ) * 0.060594058578763078;
	sum += texture2D( uSampler, vTexCoord +  -3.0 * texelSize ) * 0.070921288047096992;
	sum += texture2D( uSampler, vTexCoord +  -2.0 * texelSize ) * 0.079358891804948081;
	sum += texture2D( uSampler, vTexCoord +  -1.0 * texelSize ) * 0.084895951965930902;
	sum += texture2D( uSampler, vTexCoord +   0.0 * texelSize ) * 0.086826196862124602;
	sum += texture2D( uSampler, vTexCoord +   1.0 * texelSize ) * 0.084895951965930902;
	sum += texture2D( uSampler, vTexCoord +   2.0 * texelSize ) * 0.079358891804948081;
	sum += texture2D( uSampler, vTexCoord +   3.0 * texelSize ) * 0.070921288047096992;
	sum += texture2D( uSampler, vTexCoord +   4.0 * texelSize ) * 0.060594058578763078;
	sum += texture2D( uSampler, vTexCoord +   5.0 * texelSize ) * 0.049494378859311142;
	sum += texture2D( uSampler, vTexCoord +   6.0 * texelSize ) * 0.038650411513543079;
	sum += texture2D( uSampler, vTexCoord +   7.0 * texelSize ) * 0.028855245532226279;
	sum += texture2D( uSampler, vTexCoord +   8.0 * texelSize ) * 0.020595286319257878;
	sum += texture2D( uSampler, vTexCoord +   9.0 * texelSize ) * 0.014053461291849008;
	sum += texture2D( uSampler, vTexCoord +  10.0 * texelSize ) * 0.009167927656011385;

	gl_FragColor = sum;
//  gl_FragColor = vec4( vTexCoord, 0.5, 1.0 );
}
