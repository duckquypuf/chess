#pragma once

#include "window.h"
#include "renderer.h"
#include "piece.h"

class Application
{
public:
    Window window = Window("Chess", 1440, 900);
    Renderer renderer = Renderer("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl", window, "../src/shaders/pieceVert.glsl", "../src/shaders/pieceFrag.glsl");
    Camera camera;

    std::vector<Piece> pieces = loadFenString("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    void run()
    {
        while(!glfwWindowShouldClose(window.window))
        {
            window.processInput();
            renderer.beginFrame();
            renderer.render(camera, window, pieces);
            window.update();
        }
    }
};