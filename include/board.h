#pragma once

#include "window.h"
#include "piece.h"
#include <algorithm>

#include <map>
#include <string>
#include <vector>

class Board
{
public:
    std::vector<Piece> pieces = loadFenString("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    std::map<std::string, int> positionHistory;
    int selectedSquare = -1;
    int enPassantSquare = -1;
    bool isWhiteTurn = true;
    std::vector<int> legalMoves;
    bool isDragging = false;

    int whiteKing = -1;
    int blackKing = -1;

    int checkmate = -1;

    int lastPawnOrCapture = 0;

    std::vector<Piece *> pawns[2];
    std::vector<Piece *> rooks[2];
    std::vector<Piece *> knights[2];
    std::vector<Piece *> bishops[2];
    std::vector<Piece *> queens[2];
    std::vector<Piece *> kings[2];

    std::vector<int> attackedSquares[2]; // [0] = white attacks, [1] = black attacks
    std::vector<int> pinnedPieces;       // Squares of pinned pieces
    std::vector<int> pinRays[64];        // For each pinned piece, which squares it can move to

    std::vector<int> checkingPieces; // Pieces giving check
    std::vector<int> blockSquares;   // Squares that block check (empty if double check)

    Board()
    {
        findKings();
        recordPosition();
    }

    void recordPosition()
    {
        std::string hash = getPositionHash();
        positionHistory[hash]++;
    }

    bool isThreefoldRepetition()
    {
        std::string hash = getPositionHash();
        return positionHistory[hash] >= 3;
    }

    void clearPositionHistory()
    {
        positionHistory.clear();
    }

    void computeAttackData(bool forWhite)
    {
        int colorIdx = forWhite ? 0 : 1;
        attackedSquares[colorIdx].clear();

        // Generate attacked squares for all pieces of this color
        for (int i = 0; i < 64; i++)
        {
            Piece &piece = pieces[i];
            if (piece.type == None || piece.isWhite != forWhite)
                continue;

            std::vector<int> attacks = generateAttacks(i, piece.type, forWhite);

            // Add to attacked squares (use set or check for duplicates if needed)
            for (int sq : attacks)
            {
                attackedSquares[colorIdx].push_back(sq);
            }
        }
    }

    bool isSquareAttackedFast(int square, bool byWhite)
    {
        int colorIdx = byWhite ? 0 : 1;
        return std::find(attackedSquares[colorIdx].begin(),
                         attackedSquares[colorIdx].end(),
                         square) != attackedSquares[colorIdx].end();
    }

    std::vector<int> generateAttacks(int square, PieceType type, bool isWhite)
    {
        switch (type)
        {
        case Pawn:
            return generatePawnAttacks(square, isWhite);
        case Knight:
            return generateKnightMoves(square, isWhite);
        case Bishop:
            return generateBishopMoves(square, isWhite);
        case Rook:
            return generateRookMoves(square, isWhite);
        case Queen:
            return generateQueenMoves(square, isWhite);
        case King:
            return generateKingMoves(square, isWhite);
        default:
            return {};
        }
    }

    // Detect pins and checks
    void computePinsAndChecks(bool forWhite)
    {
        pinnedPieces.clear();
        checkingPieces.clear();
        blockSquares.clear();

        for (int i = 0; i < 64; i++)
            pinRays[i].clear();

        int kingSquare = forWhite ? whiteKing : blackKing;
        bool enemyColor = !forWhite;

        // Check all sliding pieces (bishops, rooks, queens)
        for (int i = 0; i < 64; i++)
        {
            Piece &piece = pieces[i];
            if (piece.type == None || piece.isWhite != enemyColor)
                continue;

            // Only check sliding pieces
            if (piece.type != Bishop && piece.type != Rook && piece.type != Queen)
                continue;

            // Check if this piece can see the king
            std::vector<int> ray = getRayToKing(i, kingSquare, piece.type);

            if (!ray.empty())
            {
                // Count pieces between attacker and king
                int piecesInWay = 0;
                int pinnedSquare = -1;

                for (int sq : ray)
                {
                    if (pieces[sq].type != None)
                    {
                        piecesInWay++;
                        pinnedSquare = sq;
                    }
                }

                if (piecesInWay == 0)
                {
                    // Direct check
                    checkingPieces.push_back(i);
                    blockSquares = ray; // Can block along this ray
                }
                else if (piecesInWay == 1 && pieces[pinnedSquare].isWhite == forWhite)
                {
                    // Pinned piece
                    pinnedPieces.push_back(pinnedSquare);
                    pinRays[pinnedSquare] = ray;
                    pinRays[pinnedSquare].push_back(i); // Can capture attacker
                }
            }
        }

        // Check knights for direct checks (can't pin)
        for (int i = 0; i < 64; i++)
        {
            Piece &piece = pieces[i];
            if (piece.type == Knight && piece.isWhite == enemyColor)
            {
                std::vector<int> moves = generateKnightMoves(i, enemyColor);
                if (std::find(moves.begin(), moves.end(), kingSquare) != moves.end())
                {
                    checkingPieces.push_back(i);
                    // Knights can't be blocked, only captured
                    blockSquares.push_back(i);
                }
            }
        }

        // Check pawns for direct checks
        for (int i = 0; i < 64; i++)
        {
            Piece &piece = pieces[i];
            if (piece.type == Pawn && piece.isWhite == enemyColor)
            {
                std::vector<int> attacks = generatePawnAttacks(i, enemyColor);
                if (std::find(attacks.begin(), attacks.end(), kingSquare) != attacks.end())
                {
                    checkingPieces.push_back(i);
                    blockSquares.push_back(i); // Can only capture
                }
            }
        }
    }

    // Get ray from attacker to king (returns empty if no line of sight)
    std::vector<int> getRayToKing(int from, int kingPos, PieceType attackerType)
    {
        std::vector<int> ray;

        int fileFrom = from % 8;
        int rankFrom = from / 8;
        int fileKing = kingPos % 8;
        int rankKing = kingPos / 8;

        int fileDiff = fileKing - fileFrom;
        int rankDiff = rankKing - rankFrom;

        // Determine direction
        int direction = 0;

        // Rook/Queen horizontal/vertical
        if (fileDiff == 0 && rankDiff != 0)
            direction = (rankDiff > 0) ? 8 : -8;
        else if (rankDiff == 0 && fileDiff != 0)
            direction = (fileDiff > 0) ? 1 : -1;
        // Bishop/Queen diagonal
        else if (abs(fileDiff) == abs(rankDiff))
        {
            if (fileDiff > 0 && rankDiff > 0)
                direction = 9;
            else if (fileDiff > 0 && rankDiff < 0)
                direction = -7;
            else if (fileDiff < 0 && rankDiff > 0)
                direction = 7;
            else if (fileDiff < 0 && rankDiff < 0)
                direction = -9;
        }

        if (direction == 0)
            return {}; // No line of sight

        // Check if piece type can move in this direction
        if (attackerType == Rook && (direction == 9 || direction == 7 || direction == -7 || direction == -9))
            return {};
        if (attackerType == Bishop && (direction == 8 || direction == -8 || direction == 1 || direction == -1))
            return {};

        // Build ray (excluding attacker and king)
        int current = from + direction;
        while (current != kingPos && current >= 0 && current < 64)
        {
            ray.push_back(current);
            current += direction;

            // Check for board wrap
            int currentFile = current % 8;
            int prevFile = (current - direction) % 8;
            if (abs(currentFile - prevFile) > 1)
                break;
        }

        if (current == kingPos)
            return ray;

        return {}; // Didn't reach king
    }

    // FAST legal move generation using pre-computed data
    std::vector<int> generateLegalMovesFast(int square)
    {
        Piece &piece = pieces[square];
        if (piece.type == None)
            return {};

        // Pre-compute attack data if not done
        computeAttackData(!piece.isWhite);
        computePinsAndChecks(piece.isWhite);

        std::vector<int> pseudoLegal = generatePseudoLegalMoves(square);
        std::vector<int> legal;

        int kingSquare = piece.isWhite ? whiteKing : blackKing;
        bool inCheck = !checkingPieces.empty();
        bool isDoubleCheck = checkingPieces.size() > 1;

        // King moves - must not move into attack
        if (piece.type == King)
        {
            for (int move : pseudoLegal)
            {
                // Simulate king move temporarily
                Piece temp = pieces[move];
                pieces[move] = piece;
                pieces[square].type = None;

                // Recompute attacks without this king position
                computeAttackData(!piece.isWhite);
                bool safe = !isSquareAttackedFast(move, !piece.isWhite);

                // Restore
                pieces[square] = piece;
                pieces[move] = temp;

                if (safe)
                    legal.push_back(move);
            }
            return legal;
        }

        // Double check - only king moves allowed
        if (isDoubleCheck)
            return {};

        // Check if this piece is pinned
        bool isPinned = std::find(pinnedPieces.begin(), pinnedPieces.end(), square) != pinnedPieces.end();

        if (isPinned)
        {
            // Can only move along pin ray
            for (int move : pseudoLegal)
            {
                if (std::find(pinRays[square].begin(), pinRays[square].end(), move) != pinRays[square].end())
                {
                    legal.push_back(move);
                }
            }
        }
        else if (inCheck)
        {
            // Must block or capture checking piece
            for (int move : pseudoLegal)
            {
                if (std::find(blockSquares.begin(), blockSquares.end(), move) != blockSquares.end())
                {
                    legal.push_back(move);
                }
            }
        }
        else
        {
            // Not in check, not pinned - all pseudo-legal moves are legal
            legal = pseudoLegal;
        }

        return legal;
    }

    int evaluateBoard()
    {
        if (checkmate == 0)
            return 100000; // White wins
        if (checkmate == 1)
            return -100000; // Black wins
        if (checkmate == 2)
            return 0; // Draw

        int score = 0;

        for (int i = 0; i < 64; i++)
        {
            Piece &p = pieces[i];
            if (p.type == None)
                continue;

            int pieceValue = 0;
            switch (p.type)
            {
            case Pawn:
                pieceValue = PieceData::PawnValue;
                break;
            case Knight:
                pieceValue = PieceData::KnightValue;
                break;
            case Bishop:
                pieceValue = PieceData::BishopValue;
                break;
            case Rook:
                pieceValue = PieceData::RookValue;
                break;
            case Queen:
                pieceValue = PieceData::QueenValue;
                break;
            case King:
                pieceValue = PieceData::KingValue;
                break;
            default:
                break;
            }

            score += p.isWhite ? pieceValue : -pieceValue;
        }

        return score;
    }

    std::string getPositionHash()
    {
        std::string hash = "";

        // Add piece positions
        for (int i = 0; i < 64; i++)
        {
            Piece &p = pieces[i];
            if (p.type == None)
            {
                hash += ".";
            }
            else
            {
                // Format: piece type + color (e.g., "P0" = white pawn, "p1" = black pawn)
                hash += std::to_string(p.type);
                hash += p.isWhite ? "0" : "1";
            }
        }

        // Add turn (critical - same position but different turn is different)
        hash += isWhiteTurn ? "W" : "B";

        // Add castling rights
        hash += pieces[0].hasMoved ? "0" : "1";         // White queenside rook
        hash += pieces[7].hasMoved ? "0" : "1";         // White kingside rook
        hash += pieces[56].hasMoved ? "0" : "1";        // Black queenside rook
        hash += pieces[63].hasMoved ? "0" : "1";        // Black kingside rook
        hash += pieces[whiteKing].hasMoved ? "0" : "1"; // White king
        hash += pieces[blackKing].hasMoved ? "0" : "1"; // Black king

        // Add en passant square
        hash += "_" + std::to_string(enPassantSquare);

        return hash;
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

        if (!simulate)
        {
            if (moving.type == Pawn || target.type != None || m.wasEnPassant)
            {
                lastPawnOrCapture = 0;
                clearPositionHistory(); // Pawn move or capture resets repetition
            }
            else
            {
                lastPawnOrCapture++;
            }

            recordPosition(); // Record the new position
        }

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

        return m;
    }

    void unmakeMove(const Move &m)
    {
        std::string hash = getPositionHash();
        if (positionHistory[hash] > 0)
            positionHistory[hash]--;

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

    int scoreMove(const Move &move)
    {
        int score = 0;

        Piece &moving = pieces[move.from];
        Piece &target = pieces[move.to];

        // 1. Prioritize captures (MVV-LVA: Most Valuable Victim - Least Valuable Attacker)
        if (target.type != None)
        {
            int victimValue = 0;
            int attackerValue = 0;

            switch (target.type)
            {
            case Pawn:
                victimValue = PieceData::PawnValue;
                break;
            case Knight:
                victimValue = PieceData::KnightValue;
                break;
            case Bishop:
                victimValue = PieceData::BishopValue;
                break;
            case Rook:
                victimValue = PieceData::RookValue;
                break;
            case Queen:
                victimValue = PieceData::QueenValue;
                break;
            case King:
                victimValue = PieceData::KingValue;
                break;
            default:
                break;
            }

            switch (moving.type)
            {
            case Pawn:
                attackerValue = PieceData::PawnValue;
                break;
            case Knight:
                attackerValue = PieceData::KnightValue;
                break;
            case Bishop:
                attackerValue = PieceData::BishopValue;
                break;
            case Rook:
                attackerValue = PieceData::RookValue;
                break;
            case Queen:
                attackerValue = PieceData::QueenValue;
                break;
            case King:
                attackerValue = PieceData::KingValue;
                break;
            default:
                break;
            }

            // High value victim captured by low value attacker = good!
            score += 10000 + victimValue - attackerValue;
        }

        // 2. Promotions are great
        if (moving.type == Pawn)
        {
            if ((moving.isWhite && move.to >= 56) || (!moving.isWhite && move.to <= 7))
            {
                score += PieceData::QueenValue; // Promotion value
            }
        }

        // 3. Center control (e4, d4, e5, d5)
        int file = move.to % 8;
        int rank = move.to / 8;
        if ((file == 3 || file == 4) && (rank == 3 || rank == 4))
        {
            score += 50;
        }

        return score;
    }

    int alphaBeta(int depth, int alpha, int beta, bool maximizingPlayer)
    {
        if (depth == 0 || checkmate >= 0)
            return evaluateBoard();

        // Generate all moves
        std::vector<Move> allMoves;
        for (int i = 0; i < 64; i++)
        {
            Piece &piece = pieces[i];
            if (piece.type == None || piece.isWhite != maximizingPlayer)
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

        std::sort(allMoves.begin(), allMoves.end(),
                  [this](const Move &a, const Move &b)
                  {
                      return scoreMove(a) > scoreMove(b);
                  });

        if (maximizingPlayer)
        {
            int maxEval = -999999;
            for (Move &move : allMoves)
            {
                Move m = makeMove(move.from, move.to, true);
                int eval = alphaBeta(depth - 1, alpha, beta, false);
                unmakeMove(m);

                maxEval = std::max(maxEval, eval);
                alpha = std::max(alpha, eval);

                if (beta <= alpha)
                    break;
            }
            return maxEval;
        }
        else
        {
            int minEval = 999999;
            for (Move &move : allMoves)
            {
                Move m = makeMove(move.from, move.to, true);
                int eval = alphaBeta(depth - 1, alpha, beta, true);
                unmakeMove(m);

                minEval = std::min(minEval, eval);
                beta = std::min(beta, eval);

                if (beta <= alpha)
                    break;
            }
            return minEval;
        }
    }

    Move chooseComputerMove(bool isWhite)
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

        if (allMoves.empty())
        {
            if (!isKingInCheck(isWhite))
                checkmate = 2;
            else
                checkmate = isWhite ? 0 : 1;
            return Move{-1, -1};
        }

        std::sort(allMoves.begin(), allMoves.end(),
                  [this](const Move &a, const Move &b)
                  {
                      return scoreMove(a) > scoreMove(b);
                  });

        int bestScore = isWhite ? -999999 : 999999;
        Move bestMove = allMoves[0];
        int searchDepth = 3;

        for (Move &move : allMoves)
        {
            Move m = makeMove(move.from, move.to, true);
            int score = alphaBeta(searchDepth - 1, -999999, 999999, !isWhite);
            unmakeMove(m);

            if ((isWhite && score > bestScore) || (!isWhite && score < bestScore))
            {
                bestScore = score;
                bestMove = move;
            }
        }

        return bestMove;
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

    void findPieces()
    {
        pawns[0].clear();
        pawns[1].clear();
        rooks[0].clear();
        rooks[1].clear();
        knights[0].clear();
        knights[1].clear();
        bishops[0].clear();
        bishops[1].clear();
        queens[0].clear();
        queens[1].clear();
        kings[0].clear();
        kings[1].clear();

        for (auto &p : pieces)
        {
            int c = p.isWhite ? 0 : 1;
            switch (p.type)
            {
            case PieceType::Pawn:
                pawns[c].push_back(&p);
                break;
            case PieceType::Rook:
                rooks[c].push_back(&p);
                break;
            case PieceType::Knight:
                knights[c].push_back(&p);
                break;
            case PieceType::Bishop:
                bishops[c].push_back(&p);
                break;
            case PieceType::Queen:
                queens[c].push_back(&p);
                break;
            case PieceType::King:
                kings[c].push_back(&p);
                break;
            default:
                break;
            }
        }
    }

    void updatePieces()
    {
        findPieces(); // just call this whenever pieces move/capture
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