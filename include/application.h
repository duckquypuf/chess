#pragma once

#include "window.h"
#include "renderer.h"
#include "board.h"

class Application
{
public:
    Window window = Window("Chess", 1440, 900);
    Renderer renderer = Renderer("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl", window, "../src/shaders/pieceVert.glsl", "../src/shaders/pieceFrag.glsl");
    Camera camera;
    Board board;

    void run()
    {
        while(!glfwWindowShouldClose(window.window))
        {
            window.processInput();
            board.handleInput(window, window.getInput(), renderer.smallestDimension);
            renderer.beginFrame();
            renderer.render(camera, window, board.pieces, board.selectedSquare, board.legalMoves);
            window.update();
        }
    }
};