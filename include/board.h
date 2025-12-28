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
    std::vector<Piece> pieces = loadFenString("rnbqkbnr/8/8/8/8/8/8/RNBQKBNR");
    int selectedSquare = -1;
    bool isWhiteTurn = true;
    std::vector<int> legalMoves;
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
                        //checkForMate();
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
    
    /*void moveComputer(bool isWhite)
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
    }*/

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
};