#include <iostream>

#include "move_generator.h"
#include "board.h"

std::vector<Move> MoveGen::generateLegalMoves(Board *board, bool onlyGenCaptures)
{
    generateMoves(board);

    std::vector<Move> legal;
    legal.reserve(moves.size());

    for (auto &move : moves)
    {
        if (onlyGenCaptures && board->pieces[move.to].type == None && !move.wasEnPassant && !move.wasPromotion)
            continue;

        board->makeMove(move);
        const int ourKing = board->isWhiteTurn ? board->pieceList.blackKing : board->pieceList.whiteKing;

        if (!isSquareAttacked(board, ourKing, board->isWhiteTurn))
        {
            legal.push_back(move);
        }

        board->unmakeMove(move);
    }

    /*if (legal.empty() && !onlyGenCaptures)
    {
        // Check if king is in check
        int ourKing = board->isWhiteTurn ? board->pieceList.whiteKing : board->pieceList.blackKing;
        bool inCheck = isSquareAttacked(board, ourKing, !board->isWhiteTurn);

        if (inCheck)
        {
            board->checkmate = board->isWhiteTurn ? 1 : 0; // Checkmate
        }
        else
        {
            board->checkmate = 2; // Stalemate
        }
    }*/

    return legal;
}

bool MoveGen::isSquareAttacked(const Board *board, int square, bool byWhite)
{
    const auto &pieces = board->pieces;
    const int squareFile = square & 7;  // Faster than getFile
    const int squareRank = square >> 3; // Faster than getRank

    // Check pawns - most common, check first
    const int pawnDir = byWhite ? -8 : 8;

    int sq = square + pawnDir - 1;
    if (sq >= 0 && sq < 64 && abs((sq & 7) - squareFile) == 1)
    {
        const Piece &p = pieces[sq];
        if (p.type == Pawn && p.isWhite == byWhite)
            return true;
    }

    sq = square + pawnDir + 1;
    if (sq >= 0 && sq < 64 && abs((sq & 7) - squareFile) == 1)
    {
        const Piece &p = pieces[sq];
        if (p.type == Pawn && p.isWhite == byWhite)
            return true;
    }

    // Check knights - second most common
    static const int knightMoves[8] = {17, 15, 10, 6, -6, -10, -15, -17};
    for (int offset : knightMoves)
    {
        sq = square + offset;
        if (sq < 0 || sq >= 64)
            continue;

        const int fd = abs((sq & 7) - squareFile);
        const int rd = abs((sq >> 3) - squareRank);

        if ((fd == 2 && rd == 1) || (fd == 1 && rd == 2))
        {
            const Piece &p = pieces[sq];
            if (p.type == Knight && p.isWhite == byWhite)
                return true;
        }
    }

    // Check king
    for (int direction = 0; direction < 8; direction++)
    {
        sq = square + directionOffsets[direction];
        if (sq < 0 || sq >= 64)
            continue;

        const int fd = abs((sq & 7) - squareFile);
        const int rd = abs((sq >> 3) - squareRank);

        if (fd <= 1 && rd <= 1)
        {
            const Piece &p = pieces[sq];
            if (p.type == King && p.isWhite == byWhite)
                return true;
        }
    }

    // Check sliding pieces
    for (int direction = 0; direction < 8; direction++)
    {
        const int offset = directionOffsets[direction];
        const int maxSteps = numSquaresToEdge[square][direction];

        for (int n = 0; n < maxSteps; n++)
        {
            sq = square + offset * (n + 1);
            const Piece &piece = pieces[sq];

            if (piece.type == None)
                continue;

            if (piece.isWhite == byWhite)
            {
                const bool isOrthogonal = direction < 4;

                if (isOrthogonal)
                {
                    if (piece.type == Rook || piece.type == Queen)
                        return true;
                }
                else
                {
                    if (piece.type == Bishop || piece.type == Queen)
                        return true;
                }
            }

            break;
        }
    }

    return false;
}

std::vector<Move> MoveGen::generateMoves(const Board *board)
{
    moves.clear();
    moves.reserve(50);

    const PieceList &pl = board->pieceList;

    if (board->isWhiteTurn)
    {
        // Knights (simplest first)
        for (int sq : pl.whiteKnights)
            generateKnightMoves(board, sq, board->pieces[sq]);

        // Bishops
        for (int sq : pl.whiteBishops)
            generateSlidingMoves(board, sq, board->pieces[sq]);

        // Rooks
        for (int sq : pl.whiteRooks)
            generateSlidingMoves(board, sq, board->pieces[sq]);

        // Queens
        for (int sq : pl.whiteQueens)
            generateSlidingMoves(board, sq, board->pieces[sq]);

        // King
        if (pl.whiteKing != -1)
            generateKingMoves(board, pl.whiteKing, board->pieces[pl.whiteKing]);

        // Pawns (most complex last)
        for (int sq : pl.whitePawns)
            generatePawnMoves(board, sq, board->pieces[sq]);
    }
    else
    {
        for (int sq : pl.blackKnights)
            generateKnightMoves(board, sq, board->pieces[sq]);

        for (int sq : pl.blackBishops)
            generateSlidingMoves(board, sq, board->pieces[sq]);

        for (int sq : pl.blackRooks)
            generateSlidingMoves(board, sq, board->pieces[sq]);

        for (int sq : pl.blackQueens)
            generateSlidingMoves(board, sq, board->pieces[sq]);

        if (pl.blackKing != -1)
            generateKingMoves(board, pl.blackKing, board->pieces[pl.blackKing]);

        for (int sq : pl.blackPawns)
            generatePawnMoves(board, sq, board->pieces[sq]);
    }

    return moves;
}

void MoveGen::generateSlidingMoves(const Board *board, int startSquare, const Piece &piece)
{
    const int startDir = piece.type == Bishop ? 4 : 0;
    const int endDir = piece.type == Rook ? 4 : 8;
    const bool isWhite = piece.isWhite;
    const auto &pieces = board->pieces;

    for (int direction = startDir; direction < endDir; direction++)
    {
        const int offset = directionOffsets[direction];
        const int maxSteps = numSquaresToEdge[startSquare][direction];

        for (int n = 0; n < maxSteps; n++)
        {
            const int targetSquare = startSquare + offset * (n + 1);
            const Piece &targetPiece = pieces[targetSquare];

            // Blocked by own piece
            if (targetPiece.type != None && targetPiece.isWhite == isWhite)
                break;

            moves.push_back(Move(startSquare, targetSquare));

            // Blocked by opponent piece (capture, but can't move further)
            if (targetPiece.type != None)
                break;
        }
    }
}

void MoveGen::generateKnightMoves(const Board *board, int startSquare, const Piece &piece)
{
    const int startFile = getFile(startSquare);
    const int startRank = getRank(startSquare);
    const bool isWhite = piece.isWhite;
    const auto &pieces = board->pieces;

    for (int i = 0; i < 8; i++)
    {
        const int targetSquare = startSquare + knightOffsets[i];

        // Check if move is within board bounds
        if (targetSquare < 0 || targetSquare >= 64)
            continue;

        const int fileDiff = abs(getFile(targetSquare) - startFile);
        const int rankDiff = abs(getRank(targetSquare) - startRank);

        // Check if knight didn't wrap around the board
        if (!((fileDiff == 2 && rankDiff == 1) || (fileDiff == 1 && rankDiff == 2)))
            continue;

        const Piece &targetPiece = pieces[targetSquare];

        // Can't capture own piece
        if (targetPiece.type != None && targetPiece.isWhite == isWhite)
            continue;

        moves.push_back(Move(startSquare, targetSquare));
    }
}

void MoveGen::generateKingMoves(const Board *board, int startSquare, const Piece &piece)
{
    const int startFile = getFile(startSquare);
    const int startRank = getRank(startSquare);
    const bool isWhite = piece.isWhite;
    const auto &pieces = board->pieces;

    // King moves one square in any direction
    for (int direction = 0; direction < 8; direction++)
    {
        const int targetSquare = startSquare + directionOffsets[direction];

        if (targetSquare < 0 || targetSquare >= 64)
            continue;

        const int fileDiff = abs(getFile(targetSquare) - startFile);
        const int rankDiff = abs(getRank(targetSquare) - startRank);

        if (fileDiff > 1 || rankDiff > 1)
            continue;

        const Piece &targetPiece = pieces[targetSquare];

        if (targetPiece.type != None && targetPiece.isWhite == isWhite)
            continue;

        moves.push_back(Move(startSquare, targetSquare));
    }

    // Castling - with proper check detection
    if (piece.hasMoved)
        return;

    const int backRank = isWhite ? 0 : 56;

    // Can't castle if currently in check
    if (isSquareAttacked(board, startSquare, !isWhite))
        return;

    // Kingside castling
    const Piece &kingsideRook = pieces[backRank + 7];
    if (kingsideRook.type == Rook && !kingsideRook.hasMoved)
    {
        // Check if f and g files are empty
        if (pieces[backRank + 5].type == None &&
            pieces[backRank + 6].type == None)
        {
            // Check if king doesn't move THROUGH check (f file)
            if (!isSquareAttacked(board, backRank + 5, !isWhite))
            {
                // Check if king doesn't END in check is handled by generateLegalMoves
                moves.push_back(Move(startSquare, startSquare + 2, true));
            }
        }
    }

    // Queenside castling
    const Piece &queensideRook = pieces[backRank + 0];
    if (queensideRook.type == Rook && !queensideRook.hasMoved)
    {
        // Check if b, c, and d files are empty
        if (pieces[backRank + 1].type == None &&
            pieces[backRank + 2].type == None &&
            pieces[backRank + 3].type == None)
        {
            // Check if king doesn't move THROUGH check (c file)
            if (!isSquareAttacked(board, backRank + 3, !isWhite))
            {
                // Check if king doesn't END in check is handled by generateLegalMoves
                moves.push_back(Move(startSquare, startSquare - 2, true));
            }
        }
    }
}

void MoveGen::generatePawnMoves(const Board *board, int startSquare, const Piece &piece)
{
    const int startFile = getFile(startSquare);
    const int startRank = getRank(startSquare);
    const bool isWhite = piece.isWhite;
    const int direction = isWhite ? 8 : -8;
    const auto &pieces = board->pieces;

    // Check if pawn is on the 7th rank (white) or 2nd rank (black)
    const int promotionRank = isWhite ? 6 : 1; // Rank before promotion
    const bool willPromote = (startRank == promotionRank);

    // Forward one square
    const int oneForward = startSquare + direction;
    if (oneForward >= 0 && oneForward < 64 && pieces[oneForward].type == None)
    {
        if (willPromote)
        {
            // Add all 4 promotion options
            moves.push_back(Move(startSquare, oneForward, false, Queen));
            moves.push_back(Move(startSquare, oneForward, false, Rook));
            moves.push_back(Move(startSquare, oneForward, false, Bishop));
            moves.push_back(Move(startSquare, oneForward, false, Knight));
        }
        else
        {
            moves.push_back(Move(startSquare, oneForward));

            const bool isStartRank = isWhite ? (startRank == 1) : (startRank == 6);

            if (isStartRank)
            {
                const int twoForward = startSquare + direction * 2;
                if (pieces[twoForward].type == None)
                {
                    moves.push_back(Move(startSquare, twoForward));
                }
            }
        }
    }

    // Diagonal captures
    const int captureOffsets[2] = {direction + 1, direction - 1};

    for (int offset : captureOffsets)
    {
        const int targetSquare = startSquare + offset;

        if (targetSquare < 0 || targetSquare >= 64)
            continue;

        // Check if we didn't wrap around the board
        if (abs(getFile(targetSquare) - startFile) != 1)
            continue;

        const Piece &targetPiece = pieces[targetSquare];

        // Can capture enemy piece
        if (targetPiece.type != None && targetPiece.isWhite != isWhite)
        {
            if (willPromote)
            {
                // Add all 4 promotion options for captures
                moves.push_back(Move(startSquare, targetSquare, false, Queen));
                moves.push_back(Move(startSquare, targetSquare, false, Rook));
                moves.push_back(Move(startSquare, targetSquare, false, Bishop));
                moves.push_back(Move(startSquare, targetSquare, false, Knight));
            }
            else
            {
                moves.push_back(Move(startSquare, targetSquare));
            }
        }
        // En Passant
        else if (targetSquare == board->enPassantSquare)
        {
            moves.push_back(Move(startSquare, targetSquare));
        }
    }
}