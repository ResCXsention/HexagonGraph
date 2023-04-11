#version 330 core

out vec4 colour;

void main ()
{
	vec2 position = gl_FragCoord.xy / vec2(900.0, 600.0);
   vec2 center = vec2(0.5, 0.75);
    
   float dist = distance(position, center) * 4.0;
    
   vec4 red = vec4(0.82, 0.86, 0.95, 1.0);
   vec4 blue = vec4(0.62, 0.66, 0.75, 1.0);
    
    colour = mix(red, blue, dist);
}