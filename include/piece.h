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
    uint8_t pos;

    Piece(const PieceType _type, bool white, const uint8_t _pos)
    {
        type = _type;
        isWhite = white;
        pos = _pos;
    }
};

std::vector<Piece> loadFenString(const char *fen)
{
    std::vector<Piece> pieces;

    int file = 0;
    int rank = 7;

    for (int i = 0; fen[i] != '\0'; i++)
    {
        char c = fen[i];

        if (c == ' ')
            break;

        if (c == '/')
        {
            file = 0;
            rank--;
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
        {
            uint8_t pos = rank * 8 + file;
            pieces.emplace_back(type, isWhite, pos);
            file++;
        }
    }

    return pieces;
}