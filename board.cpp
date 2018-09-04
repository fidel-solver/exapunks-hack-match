#include <iostream>

#include "board.hpp"

namespace HackMatch {
namespace Board {
namespace {
char itemToDisplay(uint8_t item) {
    switch (item) {
    case EMPTY:
        return ' ';
    case YELLOW:
        return 'y';
    case GREEN:
        return 'g';
    case RED:
        return 'r';
    case PINK:
        return 'p';
    case BLUE:
        return 'b';
    case YELLOW_BOMB:
        return 'Y';
    case GREEN_BOMB:
        return 'G';
    case RED_BOMB:
        return 'R';
    case PINK_BOMB:
        return 'P';
    case BLUE_BOMB:
        return 'B';
    default:
        std::cerr << "unhandled item: " << static_cast<int32_t>(item) << '\n';
        throw std::runtime_error("bad item");
    }
}

constexpr size_t CombineHash(size_t lhs, size_t rhs) {
    // stolen from boost::hash_combine https://www.boost.org/doc/libs/1_68_0/doc/html/hash/reference.html#boost.hash_combine
    return lhs^(rhs + 0x9e3779b9 + (lhs<<6) + (lhs>>2));
}
}

size_t BoardHash::operator()(const Board& board) const noexcept {
    size_t ret = 0;
    for (int i=0; i<MAX_COLS; ++i) {
        for (int j=0; j<board.counts[i]; ++j) {
            ret = CombineHash(ret, board.items[i][j]);
        }
    }
    ret = CombineHash(ret, board.held);
    return ret;
}

bool operator==(const Board& lhs, const Board& rhs) {
    if (lhs.held != rhs.held) return false;
    for (int i=0; i<MAX_COLS; ++i) {
        if (lhs.counts[i] != rhs.counts[i]) return false;
        for (int j=0; j<lhs.counts[i]; ++j) {
            if (lhs.items[i][j] != rhs.items[i][j]) return false;
        }
    }
    return true;
}

int itemCount(const Board& board) {
    int ret = 0;
    for (int i=0; i<MAX_COLS; ++i) {
        ret += board.counts[i];
    }
    return ret + (board.held != EMPTY);
}

void printBoard(const Board& board) {
    for (int j=0; j<MAX_ROWS; ++j) {
        for (int i=0; i<MAX_COLS; ++i) {
            if (j < board.counts[i]) {
                std::cout << itemToDisplay(board.items[i][j]);
            } else {
                std::cout << itemToDisplay(EMPTY);
            }
        }
        if (j==0) {
            std::cout << ' ' << itemToDisplay(board.held);
        }
        std::cout << '\n';
    }
}
}}
