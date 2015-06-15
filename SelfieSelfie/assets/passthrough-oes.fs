#version 100
#extension GL_OES_EGL_image_external : require

precision lowp float;

uniform samplerExternalOES uSampler;

varying vec2 vTexCoord;

void main( void )
{
	gl_FragColor = texture2D( uSampler, vTexCoord );

	#ifdef HELLO
		gl_FragColor = vec4( 1.0, 0.0, 1.0, 1.0 );
	#endif
}
