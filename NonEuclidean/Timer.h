#pragma once
#include <chrono>
#include <stdint.h>

class Timer {
public:
  Timer() {
    frequency = std::chrono::high_resolution_clock::period::den /
                std::chrono::high_resolution_clock::period::num;
  }

  void Start() { t1 = std::chrono::high_resolution_clock::now(); }

  float Stop() {
    t2 = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
    return duration.count() / 1e9f;
  }

  int64_t GetTicks() {
    t2 = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - startTime);
    return duration.count();
  }

  int64_t SecondsToTicks(float s) { return int64_t(s * 1e9); }

  float StopStart() {
    const float result = Stop();
    t1 = t2;
    return result;
  }

private:
  int64_t frequency; // ticks per second (nanoseconds)
  std::chrono::high_resolution_clock::time_point startTime =
      std::chrono::high_resolution_clock::now();
  std::chrono::high_resolution_clock::time_point t1, t2; // time points
};
