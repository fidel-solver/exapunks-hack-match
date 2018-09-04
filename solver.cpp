#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <unordered_set>

#include "board.hpp"
#include "common.hpp"
#include "solver.hpp"

namespace HackMatch {
namespace Solver {
namespace {

using CacheType = std::unordered_set<Board::Board, Board::BoardHash>;

template <typename T>
bool hasMatchImpl(const Board::Board& board, uint8_t i, uint8_t j, uint8_t item, T& visited, uint8_t& matchesRemaining) {
    if (i>=Board::MAX_COLS) return false;
    if (j>=board.counts[i]) return false;
    if (visited[i][j]) return false;
    if (board.items[i][j] != item) return false;
    visited[i][j] = true;
    --matchesRemaining;
    if (matchesRemaining==0) return true;
    if (hasMatchImpl(board, i+1, j, item, visited, matchesRemaining)) return true;
    if (hasMatchImpl(board, i-1, j, item, visited, matchesRemaining)) return true;
    if (hasMatchImpl(board, i, j+1, item, visited, matchesRemaining)) return true;
    if (hasMatchImpl(board, i, j-1, item, visited, matchesRemaining)) return true;
    return false;
}

bool hasMatch(const Board::Board& board, uint8_t i, uint8_t j) {
    bool visited[Board::MAX_COLS][Board::MAX_ROWS]{};
    uint8_t matchesRemaining = Board::isBomb(board.items[i][j]) ? 2 : 4;
    return hasMatchImpl(board, i, j, board.items[i][j], visited, matchesRemaining);
}

bool solveImpl(const Board::Board& board, std::vector<Move>& moves, const uint8_t maxMoves, CacheType& cache) {
    if (moves.size() == maxMoves) return false;
    const auto cacheRet = cache.insert(board);
    if (!cacheRet.second) return false;
    uint8_t cols[Board::MAX_COLS] = {0, 1, 2, 3, 4, 5, 6};
    if (board.held) {
        std::sort(cols, cols+Board::MAX_COLS, [&board](uint8_t l, uint8_t r){
                return board.counts[l] < board.counts[r];});
        for (uint8_t colIndex=0; colIndex<Board::MAX_COLS; ++colIndex) {
            const uint8_t i = cols[colIndex];
            if (board.counts[i] < Board::MAX_ROWS) {
                Board::Board curBoard{board};
                moves.push_back({PUT, i});
                makeMove(curBoard, moves.back());
                if (hasMatch(curBoard, i, curBoard.counts[i]-1)) {
                    return true;
                }
                if (solveImpl(curBoard, moves, maxMoves, cache)) return true;
                moves.pop_back();
            }
        }
    } else {
        std::sort(cols, cols+Board::MAX_COLS, [&board](uint8_t l, uint8_t r){
                return board.counts[l] > board.counts[r];});
        for (uint8_t colIndex=0; colIndex<Board::MAX_COLS; ++colIndex) {
            const uint8_t i = cols[colIndex];
            if (board.counts[i] > 0) {
                Board::Board curBoard{board};
                moves.push_back({TAKE, i});
                makeMove(curBoard, moves.back());
                if (solveImpl(curBoard, moves, maxMoves, cache)) return true;
                moves.pop_back();
            }
        }
    }
    for (uint8_t i=0; i<Board::MAX_COLS; ++i) {
        if (board.counts[i] > 1) {
            Board::Board curBoard{board};
            moves.push_back({SWAP, i});
            makeMove(curBoard, moves.back());
            if (hasMatch(curBoard, i, curBoard.counts[i]-1)) {
                return true;
            }
            if (hasMatch(curBoard, i, curBoard.counts[i]-2)) {
                return true;
            }
            if (solveImpl(curBoard, moves, maxMoves, cache)) return true;
            moves.pop_back();
        }
    }
    return false;
}

void balanceBoard(const Board::Board& board, std::vector<Move>& moves) {
    Timer timer{"balanceBoard time"};
    Board::Board curBoard{board};
    for (int moveCount=0; moveCount<4; ++moveCount) {
        uint8_t cols[Board::MAX_COLS] = {0, 1, 2, 3, 4, 5, 6};
        std::sort(cols, cols+Board::MAX_COLS, [&curBoard](uint8_t l, uint8_t r){
                return curBoard.counts[l] < curBoard.counts[r];});
        if (curBoard.counts[cols[0]]+1 >= curBoard.counts[cols[Board::MAX_COLS-1]]) return;
        if (curBoard.held) {
            moves.push_back({PUT, cols[0]});
        } else {
            moves.push_back({TAKE, cols[Board::MAX_COLS-1]});
        }
        makeMove(curBoard, moves.back());
    }
}
}

void printMoves(const std::vector<Move>& moves) {
    for (const auto& move : moves) {
        switch (move.command) {
        case TAKE:
            std::cout << 't';
            break;
        case PUT:
            std::cout << 'p';
            break;
        case SWAP:
            std::cout << 's';
            break;
        default:
            std::cerr << "bad command: " << static_cast<int>(move.command) << '\n';
            throw std::runtime_error("bad command");
        }
        std::cout << static_cast<int>(move.col) << ' ';
    }
    std::cout << '\n';
}

void makeMove(Board::Board& board, Move move) {
    const uint8_t col = move.col;
    switch (move.command) {
    case TAKE:
        assert(board.counts[col]);
        assert(board.held == Board::EMPTY);
        --board.counts[col];
        board.held = board.items[col][board.counts[col]];
        return;
    case PUT:
        assert(board.held != Board::EMPTY);
        assert(board.counts[col] < Board::MAX_ROWS);
        board.items[col][board.counts[col]] = board.held;
        ++board.counts[col];
        board.held = Board::EMPTY;
        return;
    case SWAP:
        assert(board.counts[col] > 1);
        std::swap(board.items[col][board.counts[col]-1], board.items[col][board.counts[col]-2]);
        return;
    }
    std::cerr << "makeMove unhandled command: " << static_cast<int>(move.command) << '\n';
    throw std::runtime_error("makeMove");
}

void solve(const Board::Board& board, std::vector<Move>& moves) {
    Timer timer{"solve time"};
    moves.clear();
    const int maxMaxMoves = itemCount(board) < 12 ? 7 : 10;
    for (int maxMoves=1; maxMoves<maxMaxMoves; ++maxMoves) {
        CacheType cache;
        cache.reserve(100000);
        assert(moves.size() == 0);
        if (solveImpl(board, moves, maxMoves, cache)) {
            return;
        }
    }
    if (moves.size() == 0) {
        balanceBoard(board, moves);
        if (moves.size()) {
            return;
        }
    }
}
}}
