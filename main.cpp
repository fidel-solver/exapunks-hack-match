//clang++ -O3 -Wall -Werror -Wextra -std=c++17 board.cpp x11_handling.cpp solver.cpp main.cpp -lX11 -lXtst

#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <optional>
#include <thread>
#include <vector>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

#include "x11_handling.hpp"
#include "board.hpp"
#include "solver.hpp"

int main(int, char**) {
    Display *display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        std::cerr << "failed to open display\n";
        return 1;
    }
    using namespace HackMatch;
    Window window = X11Handling::getExapunksWindow(display);
    X11Handling::validateAssumptions(display, window);
    X11Handling::activateWindow(display, window);
    std::vector<Solver::Move> moves;
    moves.reserve(100);
    while (true) {
        std::optional<X11Handling::PhageAndBoard> phageAndBoard = X11Handling::loadPhageAndBoardFromWindow(display, window);
        if (!phageAndBoard) {
            continue;
        }
        printBoard(phageAndBoard->board);
        Solver::solve(phageAndBoard->board, moves);
        Solver::printMoves(moves);
        uint8_t phageCol = phageAndBoard->phageCol;
        for (const auto& move : moves) {
            while (move.col > phageCol) {
                X11Handling::moveRight(display);
                ++phageCol;
            }
            while (move.col < phageCol) {
                X11Handling::moveLeft(display);
                --phageCol;
            }
            switch (move.command) {
            case Solver::PUT:
                X11Handling::tractorBeam(display);
                break;
            case Solver::TAKE:
                X11Handling::tractorBeam(display);
                break;
            case Solver::SWAP:
                X11Handling::swap(display);
                break;
            default:
                std::cerr << "bad command in solution: " << static_cast<int>(move.command) << '\n';
                throw std::runtime_error("bad command in solution");
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(17*4+3));
    }
    return 0;
}
