#pragma once

#include "game.hpp"

#include <iostream>
#include <optional>
#include <string>

// Command-line front-end for a two-player game on one terminal.
//
// Input format:  <cell> <direction>    e.g.  "b3 SE"  or  "a1 e"
//   cell      = column letter a-f + row number 1-6 (row 1 is the top)
//   direction = N NE E SE S SW W NW (case-insensitive)
// Commands: moves, help, quit
class CLI
{
public:
    explicit CLI(std::istream &in = std::cin, std::ostream &out = std::cout);

    // Runs the game loop until the game ends or a player quits.
    void run();

private:
    Game game;
    std::istream &in;
    std::ostream &out;

    void print_board();
    void print_scores();
    void print_help();
    void print_valid_moves();
    void print_result();

    // Returns nullopt (and prints why) if the text isn't a well-formed move.
    std::optional<Move> parse_move(const std::string &line);

    static std::string cell_name(const Cell &cell);
    static std::string direction_name(Direction dir);
    static const char *player_name(Turn turn);
};
