#include "board.h"
#include "renderer.h"
#include "window.h"

void Board::drawPieces(Renderer &renderer, Window &window)
{
    for (int i = 0; i < 64; i++)
    {
        Piece &piece = pieces[i];
        if (piece.type == None)
            continue;

        if (i != selectedSquare)
            renderer.drawPiece(this, window, piece, squareToWorldPos(i), i);
        else if (i == selectedSquare && isDragging)
            renderer.drawPiece(this, window, piece, glm::vec2(0.0f), i);
        else
            renderer.drawPiece(this, window, piece, squareToWorldPos(i), i);
    }
}