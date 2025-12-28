#pragma once

#include <array>
#include <algorithm>
#include <vector>

#include "piece.h"

class Board;

struct Move
{
    int from;
    int to;

    constexpr Move(int f = -1, int t = -1) noexcept
        : from(f), to(t) {}
};

namespace MoveGen
{
    // --- PRECOMPUTED DATA ---
    inline constexpr int BoardSize = 8;
    inline constexpr int SquareCount = 64;
    inline constexpr int DirectionCount = 8;

    inline constexpr std::array<int, DirectionCount> directionOffsets
    {
        8, -8, -1, 1, 7, -7, 9, -9
    };

    inline std::array<std::array<int, DirectionCount>, SquareCount> numSquaresToEdge{};

    inline void precomputeMoveData() noexcept
    {
        for (int file = 0; file < BoardSize; ++file)
        {
            for (int rank = 0; rank < BoardSize; ++rank)
            {
                const int north = 7 - rank;
                const int south = rank;
                const int west = file;
                const int east = 7 - file;

                const int squareIndex = rank * BoardSize + file;

                numSquaresToEdge[squareIndex] = 
                {
                    north,
                    south,
                    west,
                    east,
                    std::min(north, west),
                    std::min(south, east),
                    std::min(north, east),
                    std::min(south, west)
                };
            }
        }
    }

    // --- RUNTIME DATA ---
    inline std::vector<Move> moves;

    std::vector<Move> generateMoves(const Board &board);
    std::vector<Move> generateSlidingMoves(const Board &board, int startSquare, PieceType type);
};