#version 330

in vec3 worldPos;

out vec4 finalColor;

uniform float gridSize;
uniform vec4 gridColor;
uniform vec4 backgroundColor;

void main()
{
    vec2 coord = worldPos.xz / gridSize;

    vec2 g = abs(fract(coord - 0.5) - 0.5);

    vec2 w = fwidth(coord);

    vec2 line = smoothstep(w, vec2(0.0), g);

    float alpha = max(line.x, line.y);

    finalColor = mix(backgroundColor, gridColor, alpha);
}