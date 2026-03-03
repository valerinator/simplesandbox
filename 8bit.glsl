#version 330

in vec2 texcoord;
uniform sampler2D tex;

vec4 default_post_processing(vec4 c);

vec4 window_shader() {
	vec2 texsize = textureSize(tex, 0);
	vec4 color = texture2D(tex, texcoord / texsize, 0);

	color = vec4(trunc(color.rg*7.9)/7,trunc(color.b*3.9)/3, color.a);

	return default_post_processing(color);
}
