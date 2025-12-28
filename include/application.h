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
            board.updatePieces();
            window.processInput();

            board.handleInput(window, renderer.smallestDimension);
            //board.moveComputer(true);

            if(board.checkmate >= 0)
            {
                // End Game

                if(board.checkmate == 2)
                    std::cout << "STALEMATE" << std::endl;
                else if(board.checkmate == 1)
                    std::cout << "MATE FOR BLACK" << std::endl;
                else
                    std::cout << "MATE FOR WHITE" << std::endl;
            }

            board.moveComputer(false);

            if (board.checkmate >= 0)
            {
                // End Game

                if (board.checkmate == 2)
                    std::cout << "STALEMATE" << std::endl;
                else if (board.checkmate == 1)
                    std::cout << "MATE FOR BLACK" << std::endl;
                else
                    std::cout << "MATE FOR WHITE" << std::endl;
            }

            renderer.beginFrame();
            renderer.render(camera, window, board.pieces, board.selectedSquare, board.legalMoves, board.isDragging);
            window.update();
        }
    }
};