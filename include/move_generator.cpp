#include "move_generator.h"
#include "board.h"

std::vector<Move> MoveGen::generateMoves(const Board *board)
{
    moves.clear();

    for (int startSquare = 0; startSquare < SquareCount; startSquare++)
    {
        const Piece &piece = board->pieces[startSquare];

        if (piece.type != None && piece.isWhite == board->isWhiteTurn)
        {
            if (PieceData::IsSlidingPiece(piece.type))
            {
                generateSlidingMoves(board, startSquare, piece);
            }
            else if (piece.type == Knight)
            {
                generateKnightMoves(board, startSquare, piece);
            }
            else if (piece.type == King)
            {
                generateKingMoves(board, startSquare, piece);
            }
            else if (piece.type == Pawn)
            {
                generatePawnMoves(board, startSquare, piece);
            }
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

    // TODO: Add castling moves
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