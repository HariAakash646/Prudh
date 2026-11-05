#pragma once

#include "board.hpp"
#include "player.hpp"

class Game
{
public:
    Game();

    int get_player_score(int player_id);

    bool make_move(Move move);
    bool are_moves_left();

    bool complete_game();

    // Read-only views for front-ends (CLI, GUI, ...)
    Turn get_turn() const { return turn; }
    std::array<std::array<int, board_size>, board_size> get_board() { return board.get_board(); }
    std::vector<Move> get_valid_moves() { return board.get_valid_moves(turn); }

private:
    Board board;
    std::array<Player, 2> players;

    Turn turn = Player1;
    bool completed = false;
};