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
                    legalMoves = MoveGen::generateLegalMoves(this);

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
                    for(auto& move : legalMoves)
                    {
                        if (move.from == selectedSquare && move.to == targetSquare)
                        {
                            Move m = move;
                            makeMove(m);
                        }
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

        // Save data before modifying
        move.capturedPiece = pieces[move.to];
        move.prevEnPassant = enPassantSquare;

        // Save piece color before modifying
        bool movingPieceIsWhite = movingPiece.isWhite;

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
            int capturedPawnSquare = movingPieceIsWhite ? (move.to - 8) : (move.to + 8);
            pieces[capturedPawnSquare].type = None;
            move.wasEnPassant = true;
            move.epCapturedSquare = capturedPawnSquare;
        }

        // Reset en passant square every move
        enPassantSquare = -1;

        // Set new en passant square if pawn moved two squares
        if (isPawnDoubleMove)
        {
            // En passant square is the square the pawn jumped over
            enPassantSquare = movingPieceIsWhite ? (move.from + 8) : (move.from - 8);
        }

        // Handle Pawn Promotion
        int targetRank = getRank(move.to);
        if (targetPiece.type == Pawn && (targetRank == 7 || targetRank == 0))
        {
            promotePawn(move.to, Queen);
        }

        // Handle Castling - move the rook
        if (move.castling)
        {
            int backRank = movingPieceIsWhite ? 0 : 56;

            // Kingside castling (king moved to g-file)
            if (move.to == move.from + 2)
            {
                // Move rook from h-file to f-file
                Piece &rook = pieces[backRank + 7];
                Piece &rookTarget = pieces[backRank + 5];

                rookTarget.type = rook.type;
                rookTarget.isWhite = rook.isWhite;
                rookTarget.hasMoved = true;
                rook.type = None;
            }
            // Queenside castling (king moved to c-file)
            else if (move.to == move.from - 2)
            {
                // Move rook from a-file to d-file
                Piece &rook = pieces[backRank];
                Piece &rookTarget = pieces[backRank + 3];

                rookTarget.type = rook.type;
                rookTarget.isWhite = rook.isWhite;
                rookTarget.hasMoved = true;
                rook.type = None;
            }
        }

        // Update king position if king moved
        if (targetPiece.type == King)
        {
            if (movingPieceIsWhite)
                whiteKing = move.to;
            else
                blackKing = move.to;
        }

        isWhiteTurn = !isWhiteTurn;
    }

    void unmakeMove(Move &move)
    {
        Piece &from = pieces[move.from];
        Piece &to = pieces[move.to];

        // restore en passant square
        enPassantSquare = move.prevEnPassant;

        // undo castling
        if (move.castling)
        {
            bool white = to.isWhite;
            int backRank = white ? 0 : 56;

            if (move.to == move.from + 2)
            {
                pieces[backRank + 7] = pieces[backRank + 5];
                pieces[backRank + 5].type = None;
            }
            else if (move.to == move.from - 2)
            {
                pieces[backRank] = pieces[backRank + 3];
                pieces[backRank + 3].type = None;
            }
        }

        // move piece back
        from = to;

        // restore captured piece
        if (move.wasEnPassant)
        {
            pieces[move.epCapturedSquare] = move.capturedPiece;
            to.type = None;
        }
        else
        {
            to = move.capturedPiece;
        }

        // restore king square
        if (from.type == King)
        {
            if (from.isWhite)
                whiteKing = move.from;
            else
                blackKing = move.from;
        }

        isWhiteTurn = !isWhiteTurn;
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