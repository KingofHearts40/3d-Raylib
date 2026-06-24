#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

void main()
{
    // Output solid red, keeping alpha (transparency) intact
    finalColor = vec4(1.0, 0.0, 0.0, 1.0); 
}