#version 100
#extension GL_OES_EGL_image_external : require

precision highp float;

uniform samplerExternalOES uSampler;
uniform vec2      uTexelSize;

varying vec2      vTexCoord;

void main()
{
  vec2 texelSize = uTexelSize;
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
}
