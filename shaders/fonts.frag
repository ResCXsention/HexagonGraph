#version 330 core
in vec2 textureCoords;
out vec4 colour;

uniform sampler2D text;

void main ()
{
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, textureCoords).r);
	colour = vec4(0.5, 0.5, 0.5, 1.0) * sampled;
}