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
    Board board;
    Stopwatch sw;

    void run()
    {
        MoveGen::precomputeMoveData();

        println("=== CHESS ENGINE SEARCH BENCHMARK ===");
        println("");
        println("1. Move Generation Test (Perft)");
        println("2. Search Performance Test (Alpha-Beta)");
        println("3. Both Tests");
        println("");

        int choice;
        print("Select test: ");
        getInt(choice);
        println("");

        if (choice == 1 || choice == 3)
        {
            runMoveGenTest();
        }

        if (choice == 2 || choice == 3)
        {
            runSearchTest();
        }
    }

private:
    void runMoveGenTest()
    {
        println("--- MOVE GENERATION TEST (Perft) ---");

        int depth;
        print("Enter max depth (1-6): ");
        getInt(depth);
        println("");

        for (int i = 1; i <= depth; i++)
        {
            sw.start();
            int nodes = board.moveGenerationTest(i);
            sw.stop();

            long long ms = sw.getElapsedTimeMilliseconds();

            std::cout << "Depth " << i << ": "
                      << nodes << " nodes in "
                      << ms << "ms";

            if (ms > 0)
            {
                long long nps = (nodes * 1000LL) / ms;
                std::cout << " (" << nps << " nodes/sec)";
            }

            println("");
        }
        println("");
    }

    void runSearchTest()
    {
        println("--- ALPHA-BETA SEARCH TEST ---");

        int maxDepth;
        print("Enter max search depth (1-8): ");
        getInt(maxDepth);
        println("");

        const int infinity = 200000;

        for (int depth = 1; depth <= maxDepth; depth++)
        {
            // Get legal moves
            auto moves = MoveGen::generateLegalMoves(&board);

            if (moves.empty())
            {
                println("No legal moves!");
                break;
            }

            // Order moves for better performance
            board.orderMoves(moves);

            int bestValue = -infinity;
            Move bestMove;
            int nodesSearched = 0;

            sw.start();

            for (auto &move : moves)
            {
                board.makeMove(move);
                int val = -board.search(depth - 1, -infinity, infinity, 1);
                board.unmakeMove(move);

                nodesSearched++;

                if (val > bestValue)
                {
                    bestValue = val;
                    bestMove = move;
                }
            }

            sw.stop();
            long long ms = sw.getElapsedTimeMilliseconds();

            std::cout << "Depth " << depth << ": Best move = "
                      << board.squareToChessNotation(bestMove.from)
                      << board.squareToChessNotation(bestMove.to)
                      << " (eval: " << bestValue << ") ";

            std::cout << "Time: " << ms << "ms";

            if (ms > 0)
            {
                std::cout << " (" << (1000.0 / ms) << " moves/sec)";
            }

            println("");
        }
        println("");
    }
};