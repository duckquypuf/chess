#pragma once

#include "piece.h"
#include <algorithm>

class Board
{
public:
    std::vector<Piece> pieces = loadFenString("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    int selectedSquare = -1;
    bool isWhiteTurn = true;
    std::vector<int> legalMoves;
    bool isDragging = false;

    int whiteKing = -1;
    int blackKing = -1;

    Board()
    {
        findKings();
    }

    void handleInput(Window &window, float boardSize)
    {
        if (window.wasMouseJustPressed())
        {
            int square = window.screenToSquare(boardSize);

            if (square >= 0 && square < 64)
            {
                Piece &piece = pieces[square];
                if (piece.type != None && piece.isWhite == isWhiteTurn)
                {
                    selectedSquare = square;
                    legalMoves = generateLegalMoves(square);
                    isDragging = true;
                }
            }
        }
        else if (window.wasMouseJustReleased())
        {
            if (isDragging && selectedSquare != -1)
            {
                int targetSquare = window.screenToSquare(boardSize);

                if (targetSquare >= 0 && targetSquare < 64)
                {
                    if (std::find(legalMoves.begin(), legalMoves.end(), targetSquare) != legalMoves.end())
                    {
                        makeMove(selectedSquare, targetSquare);
                        isWhiteTurn = !isWhiteTurn;
                    }
                }
            }

            selectedSquare = -1;
            legalMoves.clear();
            isDragging = false;
        }
    }

    void makeMove(int fromSquare, int toSquare)
    {
        Piece &movingPiece = pieces[fromSquare];
        Piece &targetPiece = pieces[toSquare];

        if (movingPiece.type == King)
        {
            if (movingPiece.isWhite)
                whiteKing = toSquare;
            else
                blackKing = toSquare;
        }

        targetPiece.type = movingPiece.type;
        targetPiece.isWhite = movingPiece.isWhite;
        targetPiece.pos = toSquare;

        movingPiece.type = None;
    }

    void unmakeMove(int fromSquare, int toSquare, Piece capturedPiece)
    {
        Piece &movingPiece = pieces[toSquare];
        Piece &originalSquare = pieces[fromSquare];

        if (movingPiece.type == King)
        {
            if (movingPiece.isWhite)
                whiteKing = fromSquare;
            else
                blackKing = fromSquare;
        }

        originalSquare.type = movingPiece.type;
        originalSquare.isWhite = movingPiece.isWhite;
        originalSquare.pos = fromSquare;

        pieces[toSquare] = capturedPiece;
    }

    std::vector<int> generateLegalMoves(int square)
    {
        std::vector<int> pseudoLegalMoves = generatePseudoLegalMoves(square);
        std::vector<int> legalMoves;

        Piece &piece = pieces[square];

        for (int targetSquare : pseudoLegalMoves)
        {
            Piece capturedPiece = pieces[targetSquare];

            makeMove(square, targetSquare);

            bool inCheck = isKingInCheck(piece.isWhite);

            unmakeMove(square, targetSquare, capturedPiece);

            if (!inCheck)
            {
                legalMoves.push_back(targetSquare);
            }
        }

        return legalMoves;
    }


    std::vector<int> generatePseudoLegalMoves(int square)
    {
        std::vector<int> moves;
        Piece &piece = pieces[square];

        switch (piece.type)
        {
        case Pawn:
            moves = generatePawnMoves(square, piece.isWhite);
            break;
        case Knight:
            moves = generateKnightMoves(square, piece.isWhite);
            break;
        case Bishop:
            moves = generateBishopMoves(square, piece.isWhite);
            break;
        case Rook:
            moves = generateRookMoves(square, piece.isWhite);
            break;
        case Queen:
            moves = generateQueenMoves(square, piece.isWhite);
            break;
        case King:
            moves = generateKingMoves(square, piece.isWhite);
            break;
        case None:
            break;
        }

        return moves;
    }

    void findKings()
    {
        bool foundWhite = false;
        bool foundBlack = false;

        for (int i = 0; i < 64; i++)
        {
            Piece &piece = pieces[i];

            if (piece.type == King)
            {
                if (piece.isWhite)
                {
                    foundWhite = true;
                    whiteKing = i;
                }
                else
                {
                    foundBlack = true;
                    blackKing = i;
                }
            }

            if (foundWhite && foundBlack)
                break;
        }
    }

    bool isSquareAttacked(int square, bool byWhite)
    {
        for (int i = 0; i < 64; i++)
        {
            Piece &piece = pieces[i];

            if (piece.type == None || piece.isWhite != byWhite)
                continue;

            std::vector<int> moves = generatePseudoLegalMoves(i);

            if (std::find(moves.begin(), moves.end(), square) != moves.end())
            {
                return true;
            }
        }
        return false;
    }

    bool isKingInCheck(bool whiteKing)
    {
        int kingSquare = whiteKing ? this->whiteKing : this->blackKing;
        return isSquareAttacked(kingSquare, !whiteKing);
    }

private:
    bool isValidMove(int square, bool isWhite)
    {
        if (square < 0 || square > 63)
            return false;

        Piece &target = pieces[square];
        return target.type == None || target.isWhite != isWhite;
    }

    bool isOccupied(int square)
    {
        return square >= 0 && square <= 63 && pieces[square].type != None;
    }

    int getFile(int square) { return square % 8; }
    int getRank(int square) { return square / 8; }

    void addSlidingMoves(std::vector<int> &moves, int square, bool isWhite, int dirOffset)
    {
        int startFile = getFile(square);
        int startRank = getRank(square);
        int current = square + dirOffset;

        while (current >= 0 && current <= 63)
        {
            int currentFile = getFile(current);
            int currentRank = getRank(current);

            if (dirOffset == 1 && currentFile < startFile)
                break;
            if (dirOffset == -1 && currentFile > startFile)
                break;
            if (dirOffset == 8 && currentRank <= startRank)
                break;
            if (dirOffset == -8 && currentRank >= startRank)
                break;
            if (dirOffset == 9 && (currentFile <= startFile || currentRank <= startRank))
                break;
            if (dirOffset == 7 && (currentFile >= startFile || currentRank <= startRank))
                break;
            if (dirOffset == -7 && (currentFile <= startFile || currentRank >= startRank))
                break;
            if (dirOffset == -9 && (currentFile >= startFile || currentRank >= startRank))
                break;

            Piece &target = pieces[current];

            if (target.type == None)
            {
                moves.push_back(current);
            }
            else if (target.isWhite != isWhite)
            {
                moves.push_back(current);
                break;
            }
            else
            {
                break;
            }

            startFile = currentFile;
            startRank = currentRank;
            current += dirOffset;
        }
    }

public:
    std::vector<int> generatePawnMoves(int square, bool isWhite)
    {
        std::vector<int> moves;
        int file = getFile(square);
        int rank = getRank(square);
        int direction = isWhite ? 1 : -1;
        int startRank = isWhite ? 1 : 6;

        int forward = square + (direction * 8);
        if (forward >= 0 && forward <= 63 && pieces[forward].type == None)
        {
            moves.push_back(forward);

            if (rank == startRank)
            {
                int doubleForward = square + (direction * 16);
                if (pieces[doubleForward].type == None)
                {
                    moves.push_back(doubleForward);
                }
            }
        }

        int captureLeft = square + (direction * 8) - 1;
        int captureRight = square + (direction * 8) + 1;

        if (file > 0 && captureLeft >= 0 && captureLeft <= 63)
        {
            Piece &target = pieces[captureLeft];
            if (target.type != None && target.isWhite != isWhite)
            {
                moves.push_back(captureLeft);
            }
        }

        if (file < 7 && captureRight >= 0 && captureRight <= 63)
        {
            Piece &target = pieces[captureRight];
            if (target.type != None && target.isWhite != isWhite)
            {
                moves.push_back(captureRight);
            }
        }

        return moves;
    }

    std::vector<int> generateKnightMoves(int square, bool isWhite)
    {
        std::vector<int> moves;
        int file = getFile(square);
        int rank = getRank(square);

        int offsets[8] = {17, 15, 10, 6, -6, -10, -15, -17};
        int fileChanges[8] = {1, -1, 2, -2, -2, 2, -1, 1};
        int rankChanges[8] = {2, 2, 1, 1, -1, -1, -2, -2};

        for (int i = 0; i < 8; i++)
        {
            int newSquare = square + offsets[i];
            int newFile = file + fileChanges[i];
            int newRank = rank + rankChanges[i];

            if (newFile >= 0 && newFile <= 7 && newRank >= 0 && newRank <= 7)
            {
                if (isValidMove(newSquare, isWhite))
                {
                    moves.push_back(newSquare);
                }
            }
        }

        return moves;
    }

    std::vector<int> generateBishopMoves(int square, bool isWhite)
    {
        std::vector<int> moves;
        addSlidingMoves(moves, square, isWhite, 9);
        addSlidingMoves(moves, square, isWhite, 7);
        addSlidingMoves(moves, square, isWhite, -7);
        addSlidingMoves(moves, square, isWhite, -9);
        return moves;
    }

    std::vector<int> generateRookMoves(int square, bool isWhite)
    {
        std::vector<int> moves;
        addSlidingMoves(moves, square, isWhite, 8);
        addSlidingMoves(moves, square, isWhite, -8);
        addSlidingMoves(moves, square, isWhite, 1);
        addSlidingMoves(moves, square, isWhite, -1);
        return moves;
    }

    std::vector<int> generateQueenMoves(int square, bool isWhite)
    {
        std::vector<int> moves = generateRookMoves(square, isWhite);
        std::vector<int> bishopMoves = generateBishopMoves(square, isWhite);
        moves.insert(moves.end(), bishopMoves.begin(), bishopMoves.end());
        return moves;
    }

    std::vector<int> generateKingMoves(int square, bool isWhite)
    {
        std::vector<int> moves;
        int file = getFile(square);
        int rank = getRank(square);

        int offsets[8] = {8, -8, 1, -1, 9, 7, -7, -9};
        int fileChanges[8] = {0, 0, 1, -1, 1, -1, 1, -1};
        int rankChanges[8] = {1, -1, 0, 0, 1, 1, -1, -1};

        for (int i = 0; i < 8; i++)
        {
            int newSquare = square + offsets[i];
            int newFile = file + fileChanges[i];
            int newRank = rank + rankChanges[i];

            if (newFile >= 0 && newFile <= 7 && newRank >= 0 && newRank <= 7)
            {
                if (isValidMove(newSquare, isWhite))
                {
                    moves.push_back(newSquare);
                }
            }
        }

        return moves;
    }
};