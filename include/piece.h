#pragma once

#include <stdint.h>
#include <vector>

enum PieceType
{
    None = -1,
    Pawn = 0,
    Knight = 1,
    Bishop = 2,
    Rook = 3,
    Queen = 4,
    King = 5,
};

class Piece
{
public:
    PieceType type;
    bool isWhite;
    bool hasMoved;

    Piece()
    {
        type = None;
        isWhite = true;
        hasMoved = false;
    }

    Piece(const PieceType _type, bool white)
    {
        type = _type;
        isWhite = white;
        hasMoved = false;
    }
};

namespace PieceData
{
    enum PieceValue
    {
        PawnValue = 100,
        KnightValue = 300,
        BishopValue = 300,
        RookValue = 500,
        QueenValue = 900,
        KingValue = 10000,
    };

    const int pawnTable[64] = 
    {
        0, 0, 0, 0, 0, 0, 0, 0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5, 5, 10, 25, 25, 10, 5, 5,
        0, 0, 0, 20, 20, 0, 0, 0,
        5, -5, -10, 0, 0, -10, -5, 5,
        5, 10, 10, -20, -20, 10, 10, 5,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    const int knightTable[64] = 
    {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20, 0, 0, 0, 0, -20, -40,
        -30, 0, 10, 15, 15, 10, 0, -30,
        -30, 5, 15, 20, 20, 15, 5, -30,
        -30, 0, 15, 20, 20, 15, 0, -30,
        -30, 5, 10, 15, 15, 10, 5, -30,
        -40, -20, 0, 5, 5, 0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50
    };

    constexpr bool IsRookOrQueen(PieceType piece)
    {
        return piece == Rook || piece == Queen;
    }

    constexpr bool IsBishopOrQueen(PieceType piece)
    {
        return piece == Bishop || piece == Queen;
    }

    constexpr bool IsSlidingPiece(PieceType piece)
    {
        return piece == Bishop || piece == Rook || piece == Queen;
    }
};

inline std::vector<Piece> loadFenString(const char *fen)
{
    std::vector<Piece> pieces(64);

    int rank = 7;
    int file = 0;

    for (int i = 0; fen[i] != '\0'; i++)
    {
        char c = fen[i];

        if (c == ' ')
            break;

        if (c == '/')
        {
            rank--;
            file = 0;
            continue;
        }

        if (c >= '1' && c <= '8')
        {
            int count = c - '0';
            for (int j = 0; j < count; j++)
            {
                pieces[rank * 8 + file] = Piece();
                file++;
            }
            continue;
        }

        bool isWhite = (c >= 'A' && c <= 'Z');
        char lc = tolower(c);

        PieceType type = None;

        switch (lc)
        {
        case 'p':
            type = Pawn;
            break;
        case 'n':
            type = Knight;
            break;
        case 'b':
            type = Bishop;
            break;
        case 'r':
            type = Rook;
            break;
        case 'q':
            type = Queen;
            break;
        case 'k':
            type = King;
            break;
        }

        pieces[rank * 8 + file] = Piece(type, isWhite);
        file++;
    }

    return pieces;
}