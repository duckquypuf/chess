#pragma once

#include "window.h"
#include "piece.h"
#include "move_generator.h"
#include <algorithm>

#include <map>
#include <string>
#include <vector>

class Board
{
public:
    std::vector<Piece> pieces = loadFenString("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    int selectedSquare = -1;
    bool isWhiteTurn = true;
    std::vector<Move> legalMoves;
    bool isDragging = false;

    int whiteKing = -1;
    int blackKing = -1;

    int checkmate = -1;

    int enPassantSquare = -1;
    int lastPawnOrCapture = 0;

    Board()
    {
        findKings();
    }

    void handleInput(Window &window, float boardSize)
    {
        if (window.wasMouseJustPressed() && isWhiteTurn)
        {
            int square = window.screenToSquare(boardSize);

            if (square >= 0 && square < 64)
            {
                Piece &piece = pieces[square];
                if (piece.type != None && piece.isWhite == isWhiteTurn)
                {
                    selectedSquare = square;
                    MoveGen::generateMoves(this);
                    isDragging = true;
                }
            }
        }
        else if (window.wasMouseJustReleased() && isWhiteTurn)
        {
            if (isDragging && selectedSquare != -1)
            {
                int targetSquare = window.screenToSquare(boardSize);

                if (targetSquare >= 0 && targetSquare < 64)
                {
                    if (std::find(legalMoves.begin(), legalMoves.end(), targetSquare) != legalMoves.end())
                    {
                        Move m = Move(selectedSquare, targetSquare);
                        makeMove(m);
                        isWhiteTurn = false;
                        checkForMate();
                    }
                }
            }

            selectedSquare = -1;
            legalMoves.clear();
            isDragging = false;
        }
    }

    void makeMove(Move &move)
    {
        Piece &movingPiece = pieces[move.from];
        Piece &targetPiece = pieces[move.to];

        targetPiece.type = movingPiece.type;
        targetPiece.isWhite = movingPiece.isWhite;

        movingPiece.type = None;
    }

    std::vector<Move> generatePseudoLegalMoves(int square)
    {
        Piece &piece = pieces[square];

        std::vector<int> moves;

        switch(piece.type)
        {
        case Pawn:
            moves = generatePawnMoves(square, piece.isWhite);
        }
    }
    
    void moveComputer(bool isWhite)
    {
        if(checkmate >= 0) return;

        if (isWhiteTurn == isWhite)
        {
            Move move = chooseComputerMove(isWhite);

            if (move.from < 0 || move.to < 0)
                return; // game over

            makeMove(move.from, move.to, false);
            isWhiteTurn = !isWhite;
            checkForMate();
        }
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

    void checkForMate()
    {
        if (lastPawnOrCapture >= 100)
        {
            checkmate = 2; // Draw by 50-move rule
            return;
        }

        if (isThreefoldRepetition())
        {
            checkmate = 2; // Draw by repetition
            return;
        }

        bool side = isWhiteTurn;

        bool hasMove = false;

        for (int i = 0; i < 64; i++)
        {
            Piece &piece = pieces[i];
            if (piece.type == None || piece.isWhite != side)
                continue;

            if (!generateLegalMoves(i).empty())
            {
                hasMove = true;
                break;
            }
        }

        if (hasMove) return;

        if (isKingInCheck(side))
            checkmate = side ? 1 : 0; // checkmate
        else
            checkmate = 2; // stalemate
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

    void promotePawn(int square, PieceType type)
    {
        Piece& piece = pieces[square];

        if(square < 56 && piece.isWhite == true) 
            return;
        
        if(square > 7 && piece.isWhite == false) 
            return;

        if(piece.type != Pawn)
            return;
        
        if(type != Pawn && type != King && type != None)
        {
            piece.type = type;
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
        int enPassantRank = isWhite ? 4 : 3; // Rank where en passant can happen

        // Forward move
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

        // En passant
        if (rank == enPassantRank && enPassantSquare != -1)
        {
            if (file > 0 && captureLeft == enPassantSquare)
            {
                moves.push_back(enPassantSquare);
            }
            if (file < 7 && captureRight == enPassantSquare)
            {
                moves.push_back(enPassantSquare);
            }
        }

        return moves;
    }

    std::vector<int> generatePawnAttacks(int square, bool isWhite)
    {
        std::vector<int> moves;
        int file = getFile(square);
        int dir = isWhite ? 1 : -1;

        int left = square + dir * 8 - 1;
        int right = square + dir * 8 + 1;

        if (file > 0 && left >= 0 && left < 64)
            moves.push_back(left);
        if (file < 7 && right >= 0 && right < 64)
            moves.push_back(right);

        return moves;
    }

    std::vector<int> generateKnightMoves(int square, bool isWhite)
    {
        std::vector<int> moves;
        int file = getFile(square);
        int rank = getRank(square);

        int fileDiff[8] = {1, -1, 2, -2, 2, -2, 1, -1};
        int rankRiff[8] = {2, 2, 1, 1, -1, -1, -2, -2};

        for (int i = 0; i < 8; i++)
        {
            int newFile = file + fileDiff[i];
            int newRank = rank + rankRiff[i];

            if (newFile < 0 || newFile > 7 || newRank < 0 || newRank > 7)
                continue;

            int newSquare = newRank * 8 + newFile;

            if (isValidMove(newSquare, isWhite))
                moves.push_back(newSquare);
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