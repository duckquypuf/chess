#include "move_generator.h"
#include "board.h"

inline std::vector<Move> MoveGen::generateMoves(const Board *board)
{   
    moves.clear();

    for(int startSquare = 0; startSquare < BoardSize; startSquare++)
    {
        Piece &piece = board->pieces[startSquare];

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

inline std::vector<Move> generateSlidingMoves(const Board *board, int startSquare, Piece &piece)
{
    int startDir = piece.type == Bishop ? 4 : 0;
    int endDir = piece.type == Rook ? 4 : 8;

    for(int direction = startDir; direction < endDir; direction++)
    {
        for(int n = 0; n < MoveGen::numSquaresToEdge[startSquare][direction]; n++)
        {
            int targetSquare = startSquare + MoveGen::directionOffsets[direction] * (n + 1);
            Piece &targetPiece = board->pieces[targetSquare];

            if(targetPiece.isWhite == piece.isWhite && targetPiece.type != none)
                break;

            MoveGen::moves.push_back(Move(startSquare, targetSquare));

            if(targetPiece.isWhite == !piece.isWhite && targetPiece.type != None)
                break;
        }
    }
}