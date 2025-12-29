#include <iostream>

#include "move_generator.h"
#include "board.h"

std::vector<Move> MoveGen::generateLegalMoves(Board *board)
{
    std::vector<Move> pseudoLegal = generateMoves(board);
    std::vector<Move> legal;

    if(pseudoLegal.size() == 0)
    {
        board->checkmate = board->isWhiteTurn ? 1 : 0;
        return legal;
    }

    for(auto& move : pseudoLegal)
    {
        if (0 <= move.from && move.from <= 63 && 0 <= move.to && move.to <= 63)
        {
            board->makeMove(move);

            int ourKing = board->isWhiteTurn ? board->pieceList.blackKing : board->pieceList.whiteKing;

            bool leavesUsInCheck = isSquareAttacked(board, ourKing, board->isWhiteTurn);

            if (!leavesUsInCheck)
            {
                legal.push_back(move);
            }

            board->unmakeMove(move);
        }
    }

    return legal;
}

bool MoveGen::isSquareAttacked(const Board *board, int square, bool byWhite)
{
    // Check pawn attacks
    int pawnDirection = byWhite ? -8 : 8;  // White pawns attack upward, black downward
    int pawnAttacks[2] = {pawnDirection + 1, pawnDirection - 1};

    for (int offset : pawnAttacks)
    {
        int attackSquare = square + offset;

        if (attackSquare < 0 || attackSquare >= 64)
            continue;

        // Check if we didn't wrap around the board
        int squareFile = getFile(square);
        int attackFile = getFile(attackSquare);
        if (abs(attackFile - squareFile) != 1)
            continue;

        const Piece &piece = board->pieces[attackSquare];
        if (piece.type == Pawn && piece.isWhite == byWhite)
            return true;
    }

    // Check knight attacks
    for (int offset : knightOffsets)
    {
        int attackSquare = square + offset;

        if (attackSquare < 0 || attackSquare >= 64)
            continue;

        int squareFile = getFile(square);
        int squareRank = getRank(square);
        int attackFile = getFile(attackSquare);
        int attackRank = getRank(attackSquare);

        int fileDiff = abs(attackFile - squareFile);
        int rankDiff = abs(attackRank - squareRank);

        if (!((fileDiff == 2 && rankDiff == 1) || (fileDiff == 1 && rankDiff == 2)))
            continue;

        const Piece &piece = board->pieces[attackSquare];
        if (piece.type == Knight && piece.isWhite == byWhite)
            return true;
    }

    // Check king attacks
    for (int direction = 0; direction < 8; direction++)
    {
        int attackSquare = square + directionOffsets[direction];

        if (attackSquare < 0 || attackSquare >= 64)
            continue;

        int squareFile = getFile(square);
        int squareRank = getRank(square);
        int attackFile = getFile(attackSquare);
        int attackRank = getRank(attackSquare);

        if (abs(attackFile - squareFile) > 1 || abs(attackRank - squareRank) > 1)
            continue;

        const Piece &piece = board->pieces[attackSquare];
        if (piece.type == King && piece.isWhite == byWhite)
            return true;
    }

    // Check sliding pieces (bishop, rook, queen)
    for (int direction = 0; direction < 8; direction++)
    {
        for (int n = 0; n < numSquaresToEdge[square][direction]; n++)
        {
            int attackSquare = square + directionOffsets[direction] * (n + 1);
            const Piece &piece = board->pieces[attackSquare];

            if (piece.type == None)
                continue;

            // Found a piece - check if it's an attacker
            if (piece.isWhite == byWhite)
            {
                // Check if this piece can attack along this direction
                bool isOrthogonal = direction < 4; // N, S, W, E
                bool isDiagonal = direction >= 4;  // NW, SE, NE, SW

                if (isOrthogonal && (piece.type == Rook || piece.type == Queen))
                    return true;

                if (isDiagonal && (piece.type == Bishop || piece.type == Queen))
                    return true;
            }

            // Piece blocks further attacks in this direction
            break;
        }
    }

    return false;
}

std::vector<Move> MoveGen::generateMoves(const Board *board)
{
    moves.clear();

    // Use piece lists for optimization!
    if (board->isWhiteTurn)
    {
        // White pieces
        for (int sq : board->pieceList.whitePawns)
        {
            generatePawnMoves(board, sq, board->pieces[sq]);
        }
        for (int sq : board->pieceList.whiteKnights)
        {
            generateKnightMoves(board, sq, board->pieces[sq]);
        }
        for (int sq : board->pieceList.whiteBishops)
        {
            generateSlidingMoves(board, sq, board->pieces[sq]);
        }
        for (int sq : board->pieceList.whiteRooks)
        {
            generateSlidingMoves(board, sq, board->pieces[sq]);
        }
        for (int sq : board->pieceList.whiteQueens)
        {
            generateSlidingMoves(board, sq, board->pieces[sq]);
        }
        if (board->pieceList.whiteKing != -1)
        {
            generateKingMoves(board, board->pieceList.whiteKing, board->pieces[board->pieceList.whiteKing]);
        }
    }
    else
    {
        // Black pieces
        for (int sq : board->pieceList.blackPawns)
        {
            generatePawnMoves(board, sq, board->pieces[sq]);
        }
        for (int sq : board->pieceList.blackKnights)
        {
            generateKnightMoves(board, sq, board->pieces[sq]);
        }
        for (int sq : board->pieceList.blackBishops)
        {
            generateSlidingMoves(board, sq, board->pieces[sq]);
        }
        for (int sq : board->pieceList.blackRooks)
        {
            generateSlidingMoves(board, sq, board->pieces[sq]);
        }
        for (int sq : board->pieceList.blackQueens)
        {
            generateSlidingMoves(board, sq, board->pieces[sq]);
        }
        if (board->pieceList.blackKing != -1)
        {
            generateKingMoves(board, board->pieceList.blackKing, board->pieces[board->pieceList.blackKing]);
        }
    }

    return moves;
}

void MoveGen::generateSlidingMoves(const Board *board, int startSquare, const Piece &piece)
{
    int startDir = piece.type == Bishop ? 4 : 0;
    int endDir = piece.type == Rook ? 4 : 8;

    for (int direction = startDir; direction < endDir; direction++)
    {
        for (int n = 0; n < numSquaresToEdge[startSquare][direction]; n++)
        {
            int targetSquare = startSquare + directionOffsets[direction] * (n + 1);
            const Piece &targetPiece = board->pieces[targetSquare];

            // Blocked by own piece
            if (targetPiece.type != None && targetPiece.isWhite == piece.isWhite)
                break;

            moves.push_back(Move(startSquare, targetSquare));

            // Blocked by opponent piece (capture, but can't move further)
            if (targetPiece.type != None && targetPiece.isWhite != piece.isWhite)
                break;
        }
    }
}

void MoveGen::generateKnightMoves(const Board *board, int startSquare, const Piece &piece)
{
    int startFile = getFile(startSquare);
    int startRank = getRank(startSquare);

    for (int i = 0; i < 8; i++)
    {
        int targetSquare = startSquare + knightOffsets[i];

        // Check if move is within board bounds
        if (targetSquare < 0 || targetSquare >= 64)
            continue;

        int targetFile = getFile(targetSquare);
        int targetRank = getRank(targetSquare);

        // Check if knight didn't wrap around the board
        int fileDiff = abs(targetFile - startFile);
        int rankDiff = abs(targetRank - startRank);

        if (!((fileDiff == 2 && rankDiff == 1) || (fileDiff == 1 && rankDiff == 2)))
            continue;

        const Piece &targetPiece = board->pieces[targetSquare];

        // Can't capture own piece
        if (targetPiece.type != None && targetPiece.isWhite == piece.isWhite)
            continue;

        moves.push_back(Move(startSquare, targetSquare));
    }
}

void MoveGen::generateKingMoves(const Board *board, int startSquare, const Piece &piece)
{
    int startFile = getFile(startSquare);
    int startRank = getRank(startSquare);

    // King moves one square in any direction
    for (int direction = 0; direction < 8; direction++)
    {
        int targetSquare = startSquare + directionOffsets[direction];

        // Check bounds
        if (targetSquare < 0 || targetSquare >= 64)
            continue;

        int targetFile = getFile(targetSquare);
        int targetRank = getRank(targetSquare);

        // Prevent wrapping around board edges
        if (abs(targetFile - startFile) > 1 || abs(targetRank - startRank) > 1)
            continue;

        const Piece &targetPiece = board->pieces[targetSquare];

        // Can't capture own piece
        if (targetPiece.type != None && targetPiece.isWhite == piece.isWhite)
            continue;

        moves.push_back(Move(startSquare, targetSquare));
    }

    // Castling Moves
    if (piece.hasMoved)
        return;

    int backRank = piece.isWhite ? 0 : 56;

    // Kingside castling (king moves to g-file)
    const Piece &kingsideRook = board->pieces[backRank + 7];
    if (kingsideRook.type == Rook && !kingsideRook.hasMoved)
    {
        // Check if squares between king and rook are empty
        bool pathClear = true;
        for (int sq = startSquare + 1; sq < backRank + 7; sq++)
        {
            if (board->pieces[sq].type != None)
            {
                pathClear = false;
                break;
            }
        }

        if (pathClear)
        {
            // King moves to g-file (2 squares right)
            moves.push_back(Move(startSquare, startSquare + 2, true));
        }
    }

    // Queenside castling (king moves to c-file)
    const Piece &queensideRook = board->pieces[backRank + 0];
    if (queensideRook.type == Rook && !queensideRook.hasMoved)
    {
        // Check if squares between king and rook are empty
        bool pathClear = true;
        for (int sq = backRank + 1; sq < startSquare; sq++)
        {
            if (board->pieces[sq].type != None)
            {
                pathClear = false;
                break;
            }
        }

        if (pathClear)
        {
            // King moves to c-file (2 squares left)
            moves.push_back(Move(startSquare, startSquare - 2, true));
        }
    }
}

void MoveGen::generatePawnMoves(const Board *board, int startSquare, const Piece &piece)
{
    int startFile = getFile(startSquare);
    int startRank = getRank(startSquare);

    // Direction: white pawns move up (+8), black pawns move down (-8)
    int direction = piece.isWhite ? 8 : -8;

    // Forward one square
    int oneForward = startSquare + direction;
    if (oneForward >= 0 && oneForward < 64)
    {
        const Piece &targetPiece = board->pieces[oneForward];
        if (targetPiece.type == None)
        {
            moves.push_back(Move(startSquare, oneForward));

            // Forward two squares from starting position
            if (!piece.hasMoved)
            {
                int twoForward = startSquare + direction * 2;
                const Piece &twoForwardPiece = board->pieces[twoForward];
                if (twoForwardPiece.type == None)
                {
                    moves.push_back(Move(startSquare, twoForward));
                }
            }
        }
    }

    // Diagonal captures
    int captureOffsets[2] = {direction + 1, direction - 1};

    for (int offset : captureOffsets)
    {
        int targetSquare = startSquare + offset;

        if (targetSquare < 0 || targetSquare >= 64)
            continue;

        int targetFile = getFile(targetSquare);

        // Check if we didn't wrap around the board
        if (abs(targetFile - startFile) != 1)
            continue;

        const Piece &targetPiece = board->pieces[targetSquare];

        // Can only move diagonally if capturing an enemy piece
        if (targetPiece.type != None && targetPiece.isWhite != piece.isWhite)
        {
            moves.push_back(Move(startSquare, targetSquare));
        }

        // En Passant
        if (board->enPassantSquare != -1 && targetSquare == board->enPassantSquare)
        {
            // Verify pawn is on correct rank for en passant
            int requiredRank = piece.isWhite ? 4 : 3;
            if (startRank == requiredRank)
            {
                moves.push_back(Move(startSquare, targetSquare));
            }
        }
    }
}