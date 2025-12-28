#include "move_generator.h"
#include "board.h"

std::vector<Move> MoveGen::generateMoves(const Board *board)
{   
    moves.clear();

    for(int startSquare = 0; startSquare < BoardSize; startSquare++)
    {
        const Piece &piece = board->pieces[startSquare];

        if(piece.isWhite == board->isWhiteTurn)
        {
            if(PieceData::IsSlidingPiece(piece.type))
            {
                generateSlidingMoves(board, startSquare, piece);
            }
        }
    }

    return moves;
}

void MoveGen::generateSlidingMoves(const Board *board, int startSquare, const Piece &piece)
{
    int startDir = piece.type == Bishop ? 4 : 0;
    int endDir = piece.type == Rook ? 4 : 8;

    for(int direction = startDir; direction < endDir; direction++)
    {
        for(int n = 0; n < MoveGen::numSquaresToEdge[startSquare][direction]; n++)
        {
            int targetSquare = startSquare + MoveGen::directionOffsets[direction] * (n + 1);
            const Piece &targetPiece = board->pieces[targetSquare];

            if(targetPiece.isWhite == piece.isWhite && targetPiece.type != None)
                break;

            MoveGen::moves.push_back(Move(startSquare, targetSquare));

            if(targetPiece.isWhite == !piece.isWhite && targetPiece.type != None)
                break;
        }
    }
}