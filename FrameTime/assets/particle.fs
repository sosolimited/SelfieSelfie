precision mediump float;

uniform lowp sampler2D uVideo;

varying vec2 vTexCoord;

void main()
{
	vec2 circle_pos = gl_PointCoord - vec2(0.5);
	float r2 = dot(circle_pos, circle_pos);
	float a = 1.0 - smoothstep(0.249, 0.25, r2);

	gl_FragColor = vec4(a);
}