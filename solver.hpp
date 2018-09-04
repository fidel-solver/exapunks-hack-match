#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <cstdint>
#include <vector>

#include "board.hpp"

namespace HackMatch {
namespace Solver {

const uint8_t TAKE = 0;
const uint8_t PUT = 1;
const uint8_t SWAP = 2;

struct Move {
    uint8_t command;
    uint8_t col;
};

void printMoves(const std::vector<Move>& moves);
void makeMove(Board::Board& board, Move move);
void solve(const Board::Board& board, std::vector<Move>& moves);

}}


#endif
