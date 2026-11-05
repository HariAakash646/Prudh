#include "board.hpp"
#include <algorithm>

Cell &Cell::operator+=(const Movement &movement)
{
    row += movement.row;
    col += movement.col;
    return *this; // Return a reference to the updated cell
}

Cell operator+(Cell cell, const Movement &movement)
{
    cell += movement;
    return cell;
}

Board::Board()
{
    for (auto &row : board)
    {
        std::fill(row.begin(), row.end(), 1);
    }
}

Board::Board(const std::array<std::array<int, board_size>, board_size> &grid) : board(grid) {}

std::array<std::array<int, board_size>, board_size> Board::get_board()
{
    return board;
}

bool Board::is_bounded(const Cell &cell)
{
    return cell.row >= 0 && cell.row < board_size && cell.col >= 0 && cell.col < board_size;
}

bool Board::is_valid(const Move &move)
{
    Cell cell = move.cell;
    if (!is_bounded(cell))
        return false;
    if (board[cell.row][cell.col] == 0)
        return false;

    int piece_count = board[cell.row][cell.col];
    Movement movement = movements[move.dir];
    if (std::ranges::contains(Slide, move.dir))
    {
        while (true)
        {
            cell += movement;
            if (!is_bounded(cell))
                break;
            if (board[cell.row][cell.col] > 0)
                return true;
        }
    }
    else
    {
        while (piece_count > 0)
        {
            cell += movement;
            --piece_count;
            if (!is_bounded(cell))
                break;
            if (board[cell.row][cell.col] > 0 && board[cell.row][cell.col] < board[move.cell.row][move.cell.col])
                return true;
            cell += movement;
            --piece_count;
        }
    }

    return false;
}

std::vector<Move> Board::get_cell_moves(Cell cell)
{
    if (!is_bounded(cell) || board[cell.row][cell.col] == 0)
        return {};

    std::vector<Move> moves;
    for (auto dir : Slide)
    {
        if (is_valid({cell, dir}))
            moves.push_back({cell, dir});
    }

    for (auto dir : Run)
    {
        if (is_valid({cell, dir}))
            moves.push_back({cell, dir});
    }

    return moves;
}

bool Board::cell_has_move(Cell cell)
{
    if (!is_bounded(cell) || board[cell.row][cell.col] == 0)
        return false;

    for (auto dir : Slide)
    {
        if (is_valid({cell, dir}))
            return true;
    }

    for (auto dir : Run)
    {
        if (is_valid({cell, dir}))
            return true;
    }

    return false;
}

std::vector<Move> Board::get_valid_moves(Turn turn)
{
    std::vector<Move> moves;
    for (int row = 0; row < board_size; row++)
    {
        for (int col = (turn + row) % 2; col < board_size; col += 2)
        {
            moves.append_range(get_cell_moves({row, col}));
        }
    }
    return moves;
}

bool Board::move_available(Turn turn)
{
    std::vector<Move> moves;
    for (int row = 0; row < board_size; row++)
    {
        for (int col = (turn + row) % 2; col < board_size; col += 2)
        {
            if (cell_has_move({row, col}))
                return true;
        }
    }
    return false;
}

std::optional<int> Board::make_move(Move move)
{
    if (!is_valid(move))
        return std::nullopt;

    Cell cell = move.cell;
    int piece_count = board[cell.row][cell.col];
    board[cell.row][cell.col] = 0;

    Movement movement = movements[move.dir];
    if (std::ranges::contains(Slide, move.dir))
    {
        while (true)
        {
            cell += movement;

            if (board[cell.row][cell.col] > 0)
            {
                board[cell.row][cell.col] += piece_count;
                break;
            }
        }
        return 0;
    }
    else
    {
        int captured = 0;
        while (true)
        {
            cell += movement;
            if (!is_bounded(cell))
            {
                board[move.cell.row][move.cell.col] += piece_count;
                break;
            }
            --piece_count;
            if (board[cell.row][cell.col] > 0 && board[cell.row][cell.col] < board[move.cell.row][move.cell.col])
            {
                captured += 1 + board[cell.row][cell.col];
                board[cell.row][cell.col] = 0;
            }
            else
            {
                ++board[cell.row][cell.col];
            }
            cell += movement;
            if (!is_bounded(cell))
            {
                board[move.cell.row][move.cell.col] += piece_count;
                break;
            }
            else
            {
                --piece_count;
                ++board[cell.row][cell.col];
            }
        }
        return captured;
    }
}