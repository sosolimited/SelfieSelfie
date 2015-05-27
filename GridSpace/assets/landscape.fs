precision mediump float;

uniform lowp sampler2D uClearTexture;

varying vec2 vTexCoord;
varying vec4 vColor;

void main()
{
	vec4 tex_color = texture2D( uClearTexture, vTexCoord ).rgba;

	gl_FragColor = tex_color;
}
