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
#include "camera.h"
#include "window.h"
#include "piece.h"

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

    void beginFrame()
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void render(Camera cam, Window window, std::vector<Piece> pieces, int selectedSquare, std::vector<int> legalMoves)
    {
        shader->use();

        glm::mat4 view = cam.GetViewMatrix();
        glm::mat4 proj = cam.GetProjectionMatrix();

        shader->setVec2("screenRes", glm::vec2(window.screenWidth, window.screenHeight));
        shader->setFloat("boardSize", smallestDimension);

        shader->setVec3("whiteColour", glm::vec3(0.913, 0.847, 0.709));
        shader->setVec3("blackColour", glm::vec3(0.658, 0.529, 0.397));

        shader->setVec3("selectedWhiteColour", glm::vec3(0.913, 0.847, 0.0));
        shader->setVec3("selectedBlackColour", glm::vec3(0.658, 0.529, 0.0));

        shader->setInt("selectedSquare", selectedSquare);
        shader->setInt("numLegalMoves", legalMoves.size());

        for (int i = 0; i < std::min((int)legalMoves.size(), 32); i++)
        {
            shader->setInt(("legalMoves[" + std::to_string(i) + "]").c_str(), legalMoves[i]);
        }

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        pieceShader->use();

        // --- Draw Pieces ---
        float square = smallestDimension / 8.0f;
        float sx = square / window.screenWidth;
        float sy = square / window.screenHeight;

        pieceShader->setVec2("scale", glm::vec2(sx, sy));

        for(int i = 0; i < 64; i++)
        {
            Piece piece = pieces[i];
            
            if(piece.type == None) continue;

            int posX = piece.pos % 8 + 1;
            int posY = floor((piece.pos + 8.0f) / 8.0f);

            float ox = -1.0f + (window.screenWidth - smallestDimension) / window.screenWidth - sx + (sx * posX) * 2.0f;
            float oy = -1.0f + (window.screenHeight - smallestDimension) / window.screenHeight - sy + (sy * posY) * 2.0f;
            pieceShader->setVec2("offset", glm::vec2(ox, oy));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, piece.isWhite ? whiteTextures[piece.type] : blackTextures[piece.type]);
            pieceShader->setInt("pieceTex", 0);

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
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
            std::cout << "Texture loaded successfully: " << width << "x" << height << " with " << nrComponents << " components" << std::endl;
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