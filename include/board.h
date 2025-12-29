#pragma once

#include "window.h"
#include "piece.h"
#include "move_generator.h"
#include <algorithm>

#include <string>
#include <vector>
#include <cstdlib>

struct PieceList
{
    std::vector<int> whitePawns;
    std::vector<int> whiteKnights;
    std::vector<int> whiteBishops;
    std::vector<int> whiteRooks;
    std::vector<int> whiteQueens;
    int whiteKing = -1;

    std::vector<int> blackPawns;
    std::vector<int> blackKnights;
    std::vector<int> blackBishops;
    std::vector<int> blackRooks;
    std::vector<int> blackQueens;
    int blackKing = -1;

    void clear()
    {
        whitePawns.clear();
        whiteKnights.clear();
        whiteBishops.clear();
        whiteRooks.clear();
        whiteQueens.clear();
        blackPawns.clear();
        blackKnights.clear();
        blackBishops.clear();
        blackRooks.clear();
        blackQueens.clear();
        whiteKing = -1;
        blackKing = -1;
    }

    void addPiece(PieceType type, bool isWhite, int square)
    {
        if (isWhite)
        {
            switch (type)
            {
            case Pawn:
                whitePawns.push_back(square);
                break;
            case Knight:
                whiteKnights.push_back(square);
                break;
            case Bishop:
                whiteBishops.push_back(square);
                break;
            case Rook:
                whiteRooks.push_back(square);
                break;
            case Queen:
                whiteQueens.push_back(square);
                break;
            case King:
                whiteKing = square;
                break;
            default:
                break;
            }
        }
        else
        {
            switch (type)
            {
            case Pawn:
                blackPawns.push_back(square);
                break;
            case Knight:
                blackKnights.push_back(square);
                break;
            case Bishop:
                blackBishops.push_back(square);
                break;
            case Rook:
                blackRooks.push_back(square);
                break;
            case Queen:
                blackQueens.push_back(square);
                break;
            case King:
                blackKing = square;
                break;
            default:
                break;
            }
        }
    }

    void removePiece(PieceType type, bool isWhite, int square)
    {
        std::vector<int> *list = getPieceList(type, isWhite);
        if (list)
        {
            list->erase(std::remove(list->begin(), list->end(), square), list->end());
        }
    }

    void movePiece(PieceType type, bool isWhite, int from, int to)
    {
        if (type == King)
        {
            if (isWhite)
                whiteKing = to;
            else
                blackKing = to;
            return;
        }

        std::vector<int> *list = getPieceList(type, isWhite);
        if (list)
        {
            auto it = std::find(list->begin(), list->end(), from);
            if (it != list->end())
            {
                *it = to;
            }
        }
    }

private:
    std::vector<int> *getPieceList(PieceType type, bool isWhite)
    {
        if (isWhite)
        {
            switch (type)
            {
            case Pawn:
                return &whitePawns;
            case Knight:
                return &whiteKnights;
            case Bishop:
                return &whiteBishops;
            case Rook:
                return &whiteRooks;
            case Queen:
                return &whiteQueens;
            default:
                return nullptr;
            }
        }
        else
        {
            switch (type)
            {
            case Pawn:
                return &blackPawns;
            case Knight:
                return &blackKnights;
            case Bishop:
                return &blackBishops;
            case Rook:
                return &blackRooks;
            case Queen:
                return &blackQueens;
            default:
                return nullptr;
            }
        }
    }
};

class Board
{
public:
    std::vector<Piece> pieces = loadFenString("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    int selectedSquare = -1;
    bool isWhiteTurn = true;
    std::vector<Move> legalMoves;
    bool isDragging = false;

    int checkmate = -1;

    int enPassantSquare = -1;
    int lastPawnOrCapture = 0;

    PieceList pieceList;

    Board()
    {
        findPieces();
    }

    void handleInput(Window &window, float boardSize)
    {
        if (window.wasMouseJustPressed())
        {
            int square = window.screenToSquare(boardSize);

            if (square >= 0 && square < 64)
            {
                const Piece &piece = pieces[square];
                if (piece.type != None && piece.isWhite == isWhiteTurn)
                {
                    selectedSquare = square;

                    // Generate moves and extract destinations for this piece
                    legalMoves.clear();
                    auto all = MoveGen::generateLegalMoves(this);
                    for (auto &m : all)
                    {
                        if (m.from == selectedSquare)
                            legalMoves.push_back(m);
                    }

                    isDragging = !legalMoves.empty();
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
                    for (auto &move : legalMoves)
                    {
                        if (move.from == selectedSquare && move.to == targetSquare)
                        {
                            Move m = move;
                            makeMove(m);
                            break;
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

        // Save state for unmake
        move.capturedPiece = pieces[move.to];
        move.prevEnPassant = enPassantSquare;
        move.movedPieceHadMoved = movingPiece.hasMoved;

        PieceType movingType = movingPiece.type;
        bool movingIsWhite = movingPiece.isWhite;

        int movedSquares = abs(move.to - move.from);
        bool isPawnDoubleMove = (movingPiece.type == Pawn && movedSquares == 16);

        bool isEnPassant = (movingPiece.type == Pawn && move.to == enPassantSquare && enPassantSquare != -1);

        // If capturing, remove captured piece from list
        if (targetPiece.type != None)
        {
            pieceList.removePiece(targetPiece.type, targetPiece.isWhite, move.to);
        }

        // Handle en passant capture
        if (isEnPassant)
        {
            int capturedPawnSquare = movingIsWhite ? (move.to - 8) : (move.to + 8);
            move.epCapturedPiece = pieces[capturedPawnSquare];
            move.epCapturedSquare = capturedPawnSquare;
            move.wasEnPassant = true;

            pieces[capturedPawnSquare].type = None;
            pieceList.removePiece(Pawn, !movingIsWhite, capturedPawnSquare);
        }

        // Make the move on the board
        targetPiece.type = movingPiece.type;
        targetPiece.isWhite = movingPiece.isWhite;
        targetPiece.hasMoved = true;
        movingPiece.type = None;

        enPassantSquare = -1;

        if (isPawnDoubleMove)
        {
            enPassantSquare = movingIsWhite ? (move.from + 8) : (move.from - 8);
        }

        // Handle Pawn Promotion
        int targetRank = getRank(move.to);
        if (targetPiece.type == Pawn && (targetRank == 7 || targetRank == 0))
        {
            promotePawn(move.to, Queen);
            move.wasPromotion = true;

            // Update piece list: remove pawn, add queen
            pieceList.removePiece(Pawn, movingIsWhite, move.from);
            pieceList.addPiece(Queen, movingIsWhite, move.to);
        }
        else
        {
            // Normal move - update piece position in list
            pieceList.movePiece(movingType, movingIsWhite, move.from, move.to);
        }

        // Handle Castling
        if (move.castling)
        {
            int backRank = movingIsWhite ? 0 : 56;

            if (move.to == move.from + 2) // Kingside
            {
                Piece &rook = pieces[backRank + 7];
                Piece &rookTarget = pieces[backRank + 5];

                move.rookHadMoved = rook.hasMoved;

                rookTarget.type = rook.type;
                rookTarget.isWhite = rook.isWhite;
                rookTarget.hasMoved = true;
                rook.type = None;

                pieceList.movePiece(Rook, movingIsWhite, backRank + 7, backRank + 5);
            }
            else if (move.to == move.from - 2) // Queenside
            {
                Piece &rook = pieces[backRank];
                Piece &rookTarget = pieces[backRank + 3];

                move.rookHadMoved = rook.hasMoved;

                rookTarget.type = rook.type;
                rookTarget.isWhite = rook.isWhite;
                rookTarget.hasMoved = true;
                rook.type = None;

                pieceList.movePiece(Rook, movingIsWhite, backRank, backRank + 3);
            }
        }

        isWhiteTurn = !isWhiteTurn;
    }

    void unmakeMove(Move &move)
    {
        Piece &from = pieces[move.from];
        Piece &to = pieces[move.to];

        PieceType movedType = to.type;
        bool movedIsWhite = to.isWhite;

        // Restore en passant square
        enPassantSquare = move.prevEnPassant;

        // Undo castling
        if (move.castling)
        {
            int backRank = movedIsWhite ? 0 : 56;

            if (move.to == move.from + 2) // Kingside
            {
                pieces[backRank + 7] = pieces[backRank + 5];
                pieces[backRank + 7].hasMoved = move.rookHadMoved;
                pieces[backRank + 5].type = None;

                pieceList.movePiece(Rook, movedIsWhite, backRank + 5, backRank + 7);
            }
            else if (move.to == move.from - 2) // Queenside
            {
                pieces[backRank] = pieces[backRank + 3];
                pieces[backRank].hasMoved = move.rookHadMoved;
                pieces[backRank + 3].type = None;

                pieceList.movePiece(Rook, movedIsWhite, backRank + 3, backRank);
            }
        }

        // Undo promotion
        if (move.wasPromotion)
        {
            pieceList.removePiece(Queen, movedIsWhite, move.to);
            pieceList.addPiece(Pawn, movedIsWhite, move.from);
            movedType = Pawn;
        }
        else
        {
            // Normal unmove - update piece position in list
            pieceList.movePiece(movedType, movedIsWhite, move.to, move.from);
        }

        // Move piece back
        from = to;
        from.hasMoved = move.movedPieceHadMoved;

        if (move.wasPromotion)
        {
            from.type = Pawn;
        }

        // Restore captured piece
        if (move.wasEnPassant)
        {
            pieces[move.epCapturedSquare] = move.epCapturedPiece;
            to.type = None;

            pieceList.addPiece(Pawn, !movedIsWhite, move.epCapturedSquare);
        }
        else
        {
            to = move.capturedPiece;

            // If we captured a piece, restore it to piece list
            if (move.capturedPiece.type != None)
            {
                pieceList.addPiece(move.capturedPiece.type, move.capturedPiece.isWhite, move.to);
            }
        }

        isWhiteTurn = !isWhiteTurn;
    }

    void findPieces()
    {
        pieceList.clear();
        for (int i = 0; i < 64; i++)
        {
            Piece &piece = pieces[i];
            if (piece.type != None)
            {
                pieceList.addPiece(piece.type, piece.isWhite, i);
            }
        }
    }

    void moveComputer(bool isWhite)
    {
        if (checkmate >= 0)
            return;

        if (isWhiteTurn == isWhite)
        {
            Move move = chooseComputerMove(isWhite);

            if (move.from < 0 || move.to < 0)
                return; // game over

            makeMove(move);
        }
    }

    Move chooseComputerMove(bool isWhite)
    {
        std::vector<Move> moves = MoveGen::generateLegalMoves(this);

        if (moves.size() == 0)
        {
            checkmate = isWhite ? 1 : 0;
            return Move(-1, -1);
        }

        int i = rand() % moves.size();

        return moves[i];
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
        Piece &piece = pieces[square];

        if (square < 56 && piece.isWhite == true)
            return;

        if (square > 7 && piece.isWhite == false)
            return;

        if (piece.type != Pawn)
            return;

        if (type != Pawn && type != King && type != None)
        {
            piece.type = type;
        }
    }
};