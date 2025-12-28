#include <iostream>
#include <iomanip>
#include "chess_tester.h"

void printResults(const std::vector<TestResult> &results, int maxDepth)
{
    std::cout << "\n";
    std::cout << "==========================================================================================================\n";
    std::cout << "                            CHESS AI PERFORMANCE TEST RESULTS                                          \n";
    std::cout << "==========================================================================================================\n\n";

    // Print header
    std::cout << std::left
              << std::setw(30) << "Position"
              << std::setw(8) << "Depth"
              << std::setw(15) << "Best Move"
              << std::setw(12) << "Evaluation"
              << std::setw(15) << "Time (ms)"
              << std::setw(15) << "Time/Depth"
              << "\n";
    std::cout << std::string(100, '-') << "\n";

    // Print results
    for (const TestResult &result : results)
    {
        std::string move = ChessTester().squareToAlgebraic(result.bestMoveFrom) +
                           " -> " +
                           ChessTester().squareToAlgebraic(result.bestMoveTo);

        double timePerDepth = result.timeMs / result.depth;

        std::cout << std::left
                  << std::setw(30) << result.positionName
                  << std::setw(8) << result.depth
                  << std::setw(15) << move
                  << std::setw(12) << result.evaluation
                  << std::setw(15) << std::fixed << std::setprecision(2) << result.timeMs
                  << std::setw(15) << std::fixed << std::setprecision(2) << timePerDepth
                  << "\n";
    }

    std::cout << std::string(100, '-') << "\n\n";

    // Print summary statistics
    std::cout << "SUMMARY STATISTICS:\n";
    std::cout << std::string(50, '-') << "\n";

    for (int depth = 1; depth <= maxDepth; depth++)
    {
        double totalTime = 0.0;
        int count = 0;

        for (const TestResult &result : results)
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
            std::cout << "Depth " << depth << ": "
                      << "Average Time = " << std::fixed << std::setprecision(2) << avgTime << " ms "
                      << "(" << count << " positions tested)\n";
        }
    }

    std::cout << std::string(100, '=') << "\n\n";
}

int main()
{
    ChessTester tester;

    std::cout << "\n";
    std::cout << "==========================================================================================================\n";
    std::cout << "                         CHESS AI ALPHA-BETA PERFORMANCE TESTER                                        \n";
    std::cout << "==========================================================================================================\n";
    std::cout << "\n";
    std::cout << "This tool tests the performance of alpha-beta pruning at different search depths.\n";
    std::cout << "\n";

    // Menu
    std::cout << "Options:\n";
    std::cout << "  1. Run new tests\n";
    std::cout << "  2. Compare with previous test results\n";
    std::cout << "Enter choice (1-2): ";

    int choice;
    std::cin >> choice;

    if (choice == 2)
    {
        std::cout << "Enter previous results filename (e.g., results_old.json): ";
        std::string filename;
        std::cin >> filename;

        tester.compareWithPrevious(filename);

        std::cout << "Press Enter to continue with new tests, or Ctrl+C to exit...";
        std::cin.ignore();
        std::cin.get();
    }

    // Get max depth from user
    int maxDepth;
    std::cout << "\nEnter maximum search depth to test (1-7, recommended: 3-5): ";
    std::cin >> maxDepth;

    if (maxDepth < 1)
        maxDepth = 1;
    if (maxDepth > 7)
        maxDepth = 7;

    std::cout << "\n";
    std::cout << "Testing " << tester.testPositions.size() << " positions at depths 1-" << maxDepth << "...\n";
    std::cout << "(Total tests: " << tester.testPositions.size() * maxDepth << ")\n\n";

    // Run tests
    int currentTest = 0;
    int totalTests = tester.testPositions.size() * maxDepth;

    for (const TestPosition &pos : tester.testPositions)
    {
        for (int depth = 1; depth <= maxDepth; depth++)
        {
            currentTest++;
            std::cout << "\rProgress: " << currentTest << "/" << totalTests
                      << " (" << (currentTest * 100 / totalTests) << "%) - Testing: "
                      << pos.name << " at depth " << depth << "       " << std::flush;

            tester.runSingleTest(pos, depth);
        }
    }

    std::cout << "\n\nAll tests complete!\n";

    // Print results
    printResults(tester.testResults, maxDepth);

    // Ask to export
    std::cout << "Export results to JSON? (y/n): ";
    char exportChoice;
    std::cin >> exportChoice;

    if (exportChoice == 'y' || exportChoice == 'Y')
    {
        // Generate filename with timestamp
        std::time_t now = std::time(nullptr);
        char filename[100];
        std::strftime(filename, sizeof(filename), "results_%Y%m%d_%H%M%S.json", std::localtime(&now));

        tester.exportToJSON(filename);

        std::cout << "\nYou can compare future runs against this file.\n";
    }

    // Keep window open
    std::cout << "\nPress Enter to exit...";
    std::cin.ignore();
    std::cin.get();

    return 0;
}