#pragma once

#include <array>

constexpr int board_size = 6;

class Board {
public:
    Board();
    
    bool move();
    std::array<std::array<int, board_size>, board_size> get_board();
private:

    std::array<std::array<int, board_size>, board_size> board;

};