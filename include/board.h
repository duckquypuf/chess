#pragma once

#include "piece.h"
#include <algorithm>

class Board
{
public:
    std::vector<Piece> pieces = loadFenString("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    int selectedSquare = -1;
    int enPassantSquare = -1;
    bool isWhiteTurn = true;
    std::vector<int> legalMoves;
    bool isDragging = false;

    int whiteKing = -1;
    int blackKing = -1;

    int checkmate = -1;

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
                    legalMoves = generateLegalMoves(square);
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
                        makeMove(selectedSquare, targetSquare);
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

    Move makeMove(int from, int to, bool simulate = false)
    {
        Move m;
        m.from = from;
        m.to = to;

        m.prevWhiteKing = whiteKing;
        m.prevBlackKing = blackKing;

        m.prevLastPawnOrCapture = lastPawnOrCapture;

        Piece &moving = pieces[from];
        Piece &target = pieces[to];

        m.captured = target;
        m.prevEnPassant = enPassantSquare;
        m.pieceHadMoved = moving.hasMoved;

        m.wasCastling = false;
        m.wasEnPassant = false;

        // en passant
        if (moving.type == Pawn && to == enPassantSquare)
        {
            m.wasEnPassant = true;
            m.enPassantPawnSquare = moving.isWhite ? to - 8 : to + 8;
            m.captured = pieces[m.enPassantPawnSquare];
            pieces[m.enPassantPawnSquare].type = None;
        }

        // castling
        if (moving.type == King && abs(to - from) == 2)
        {
            m.wasCastling = true;

            if (to > from)
            {
                m.rookFrom = from + 3;
                m.rookTo = from + 1;
            }
            else
            {
                m.rookFrom = from - 4;
                m.rookTo = from - 1;
            }

            m.rookHadMoved = pieces[m.rookFrom].hasMoved;

            pieces[m.rookTo] = pieces[m.rookFrom];
            pieces[m.rookTo].pos = m.rookTo;
            pieces[m.rookTo].hasMoved = true;
            pieces[m.rookFrom].type = None;
        }

        // king tracking
        if (moving.type == King)
        {
            if (moving.isWhite)
                whiteKing = to;
            else
                blackKing = to;
        }

        // en passant reset
        enPassantSquare = -1;
        if (moving.type == Pawn && abs(to - from) == 16)
            enPassantSquare = moving.isWhite ? from + 8 : from - 8;

        // move piece
        pieces[to] = moving;
        pieces[to].pos = to;
        pieces[to].hasMoved = true;

        pieces[from].type = None;

        m.wasPromotion = false;

        if (!simulate && pieces[to].type == Pawn)
        {
            if ((pieces[to].isWhite && to >= 56) ||
                (!pieces[to].isWhite && to <= 7))
            {
                m.wasPromotion = true;
                pieces[to].type = Queen;
            }
        }

        if (!simulate)
        {
            if (moving.type == Pawn || target.type != None || m.wasEnPassant)
                lastPawnOrCapture = 0;
            else
                lastPawnOrCapture++;
        }

        return m;
    }

    void unmakeMove(const Move &m)
    {
        Piece &moving = pieces[m.to];

        // move back
        pieces[m.from] = moving;
        pieces[m.from].pos = m.from;
        pieces[m.from].hasMoved = m.pieceHadMoved;

        // restore promotion
        if (m.wasPromotion)
        {
            pieces[m.to].type = Pawn;
        }

        // restore capture
        pieces[m.to] = m.captured;

        // restore en passant pawn
        if (m.wasEnPassant)
        {
            pieces[m.enPassantPawnSquare] = m.captured;
            pieces[m.to].type = None;
        }

        // undo castling
        if (m.wasCastling)
        {
            pieces[m.rookFrom] = pieces[m.rookTo];
            pieces[m.rookFrom].pos = m.rookFrom;
            pieces[m.rookFrom].hasMoved = m.rookHadMoved;
            pieces[m.rookTo].type = None;
        }

        // restore king pos
        if (pieces[m.from].type == King)
        {
            if (pieces[m.from].isWhite)
                whiteKing = m.from;
            else
                blackKing = m.from;
        }

        enPassantSquare = m.prevEnPassant;

        whiteKing = m.prevWhiteKing;
        blackKing = m.prevBlackKing;

        lastPawnOrCapture = m.prevLastPawnOrCapture;
    }

    Move chooseComputerMove(bool isWhite = false)
    {
        std::vector<Move> allMoves;

        for (int i = 0; i < 64; i++)
        {
            Piece &piece = pieces[i];
            if (piece.type == None || piece.isWhite != isWhite)
                continue;

            std::vector<int> moves = generateLegalMoves(i);
            for (int to : moves)
            {
                Move m;
                m.from = i;
                m.to = to;
                allMoves.push_back(m);
            }
        }

        if (allMoves.empty() && !isKingInCheck(isWhite))
        {
            checkmate = 2; // stalemate
        }

        if (allMoves.empty())
        {
            checkmate = isWhite ? 0 : 1; // computer lost
            return Move{-1, -1};
        }

        int idx = rand() % allMoves.size();
        Move chosen = allMoves[idx];

        return chosen;
    }

    std::vector<int> generateLegalMoves(int square)
    {
        std::vector<int> pseudoLegalMoves = generatePseudoLegalMoves(square);
        std::vector<int> legalMoves;

        Piece &piece = pieces[square];

        for (int targetSquare : pseudoLegalMoves)
        {
            Piece capturedPiece = pieces[targetSquare];
            bool hadMoved = piece.hasMoved;
            int prevEnPassant = enPassantSquare;

            Move m = makeMove(square, targetSquare, true);
            bool inCheck = isKingInCheck(piece.isWhite);
            unmakeMove(m);

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

            // Add castling moves here (safe from recursion)
            if (!piece.hasMoved && !isKingInCheck(piece.isWhite))
            {
                // Kingside
                int kingsideRookSquare = square + 3;
                if (kingsideRookSquare < 64 && pieces[kingsideRookSquare].type == Rook &&
                    !pieces[kingsideRookSquare].hasMoved &&
                    pieces[kingsideRookSquare].isWhite == piece.isWhite)
                {
                    if (pieces[square + 1].type == None && pieces[square + 2].type == None)
                    {
                        if (!isSquareAttacked(square + 1, !piece.isWhite) &&
                            !isSquareAttacked(square + 2, !piece.isWhite))
                        {
                            moves.push_back(square + 2);
                        }
                    }
                }

                // Queenside
                int queensideRookSquare = square - 4;
                if (queensideRookSquare >= 0 && pieces[queensideRookSquare].type == Rook &&
                    !pieces[queensideRookSquare].hasMoved &&
                    pieces[queensideRookSquare].isWhite == piece.isWhite)
                {
                    if (pieces[square - 1].type == None && pieces[square - 2].type == None &&
                        pieces[square - 3].type == None)
                    {
                        if (!isSquareAttacked(square - 1, !piece.isWhite) &&
                            !isSquareAttacked(square - 2, !piece.isWhite))
                        {
                            moves.push_back(square - 2);
                        }
                    }
                }
            }
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

            std::vector<int> moves;
            if (piece.type == King)
            {
                moves = generateKingMoves(i, piece.isWhite);
            }
            else
            {
                switch (piece.type)
                {
                case Pawn:
                    moves = generatePawnAttacks(i, piece.isWhite);
                    break;
                case Knight:
                    moves = generateKnightMoves(i, piece.isWhite);
                    break;
                case Bishop:
                    moves = generateBishopMoves(i, piece.isWhite);
                    break;
                case Rook:
                    moves = generateRookMoves(i, piece.isWhite);
                    break;
                case Queen:
                    moves = generateQueenMoves(i, piece.isWhite);
                    break;
                default:
                    break;
                }
            }

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

    void checkForMate()
    {
        if(lastPawnOrCapture >= 100) checkmate = 2;

        bool side = isWhiteTurn;

        if (!isKingInCheck(side))
            return;

        for (int i = 0; i < 64; i++)
        {
            Piece &piece = pieces[i];
            if (piece.type == None || piece.isWhite != side)
                continue;

            if (!generateLegalMoves(i).empty())
                return;
        }

        checkmate = side ? 1 : 0;
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