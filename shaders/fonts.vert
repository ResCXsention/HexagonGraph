#version 330 core
layout (location = 0) in vec4 aPos; // vec2 position, vec2 texture
out vec2 textureCoords;

uniform mat4 projection = mat4(1.0);
uniform mat4 transform = mat4(1.0);

void main()
{
	 gl_Position = projection * transform *vec4(aPos.xy, 0.0, 1.0);
	 textureCoords = aPos.zw;
}