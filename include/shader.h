#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <istream>

class Shader {
    public:
        unsigned int ID;

        Shader(const char *vertexPath, const char *fragmentPath, const char *geometryPath = nullptr)
        {
            std::string vertexCode, fragmentCode, geometryCode;
            std::ifstream vShaderFile, fShaderFile, gShaderFile;

            vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            if (geometryPath)
                gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            try
            {
                vShaderFile.open(vertexPath);
                fShaderFile.open(fragmentPath);
                std::stringstream vShaderStream, fShaderStream;

                vShaderStream << vShaderFile.rdbuf();
                fShaderStream << fShaderFile.rdbuf();

                vShaderFile.close();
                fShaderFile.close();

                vertexCode = vShaderStream.str();
                fragmentCode = fShaderStream.str();

                if (geometryPath)
                {
                    gShaderFile.open(geometryPath);
                    std::stringstream gShaderStream;
                    gShaderStream << gShaderFile.rdbuf();
                    gShaderFile.close();
                    geometryCode = gShaderStream.str();
                }
            }
            catch (std::ifstream::failure &e)
            {
                std::cout << "ERROR: Failed to read shader file(s)\n";
            }

            const char *vShaderCode = vertexCode.c_str();
            const char *fShaderCode = fragmentCode.c_str();

            unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vShaderCode, NULL);
            glCompileShader(vertex);
            checkCompileErrors(vertex, "VERTEX");

            unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fShaderCode, NULL);
            glCompileShader(fragment);
            checkCompileErrors(fragment, "FRAGMENT");

            unsigned int geometry;
            if (geometryPath)
            {
                const char *gShaderCode = geometryCode.c_str();
                geometry = glCreateShader(GL_GEOMETRY_SHADER);
                glShaderSource(geometry, 1, &gShaderCode, NULL);
                glCompileShader(geometry);
                checkCompileErrors(geometry, "GEOMETRY");
            }

            ID = glCreateProgram();
            glAttachShader(ID, vertex);
            glAttachShader(ID, fragment);
            if (geometryPath)
                glAttachShader(ID, geometry);
            glLinkProgram(ID);
            checkCompileErrors(ID, "PROGRAM");

            glDeleteShader(vertex);
            glDeleteShader(fragment);
            if (geometryPath)
                glDeleteShader(geometry);
        }

        void use() {
            glUseProgram(ID);
        }

        void setMat4(const char* name, glm::mat4 value) {
            unsigned int location = glGetUniformLocation(ID, name);
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
        }

        void setInt(const char* name, int value) {
            unsigned int location = glGetUniformLocation(ID, name);
            glUniform1i(location, value);
        }

        void setFloat(const char *name, float value) {
            unsigned int location = glGetUniformLocation(ID, name);
            glUniform1f(location, value);
        }

        void setVec2(const char *name, glm::vec2 value) {
            unsigned int location = glGetUniformLocation(ID, name);
            glUniform2fv(location, 1, glm::value_ptr(value));
        }

        void setVec3(const char* name, glm::vec3 value) {
            unsigned int location = glGetUniformLocation(ID, name);
            glUniform3fv(location, 1, glm::value_ptr(value));
        }

        void setVec4(const char* name, glm::vec4 value) {
            unsigned int location = glGetUniformLocation(ID, name);
            glUniform4fv(location, 1, glm::value_ptr(value));
        }

    private:
        void checkCompileErrors(unsigned int shader, std::string type)
        {
            int success;
            char infoLog[1024];
            if (type != "PROGRAM")
            {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success)
                {
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                              << infoLog << "\n";
                }
            }
            else
            {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if (!success)
                {
                    glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                              << infoLog << "\n";
                }
            }
        }
};