#pragma once

#include "window.h"
#include "renderer.h"
#include "board.h"

class Application
{
public:
    Window window = Window("Chess", 1440, 900);
    Renderer renderer = Renderer("src/shaders/vertex.glsl", "src/shaders/fragment.glsl", window, "src/shaders/pieceVert.glsl", "src/shaders/pieceFrag.glsl");
    Board board;

    void run()
    {
        MoveGen::precomputeMoveData();

        while(!glfwWindowShouldClose(window.window))
        {
            window.processInput();
            board.updateAnimation(window.deltaTime);

            if(!board.isAnimating)
            {
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

                board.handleInput(window, renderer.smallestDimension);
                //board.moveComputer(false);

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
            }

            renderer.beginFrame();
            renderer.render(window, board);
            board.drawPieces(renderer, window);
            window.update();
        }
    }
};