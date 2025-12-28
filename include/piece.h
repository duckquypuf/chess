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
};

class Piece
{
public:
    PieceType type;
    bool isWhite;
    bool hasMoved;
    uint8_t pos;

    Piece()
    {
        type = None;
        isWhite = true;
        hasMoved = false;
    }

    Piece(const PieceType _type, bool white, const uint8_t _pos)
    {
        type = _type;
        isWhite = white;
        pos = _pos;
        hasMoved = false;
    }
};

std::vector<Piece> loadFenString(const char *fen)
{
    std::vector<Piece> pieces;

    int file = 0;
    int rank = 0;

    for (int i = 0; fen[i] != '\0'; i++)
    {
        char c = fen[i];

        if (c == ' ')
            break;

        if (c == '/')
        {
            file = 0;
            rank++;
            continue;
        }

        if (c >= '1' && c <= '8')
        {
            int count = c - '0';
            for (int j = 0; j < count; j++)
            {
                uint8_t pos = rank * 8 + file;
                pieces.emplace_back(None, false, pos);
                file++;
            }
            continue;
        }

        bool isWhite = (c >= 'a' && c <= 'z');
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
        {
            uint8_t pos = rank * 8 + file;
            pieces.emplace_back(type, isWhite, pos);
            file++;
        }
    }

    return pieces;
}

struct Move
{
    int from;
    int to;

    Piece captured;

    int prevEnPassant;

    bool pieceHadMoved;

    bool wasCastling;
    int rookFrom;
    int rookTo;
    bool rookHadMoved;

    bool wasEnPassant;
    int enPassantPawnSquare;

    bool wasPromotion;
    PieceType promotedFrom;

    int prevWhiteKing;
    int prevBlackKing;

    int prevLastPawnOrCapture;

    Move(int from = -1, int to = -1)
    {

    }
};