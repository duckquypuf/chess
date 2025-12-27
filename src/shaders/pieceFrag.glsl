#version 410 core

in vec2 vUV;
uniform sampler2D pieceTex;

out vec4 FragColour;

void main()
{
    vec4 col = texture(pieceTex, vUV);
    if (col.a < 0.1) discard;
    FragColour = col;
}