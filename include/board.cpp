#include "board.h"
#include "renderer.h"
#include "window.h"

inline float ease(float t)
{
    return 0.5f - cos(t * 3.14159265f) * 0.5f;
}

void Board::drawPieces(Renderer &renderer, Window &window)
{
    for (int i = 0; i < 64; i++)
    {
        Piece &piece = pieces[i];
        if (piece.type == None)
            continue;

        if (i == selectedSquare && isDragging)
            continue;

        glm::vec2 pos = squareToWorldPos(i);

        if (isAnimating && i == animMove.from)
        {
            glm::vec2 from = squareToWorldPos(animMove.from);
            glm::vec2 to = squareToWorldPos(animMove.to);
            float t = ease(animT/animDuration);
            pos = glm::mix(from, to, t);
        }

        renderer.drawPiece(this, window, piece, pos, i);
    }

    if(selectedSquare >= 0 && selectedSquare <= 63 && isDragging)
        renderer.drawPiece(this, window, pieces[selectedSquare], glm::vec2(0.0f), selectedSquare);
}