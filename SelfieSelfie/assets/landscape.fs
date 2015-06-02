precision mediump float;

uniform lowp sampler2D uClearTexture;

varying vec2 vTexCoord;
varying vec4 vColor;
varying float vColorWeight;

void main()
{
	vec4 tex_color = texture2D( uClearTexture, vTexCoord ).rgba;

	gl_FragColor = mix( tex_color, vColor, vColorWeight );
//	gl_FragColor = vec4( vTexCoord.rg, 0.5, 1.0 );
}
