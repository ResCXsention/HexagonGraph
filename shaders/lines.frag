#version 330 core

out vec4 colour;

uniform vec3 lineColour;

void main ()
{
	colour = vec4(lineColour.xyz, 1.0);
}