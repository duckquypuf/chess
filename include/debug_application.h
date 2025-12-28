#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "chess_tester.h"
#include <iostream>

class DebugApplication
{
public:
    GLFWwindow *window;
    ChessTester tester;

    int selectedDepth = 3;
    int maxDepth = 5;
    bool isRunning = false;
    int currentTestIndex = 0;
    int currentDepth = 1;

    DebugApplication()
    {
        initWindow();
        initImGui();
    }

    ~DebugApplication()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(1200, 800, "Chess AI Performance Tester", NULL, NULL);

        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return;
        }
    }

    void initImGui()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 410");
    }

    void run()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            // Process one test per frame if running
            if (isRunning)
            {
                if (currentTestIndex < tester.testPositions.size() && currentDepth <= maxDepth)
                {
                    tester.runSingleTest(tester.testPositions[currentTestIndex], currentDepth);

                    currentDepth++;
                    if (currentDepth > maxDepth)
                    {
                        currentDepth = 1;
                        currentTestIndex++;
                    }
                }
                else
                {
                    isRunning = false;
                }
            }

            renderUI();

            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }
    }

    void renderUI()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Chess AI Performance Tester", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        ImGui::Text("Alpha-Beta Pruning Performance Analysis");
        ImGui::Separator();

        // Controls
        ImGui::Text("Test Configuration:");
        ImGui::SliderInt("Max Search Depth", &maxDepth, 1, 7);

        ImGui::Spacing();

        if (ImGui::Button("Run All Tests", ImVec2(150, 30)) && !isRunning)
        {
            tester.clearResults();
            currentTestIndex = 0;
            currentDepth = 1;
            isRunning = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Clear Results", ImVec2(150, 30)) && !isRunning)
        {
            tester.clearResults();
        }

        ImGui::SameLine();

        if (isRunning)
        {
            ImGui::Text("Running... (%d/%d)",
                        currentTestIndex * maxDepth + currentDepth - 1,
                        (int)tester.testPositions.size() * maxDepth);
        }
        else
        {
            ImGui::Text("Status: Ready");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Results Table
        ImGui::Text("Test Results:");

        if (ImGui::BeginTable("results", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 500)))
        {
            ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthFixed, 200);
            ImGui::TableSetupColumn("Depth", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("Best Move", ImGuiTableColumnFlags_WidthFixed, 100);
            ImGui::TableSetupColumn("Evaluation", ImGuiTableColumnFlags_WidthFixed, 100);
            ImGui::TableSetupColumn("Time (ms)", ImGuiTableColumnFlags_WidthFixed, 100);
            ImGui::TableSetupColumn("Time/Depth", ImGuiTableColumnFlags_WidthFixed, 100);
            ImGui::TableHeadersRow();

            for (const TestResult &result : tester.testResults)
            {
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("%s", result.positionName.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%d", result.depth);

                ImGui::TableNextColumn();
                std::string move = tester.squareToAlgebraic(result.bestMoveFrom) +
                                   " -> " +
                                   tester.squareToAlgebraic(result.bestMoveTo);
                ImGui::Text("%s", move.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%d", result.evaluation);

                ImGui::TableNextColumn();
                ImGui::Text("%.2f", result.timeMs);

                ImGui::TableNextColumn();
                double timePerDepth = result.timeMs / result.depth;
                ImGui::Text("%.2f", timePerDepth);
            }

            ImGui::EndTable();
        }

        // Summary Statistics
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Summary Statistics:");

        if (!tester.testResults.empty())
        {
            // Calculate averages per depth
            for (int depth = 1; depth <= maxDepth; depth++)
            {
                double totalTime = 0.0;
                int count = 0;

                for (const TestResult &result : tester.testResults)
                {
                    if (result.depth == depth)
                    {
                        totalTime += result.timeMs;
                        count++;
                    }
                }

                if (count > 0)
                {
                    double avgTime = totalTime / count;
                    ImGui::Text("  Depth %d: Avg Time = %.2f ms (%d positions)",
                                depth, avgTime, count);
                }
            }
        }

        ImGui::End();
    }
};