#pragma once

#include <iostream>
#include <string>

#include "window.h"
#include "renderer.h"
#include "board.h"
#include "Stopwatch.h"

inline void print(std::string text)
{
    std::cout << text;
}

inline void println(std::string text)
{
    std::cout << text << std::endl;
}

inline void getInt(int &num)
{
    std::cin >> num;
}

class DebugApplication
{
public:
    //Window window = Window("Chess", 1440, 900);
    //Renderer renderer = Renderer("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl", window, "../src/shaders/pieceVert.glsl", "../src/shaders/pieceFrag.glsl");
    Board board;

    Stopwatch sw;

    void run()
    {
        MoveGen::precomputeMoveData();

        println("--- RUNNING MOVE GENERATION TEST ---");

        int depth;

        print("Enter max depth (1+): ");
        getInt(depth);
        println("");

        for(int i = 1; i < depth; i++)
        {
            sw.start();
            print("Depth " + std::to_string(i) + ": ");
            print(std::to_string(board.moveGenerationTest(i)));
            sw.stop();
            print(" possible positions.");
            print("             Time took: ");
            long long ms = sw.getElapsedTimeMilliseconds();
            print(std::to_string(ms) + "ms. ");
            println("");
        }

        /*while (!glfwWindowShouldClose(window.window))
        {
            window.processInput();
            renderer.beginFrame();
            renderer.render(camera, window, board);
            board.drawPieces(renderer, window);
            window.update();
        }*/
    }
};