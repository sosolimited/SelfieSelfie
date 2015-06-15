#version 100

precision lowp float;

#ifdef ANDROID_CAMERA_TEXTURE
#extension GL_OES_EGL_image_external : require
uniform samplerExternalOES uSampler
#else
uniform sampler2D	uSampler;
#endif

varying vec2 vTexCoord;

void main( void )
{
	gl_FragColor = texture2D( uSampler, vTexCoord );

	#ifdef HELLO
		gl_FragColor = vec4( 1.0, 0.0, 1.0, 1.0 );
	#endif
}
