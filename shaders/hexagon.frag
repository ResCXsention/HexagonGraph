#version 330 core

out vec4 colour;

void main ()
{
	//colour = vec4(0.62, 0.66, 0.75, 1.0);
	vec2 position = gl_FragCoord.xy / vec2(900.0, 600.0); // normalize the coordinates
   vec2 center = vec2(0.5, 0.75); // center of the screen
    
   float dist = distance(position, center) * 4.0; // compute the distance from the center
    
   vec4 red = vec4(0.82, 0.86, 0.95, 1.0);
   vec4 blue = vec4(0.62, 0.66, 0.75, 1.0);
    
    colour = mix(red, blue, dist); // interpolate between the two colors
}

// courtesy ChatGPT
/*
In this example, the position vector is computed by normalizing the gl_FragCoord.xy coordinates with the dimensions of the screen (800x600 in this case). The center vector is set to (0.5, 0.5) which is the center of the screen.

The distance variable is then calculated using the distance() function which takes two vectors as arguments and returns the distance between them. In this case, it computes the distance between position and center.

Finally, the mix() function is used to interpolate between the red and blue colors based on the distance value. The closer the fragment is to the center of the screen, the closer its color will be to red. The further away it is, the closer it will be to blue. The output color is stored in the FragColor variable which will be used to color the current fragment.
*/