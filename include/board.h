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
    std::vector<Piece> pieces = loadFenString("RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr");
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
        if (window.wasMouseJustPressed())// && isWhiteTurn)
        {
            int square = window.screenToSquare(boardSize);

            if (square >= 0 && square < 64)
            {
                const Piece &piece = pieces[square];
                if (piece.type != None && piece.isWhite == isWhiteTurn)
                {
                    selectedSquare = square;

                    // Generate moves and extract destinations for this piece
                    std::vector<Move> generatedMoves = MoveGen::generateMoves(this);
                    legalMoves.clear();
                    for (const Move &m : generatedMoves)
                    {
                        if (m.from == selectedSquare)
                        {
                            legalMoves.push_back(m.to);
                        }
                    }

                    isDragging = true;
                }
            }
        }
        else if (window.wasMouseJustReleased())// && isWhiteTurn)
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
                        isWhiteTurn = !isWhiteTurn;
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

        // Check if this is a pawn moving two squares (for en passant next turn)
        int movedSquares = abs(move.to - move.from);
        bool isPawnDoubleMove = (movingPiece.type == Pawn && movedSquares == 16);

        // Check if this is an en passant capture
        bool isEnPassant = (movingPiece.type == Pawn &&
                            move.to == enPassantSquare &&
                            enPassantSquare != -1);

        // Make the move
        targetPiece.type = movingPiece.type;
        targetPiece.isWhite = movingPiece.isWhite;
        targetPiece.hasMoved = true;
        movingPiece.type = None;

        // Handle en passant capture - remove the captured pawn
        if (isEnPassant)
        {
            int capturedPawnSquare = movingPiece.isWhite ? (move.to - 8) : (move.to + 8);
            pieces[capturedPawnSquare].type = None;
        }

        // Reset en passant square every move
        enPassantSquare = -1;

        // Set new en passant square if pawn moved two squares
        if (isPawnDoubleMove)
        {
            // En passant square is the square the pawn jumped over
            enPassantSquare = movingPiece.isWhite ? (move.from + 8) : (move.from - 8);
        }

        // Handle Pawn Promotion
        if (targetPiece.type == Pawn && getRank(targetPiece.pos) > 55 || getRank(targetPiece.pos) < 8)
        {
            promotePawn(targetPiece.pos, Queen);
        }
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