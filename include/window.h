#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <sstream>
#include <iomanip>

struct InputState
{
    bool leftMouseButton = false;
    bool rightMouseButton = false;

    bool w = false;
    bool a = false;
    bool s = false;
    bool d = false;
    bool spacebar = false;

    float mouseX = 0.0f;
    float mouseY = 0.0f;
};

class Window
{
public:
    int screenWidth, screenHeight;
    GLFWwindow* window;
    std::string windowName;

    Window(std::string windowName, int screenWidth, int screenHeight)
    {
        this->screenWidth = screenWidth;
        this->screenHeight = screenHeight;
        this->windowName = windowName;
        initOpenGL();
    }

    void initOpenGL()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(screenWidth, screenHeight, windowName.c_str(), NULL, NULL);

        if (window == NULL)
        {
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            glfwTerminate();
            return;
        }
    }

    void processInput()
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    }

    InputState getInput()
    {
        InputState input{};

        input.w = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
        input.s = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        input.a = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
        input.d = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
        input.spacebar = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
 
        double x, y;
        glfwGetCursorPos(window, &x, &y);
    
        input.mouseX = float(x - lastX);
        input.mouseY = float(lastY - y);

        lastX = x;
        lastY = y;

        input.leftMouseButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        input.rightMouseButton= glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

        return input;
    }

    int screenToSquare(float boardSize)
    {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        float squareSize = boardSize / 8.0f;
        float boardStartX = (screenWidth - boardSize) / 2.0f;
        float boardStartY = (screenHeight - boardSize) / 2.0f;

        if (mouseX < boardStartX || mouseX > boardStartX + boardSize ||
            mouseY < boardStartY || mouseY > boardStartY + boardSize)
        {
            return -1; // Outside board
        }

        int file = (int)((mouseX - boardStartX) / squareSize);
        int rank = 7 - (int)((mouseY - boardStartY) / squareSize); // Flip Y

        return rank * 8 + file;
    }

    void update()
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

private:
    double lastX, lastY;
    bool firstMouse = true;
    bool lastLeftMouseState = false;

public:
    bool wasLeftMouseJustPressed()
    {
        bool currentState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        bool justPressed = currentState && !lastLeftMouseState;
        lastLeftMouseState = currentState;
        return justPressed;
    }
};