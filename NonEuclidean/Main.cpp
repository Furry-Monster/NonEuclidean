#define _CRT_SECURE_NO_WARNINGS
#include "Engine.h"
#include <iostream>
#include <limits.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  char exePath[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", exePath, PATH_MAX);
  if (count != -1) {
    exePath[count] = '\0';
    for (int i = count - 1; i >= 0; --i) {
      if (exePath[i] == '/') {
        exePath[i] = '\0';
        break;
      }
    }
    if (chdir(exePath) != 0) {
      std::cerr << "Warning: Could not change to executable directory"
                << std::endl;
    }
  }

  Engine engine;
  return engine.Run();
}
