#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

class Stopwatch
{
private:
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point stop_time;
    bool is_running = false;

public:
    void start()
    {
        start_time = std::chrono::steady_clock::now();
        is_running = true;
    }

    void stop()
    {
        stop_time = std::chrono::steady_clock::now();
        is_running = false;
    }

    // Returns the duration in milliseconds as an integer
    long long getElapsedTimeMilliseconds() const
    {
        std::chrono::steady_clock::time_point end_time = is_running ? std::chrono::steady_clock::now() : stop_time;
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        return duration.count();
    }

    // Returns the duration in seconds with decimal precision
    double getElapsedTimeSeconds() const
    {
        std::chrono::steady_clock::time_point end_time = is_running ? std::chrono::steady_clock::now() : stop_time;
        std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time);
        return duration.count();
    }
};