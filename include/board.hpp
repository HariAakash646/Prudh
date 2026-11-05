#pragma once

#include <array>
#include <vector>
#include <optional>

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

constexpr auto Run = std::array{Direction::N, Direction::E, Direction::S, Direction::W};
constexpr auto Slide = std::array{Direction::NE, Direction::SE, Direction::SW, Direction::NW};

enum Turn
{
    Player1,
    Player2
};

struct Movement
{
    int row, col;
};

struct Cell
{
    int row, col;

    Cell &operator+=(const Movement &movement);
};

struct Move
{
    Cell cell;
    Direction dir;
};

class Board
{
public:
    Board();
    // Start from an arbitrary position. Each value is the stack height at that cell;
    // ownership is still decided by colour ((row + col) even = Player1).
    explicit Board(const std::array<std::array<int, board_size>, board_size> &grid);

    std::optional<int> make_move(Move move);
    std::array<std::array<int, board_size>, board_size> get_board();

    std::vector<Move> get_valid_moves(Turn turn);
    bool move_available(Turn turn);

    int get_tallest_cell_value(Turn turn);

private:
    std::array<std::array<int, board_size>, board_size> board;

    bool is_bounded(const Cell &cell);
    bool is_valid(const Move &move);
    std::vector<Move> get_cell_moves(Cell cell);
    bool cell_has_move(Cell cell);

    const std::array<Movement, 8> movements = {{{-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}}};
};