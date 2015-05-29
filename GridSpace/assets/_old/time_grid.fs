precision mediump float;

uniform lowp sampler2D uVideo;

varying vec2 vTexCoord;

void main()
{
	vec4 tex_color = texture2D( uVideo, vTexCoord ).rgba;

	gl_FragColor = tex_color;
}
