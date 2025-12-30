#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <algorithm>
#include <string>

#include "stb_image.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "shader.h"
#include "window.h"
#include "piece.h"
#include "move_generator.h"
#include "board.h"

class Renderer
{
public:
    float smallestDimension = 0.0f;

    Renderer(const char *vertPath, const char *fragPath, Window window, const char *pieceVPath, const char *pieceFPath)
    {
        shader = new Shader(vertPath, fragPath);
        pieceShader = new Shader(pieceVPath, pieceFPath);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        stbi_set_flip_vertically_on_load(true);

        smallestDimension = std::min(window.screenHeight, window.screenWidth);

        for(int i = 0; i < 6; i++)
        {
            whiteTextures[i] = loadTexture(
                (std::string("../assets/textures/") + std::to_string(i) + ".png").c_str()
            );

            blackTextures[i] = loadTexture(
                (std::string("../assets/textures/b") + std::to_string(i) + ".png").c_str()
            );
        }
    }

    ~Renderer()
    {
        delete shader;
        delete pieceShader;
    }

    void beginFrame()
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void drawPiece(Board *board, Window &window, const Piece &piece, glm::vec2 pos, int i)
    {
        if (piece.type == None)
            return;

        pieceShader->use();

        float square = smallestDimension / 8.0f;
        float sx = square / window.screenWidth;
        float sy = square / window.screenHeight;

        pieceShader->setVec2("scale", glm::vec2(sx, sy));

        if(!board->isDragging || board->selectedSquare != i)
        {
            float ox = -1.0f + (window.screenWidth - smallestDimension) / window.screenWidth - sx + (sx * pos.x) * 2.0f;
            float oy = -1.0f + (window.screenHeight - smallestDimension) / window.screenHeight - sy + (sy * pos.y) * 2.0f;
            pieceShader->setVec2("offset", glm::vec2(ox, oy));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, piece.isWhite ? whiteTextures[piece.type] : blackTextures[piece.type]);
            pieceShader->setInt("pieceTex", 0);

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        

        if (board->isDragging && board->selectedSquare == i)
        {
            glm::vec2 mousePos = window.getMousePosition();

            // Convert to normalized device coordinates centered on mouse
            float nx = (mousePos.x / window.screenWidth) * 2.0f - 1.0f;
            float ny = 1.0f - (mousePos.y / window.screenHeight) * 2.0f;

            pieceShader->setVec2("offset", glm::vec2(nx, ny));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, piece.isWhite ? whiteTextures[piece.type] : blackTextures[piece.type]);
            pieceShader->setInt("pieceTex", 0);

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

    void render(Window window, const Board &board)
    {
        shader->use();

        shader->setVec2("screenRes", glm::vec2(window.screenWidth, window.screenHeight));
        shader->setFloat("boardSize", smallestDimension);

        shader->setVec3("whiteColour", glm::vec3(0.891, 0.818, 0.719));
        shader->setVec3("blackColour", glm::vec3(0.579, 0.432, 0.360));

        shader->setVec3("selectedColour", glm::vec3(0.813, 0.458, 0.187));

        shader->setInt("selectedSquare", board.selectedSquare);

        std::vector<int> legalMovesForPiece;

        for(auto& move : board.legalMoves)
        {
            if(move.from == board.selectedSquare)
            {
                legalMovesForPiece.push_back(move.to);
            }
        }
        
        shader->setInt("numLegalMoves", legalMovesForPiece.size());

        for (int i = 0; i < std::min((int)legalMovesForPiece.size(), 32); i++)
        {
            shader->setInt(("legalMoves[" + std::to_string(i) + "]").c_str(), legalMovesForPiece[i]);
        }

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

private:
    Shader *shader;
    Shader *pieceShader;

    uint VAO = 0, VBO = 0;

    static constexpr float quadVertices[24] =
        {
            -1.0f, -1.0f, 0.0f, 0.0f,
            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,

            1.0f, -1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f};

    uint whiteTextures[6];
    uint blackTextures[6];

    unsigned int loadTexture(const char *path)
    {
        if (!std::filesystem::exists(path))
        {
            std::cout << "ERROR: File does not exist at path: " << path << std::endl;
            return 0;
        }

        unsigned int id;
        glGenTextures(1, &id);

        int width, height, nrComponents;
        unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);

        if (data)
        {
            GLenum format;

            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, id);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "STB Image failed to load. Reason: " << stbi_failure_reason() << std::endl;
            std::cout << "Path attempted: " << path << std::endl;
            stbi_image_free(data);
            return 0;
        }
        return id;
    }
};