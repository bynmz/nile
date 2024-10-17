#include "apps/game/2d/breakout/breakout.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main(int argc, char *argv[]) {
  nile::Breakout game;
  nile::App2D &app = game;
  
  try {
    app.start();

  return 0;

  } catch (const std::runtime_error &e) {
    std::cerr << "Runtime error: " << e.what() << '\n';
    return EXIT_FAILURE;
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << '\n';
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Unknown exception caught" << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}