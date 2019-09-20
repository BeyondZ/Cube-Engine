#version 130

uniform sampler2D TU_colorBuffer;
varying vec2 v_texcoord;
float getLuminace(vec3 color)
{
	return color.r * 0.3 + color.g* 0.59 + color.b * 0.11;
}
void main()
{
	vec4 color = texture2D(TU_colorBuffer, v_texcoord);
	gl_FragColor = vec4(vec3(getLuminace(color.rgb)), 1.0);
}
