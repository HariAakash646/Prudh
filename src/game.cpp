#include "game.hpp"

Game::Game() {}

int Game::get_player_score(int player_id)
{
    return players[player_id].get_score();
}

bool Game::make_move(Move move)
{
    if ((move.cell.row + move.cell.col) % 2 != turn)
        return false;

    auto gained_points_opt = board.make_move(move);
    if (gained_points_opt == std::nullopt)
        return false;

    players[turn].increment_score(gained_points_opt.value());
    
    if(turn == Turn::Player1) {
        turn = Turn::Player2;
    }
    else {
        turn = Turn::Player1;
    }

    return true;
}

bool Game::are_moves_left() {
    return board.move_available(turn);
}

bool Game::complete_game() {
    if(are_moves_left()) 
        return false;
    Turn bonus_player;
    if (turn == Turn::Player1)
    {
        bonus_player = Turn::Player2;
    }
    else
    {
        bonus_player = Turn::Player1;
    }
    completed = true;

    players[bonus_player].increment_score(board.get_tallest_cell_value(bonus_player));
    
    return true;
}