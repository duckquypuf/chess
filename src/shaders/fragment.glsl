#version 410 core

out vec4 FragColour;

uniform vec2 screenRes;
uniform float boardSize;

uniform vec3 whiteColour, blackColour;
uniform vec3 selectedColour;

uniform int selectedSquare;

uniform int numLegalMoves;
uniform int legalMoves[32];

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
        
        int file = int(floor((gl_FragCoord.x - minBound.x) / squareSize));
        int rank = int(floor((gl_FragCoord.y - minBound.y) / squareSize));
        
        bool isWhite = (file + rank) % 2 == 1;

        int squareIndex = rank * 8 + file;
        bool isSelected = squareIndex == selectedSquare;

        bool isLegalMove = false;
        for(int i = 0; i < numLegalMoves; i++)
        {
            if(legalMoves[i] == squareIndex)
            {
                isLegalMove = true;
                break;
            }
        }

        colour = vec3((isWhite ? whiteColour : blackColour));

        if(isSelected)
        {
            colour = mix(colour, selectedColour, 0.7);
        }
        else if(isLegalMove)
        {
            colour = mix(colour, vec3(0.919, 0.250, 0.299), 0.6);
        }
    }

    FragColour = vec4(colour, 1.0);
}