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

private:
    Board board;
    std::array<Player, 2> players;

    Turn turn;
    bool completed;
};