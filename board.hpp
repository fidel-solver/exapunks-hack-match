#ifndef BOARD_HPP
#define BOARD_HPP

#include <cstdint>

namespace HackMatch {
namespace Board {
const uint8_t EMPTY = 0;
const uint8_t YELLOW      = 0b0001;
const uint8_t GREEN       = 0b0010;
const uint8_t RED         = 0b0011;
const uint8_t PINK        = 0b0100;
const uint8_t BLUE        = 0b0101;
const uint8_t BOMB_MASK   = 0b1000;
const uint8_t YELLOW_BOMB = BOMB_MASK + YELLOW;
const uint8_t GREEN_BOMB  = BOMB_MASK + GREEN;
const uint8_t RED_BOMB    = BOMB_MASK + RED;
const uint8_t PINK_BOMB   = BOMB_MASK + PINK;
const uint8_t BLUE_BOMB   = BOMB_MASK + BLUE;

static constexpr bool isBomb(uint8_t item) {
    return item & BOMB_MASK;
}

const uint8_t MAX_COLS = 7;
const uint8_t MAX_ROWS = 9;

struct Board {
    uint8_t items[MAX_COLS][MAX_ROWS];
    uint8_t counts[MAX_COLS];
    uint8_t held;
};

struct BoardHash {
    std::size_t operator()(const Board& board) const noexcept;
};

int itemCount(const Board& board);

bool operator==(const Board& lhs, const Board& rhs);

void printBoard(const Board& board);
}}

#endif
