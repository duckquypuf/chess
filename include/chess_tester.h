#pragma once

#include "board.h"
#include <chrono>
#include <vector>
#include <string>

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
};