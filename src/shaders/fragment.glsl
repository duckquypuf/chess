#version 410 core

out vec4 FragColour;

uniform vec2 screenRes;
uniform float boardSize;

uniform vec3 whiteColour, blackColour;

void main()
{
    vec2 center = screenRes;
    float halfSize = boardSize;
    
    vec2 minBound = center - halfSize;
    vec2 maxBound = center + halfSize;

    vec3 colour = vec3(0.1); 

    if (gl_FragCoord.x >= minBound.x && gl_FragCoord.x <= maxBound.x &&
        gl_FragCoord.y >= minBound.y && gl_FragCoord.y <= maxBound.y)
    {
        float squareSize = boardSize / 4.0;
        
        int ix = int(floor((gl_FragCoord.x - minBound.x) / squareSize));
        int iy = int(floor((gl_FragCoord.y - minBound.y) / squareSize));
        
        bool isWhite = (ix + iy) % 2 == 1;
        colour = vec3(isWhite ? whiteColour : blackColour);
    }

    FragColour = vec4(colour, 1.0);
}