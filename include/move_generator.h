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
    bool castling;

    Piece capturedPiece;
    int prevEnPassant;
    bool wasEnPassant;
    int epCapturedSquare;

    Move(int f = -1, int t = -1, bool c = false) noexcept
        : from(f), to(t), castling(c),
          capturedPiece(),
          prevEnPassant(-1),
          wasEnPassant(false),
          epCapturedSquare(-1) {}
};

namespace MoveGen
{
    // --- PRECOMPUTED DATA ---
    inline constexpr int BoardSize = 8;
    inline constexpr int SquareCount = 64;
    inline constexpr int DirectionCount = 8;

    inline constexpr std::array<int, DirectionCount> directionOffsets{
        8, -8, -1, 1, 7, -7, 9, -9};

    // Knight move offsets
    inline constexpr std::array<int, 8> knightOffsets{
        17, 15, 10, 6, -6, -10, -15, -17};

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
                        std::min(south, west)};
            }
        }
    }

    // --- RUNTIME DATA ---
    inline std::vector<Move> moves;

    // Helper functions
    inline int getFile(int square) { return square % 8; }
    inline int getRank(int square) { return square / 8; }

    // Move generation functions
    std::vector<Move> generateLegalMoves(Board *board);
    std::vector<Move> generateMoves(const Board *board);
    void generateSlidingMoves(const Board *board, int startSquare, const Piece &piece);
    void generateKnightMoves(const Board *board, int startSquare, const Piece &piece);
    void generateKingMoves(const Board *board, int startSquare, const Piece &piece);
    void generatePawnMoves(const Board *board, int startSquare, const Piece &piece);
};