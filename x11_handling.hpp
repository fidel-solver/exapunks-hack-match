#ifndef X11_HANDLING_HPP
#define X11_HANDLING_HPP

#include <optional>

#include <X11/Xlib.h>

#include "board.hpp"

namespace HackMatch {
namespace X11Handling {
struct PhageAndBoard {
    uint8_t phageCol;
    Board::Board board;
};

Window getExapunksWindow(Display* display);
void validateAssumptions(Display* display, Window window);
void activateWindow(Display* display, Window window);
std::optional<PhageAndBoard> loadPhageAndBoardFromWindow(Display* display, Window window);
void moveLeft(Display* display);
void moveRight(Display* display);
void swap(Display* display);
void tractorBeam(Display *display);
}}
#endif
