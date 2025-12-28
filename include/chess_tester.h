#pragma once

#include "board.h"
#include <chrono>
#include <vector>
#include <string>
#include <fstream>
#include <ctime>

struct TestPosition
{
    std::string name;
    std::string fen;
    bool isWhiteTurn;
};

struct TestResult
{
    std::string positionName;
    int depth;
    int bestMoveFrom;
    int bestMoveTo;
    int evaluation;
    double timeMs;
};

struct BestMoveResult
{
    Move move;
    int evaluation;
};

class ChessTester
{
public:
    std::vector<TestPosition> testPositions = {
        {"Starting Position", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR", true},
        {"Mid Game", "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R", true},
        {"Endgame - Rook vs Pawns", "8/5pk1/6p1/8/8/6P1/5PKR/8", true},
        {"Tactical - Fork Available", "rnbqkb1r/pppp1ppp/5n2/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R", true},
        {"Scholar's Mate Position", "r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR", true},
        {"Open Position", "rnbqkb1r/ppp2ppp/4pn2/3p4/2PP4/2N2N2/PP2PPPP/R1BQKB1R", true},
    };

    std::vector<TestResult> testResults;

    void runSingleTest(const TestPosition &pos, int depth)
    {
        Board board;
        board.pieces = loadFenString(pos.fen.c_str());
        board.isWhiteTurn = pos.isWhiteTurn;
        board.findKings();

        auto start = std::chrono::high_resolution_clock::now();

        BestMoveResult result = findBestMove(board, depth, pos.isWhiteTurn);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double timeMs = duration.count() / 1000.0;

        TestResult testResult;
        testResult.positionName = pos.name;
        testResult.depth = depth;
        testResult.bestMoveFrom = result.move.from;
        testResult.bestMoveTo = result.move.to;
        testResult.evaluation = result.evaluation;
        testResult.timeMs = timeMs;

        testResults.push_back(testResult);
    }

    void runAllTests(int maxDepth)
    {
        testResults.clear();

        for (const TestPosition &pos : testPositions)
        {
            for (int depth = 1; depth <= maxDepth; depth++)
            {
                runSingleTest(pos, depth);
            }
        }
    }

    BestMoveResult findBestMove(Board &board, int depth, bool isWhite)
    {
        std::vector<Move> allMoves;

        for (int i = 0; i < 64; i++)
        {
            Piece &piece = board.pieces[i];
            if (piece.type == None || piece.isWhite != isWhite)
                continue;

            std::vector<int> moves = board.generateLegalMoves(i);
            for (int to : moves)
            {
                Move m;
                m.from = i;
                m.to = to;
                allMoves.push_back(m);
            }
        }

        BestMoveResult result;

        if (allMoves.empty())
        {
            result.move.from = -1;
            result.move.to = -1;
            result.evaluation = 0;
            return result;
        }

        int bestScore = isWhite ? -999999 : 999999;
        Move bestMove = allMoves[0];

        for (Move &move : allMoves)
        {
            Move m = board.makeMove(move.from, move.to, true);
            int score = board.alphaBeta(depth - 1, -999999, 999999, !isWhite);
            board.unmakeMove(m);

            if ((isWhite && score > bestScore) || (!isWhite && score < bestScore))
            {
                bestScore = score;
                bestMove = move;
            }
        }

        result.move = bestMove;
        result.evaluation = bestScore;
        return result;
    }

    void clearResults()
    {
        testResults.clear();
    }

    std::string squareToAlgebraic(int square)
    {
        if (square < 0 || square > 63)
            return "??";

        char file = 'a' + (square % 8);
        char rank = '1' + (square / 8);
        return std::string(1, file) + std::string(1, rank);
    }

    void exportToJSON(const std::string &filename)
    {
        std::ofstream file(filename);

        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }

        // Get timestamp
        std::time_t now = std::time(nullptr);
        char timestamp[100];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        file << "{\n";
        file << "  \"timestamp\": \"" << timestamp << "\",\n";
        file << "  \"total_tests\": " << testResults.size() << ",\n";
        file << "  \"results\": [\n";

        for (size_t i = 0; i < testResults.size(); i++)
        {
            const TestResult &r = testResults[i];

            file << "    {\n";
            file << "      \"position\": \"" << r.positionName << "\",\n";
            file << "      \"depth\": " << r.depth << ",\n";
            file << "      \"best_move_from\": " << r.bestMoveFrom << ",\n";
            file << "      \"best_move_to\": " << r.bestMoveTo << ",\n";
            file << "      \"best_move_algebraic\": \"" << squareToAlgebraic(r.bestMoveFrom)
                 << " -> " << squareToAlgebraic(r.bestMoveTo) << "\",\n";
            file << "      \"evaluation\": " << r.evaluation << ",\n";
            file << "      \"time_ms\": " << std::fixed << std::setprecision(2) << r.timeMs << "\n";
            file << "    }";

            if (i < testResults.size() - 1)
                file << ",";

            file << "\n";
        }

        file << "  ],\n";

        // Calculate summary statistics
        file << "  \"summary\": {\n";

        // Find max depth
        int maxDepth = 0;
        for (const auto &r : testResults)
            maxDepth = std::max(maxDepth, r.depth);

        // Stats per depth
        for (int depth = 1; depth <= maxDepth; depth++)
        {
            double totalTime = 0.0;
            int count = 0;

            for (const auto &r : testResults)
            {
                if (r.depth == depth)
                {
                    totalTime += r.timeMs;
                    count++;
                }
            }

            if (count > 0)
            {
                file << "    \"depth_" << depth << "\": {\n";
                file << "      \"avg_time_ms\": " << std::fixed << std::setprecision(2)
                     << (totalTime / count) << ",\n";
                file << "      \"positions_tested\": " << count << "\n";
                file << "    }";

                if (depth < maxDepth)
                    file << ",";

                file << "\n";
            }
        }

        file << "  }\n";
        file << "}\n";

        file.close();

        std::cout << "Results exported to: " << filename << std::endl;
    }

    // Import and compare with previous results
    void compareWithPrevious(const std::string &filename)
    {
        std::ifstream file(filename);

        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }

        std::cout << "\n=== Comparison with Previous Run ===\n";
        std::cout << "Previous results from: " << filename << "\n\n";

        // Simple parsing - look for depth summaries
        std::string line;
        std::map<int, double> previousTimes;

        while (std::getline(file, line))
        {
            // Look for "depth_N": { "avg_time_ms": X.XX
            if (line.find("\"depth_") != std::string::npos)
            {
                size_t depthPos = line.find("depth_") + 6;
                size_t endPos = line.find("\"", depthPos);
                int depth = std::stoi(line.substr(depthPos, endPos - depthPos));

                // Next line should have avg_time_ms
                if (std::getline(file, line))
                {
                    size_t timePos = line.find("avg_time_ms\": ") + 14;
                    size_t endTime = line.find(",", timePos);
                    double time = std::stod(line.substr(timePos, endTime - timePos));

                    previousTimes[depth] = time;
                }
            }
        }

        file.close();

        // Compare with current results
        std::map<int, double> currentTimes;
        std::map<int, int> counts;

        for (const auto &r : testResults)
        {
            currentTimes[r.depth] += r.timeMs;
            counts[r.depth]++;
        }

        for (auto &pair : currentTimes)
        {
            pair.second /= counts[pair.first];
        }

        // Print comparison
        std::cout << std::left << std::setw(8) << "Depth"
                  << std::setw(15) << "Previous (ms)"
                  << std::setw(15) << "Current (ms)"
                  << std::setw(15) << "Speedup"
                  << std::setw(15) << "Improvement"
                  << "\n";
        std::cout << std::string(65, '-') << "\n";

        for (const auto &pair : currentTimes)
        {
            int depth = pair.first;
            double current = pair.second;

            if (previousTimes.count(depth))
            {
                double previous = previousTimes[depth];
                double speedup = previous / current;
                double improvement = 100.0 * (1.0 - current / previous);

                std::cout << std::left << std::setw(8) << depth
                          << std::setw(15) << std::fixed << std::setprecision(2) << previous
                          << std::setw(15) << current
                          << std::setw(15) << speedup << "x"
                          << std::setw(15) << improvement << "%"
                          << "\n";
            }
        }

        std::cout << "\n";
    }
};