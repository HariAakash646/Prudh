#pragma once

#include <array>
#include <vector>

constexpr int board_size = 6;

enum Direction
{
    N,
    NE,
    E,
    SE,
    S,
    SW,
    W,
    NW
};

enum Turn
{
    Player1,
    Player2
};

struct Cell {
    int row;
    char col;
};

struct Move {
    Cell cell;
    Direction dir;
};

class Board
{
public:
    Board();

    bool make_move(Move move);
    std::array<std::array<int, board_size>, board_size> get_board();

    std::vector<Move> get_valid_moves(Turn turn);
    bool move_available(Turn turn);

private:
    std::array<std::array<int, board_size>, board_size> board;

    std::vector<Move> get_cell_moves(Cell cell);

};