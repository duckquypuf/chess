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
    std::vector<Piece> pieces;

    for (int i = 0; fen[i] != '\0'; i++)
    {
        char c = fen[i];

        if (c == ' ')
            break;

        if (c == '/')
            continue;

        if (c >= '1' && c <= '8')
        {
            int count = c - '0';
            for (int j = 0; j < count; j++)
                pieces.emplace_back(None, false);
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

        if (type != None)
            pieces.emplace_back(type, isWhite);
    }

    return pieces;
}