#include "doctest.h"
#include "game.hpp"

#include <algorithm>

// Conventions (agreed rules):
//  - Player1 (id 0) moves first and owns (row + col) even squares; Player2 is id 1.
//  - Game::make_move returns false for illegal moves or moves from a square the
//    current player doesn't own; the turn does not change in that case.
//  - complete_game() returns true once the game is over and awards the last
//    mover a bonus equal to their tallest remaining stack.

TEST_CASE("New game: zero scores, moves available")
{
    Game g;
    CHECK(g.get_player_score(0) == 0);
    CHECK(g.get_player_score(1) == 0);
    CHECK(g.are_moves_left());
    CHECK_FALSE(g.complete_game());
}

TEST_CASE("Player1 moves first and turns alternate")
{
    Game g;
    CHECK_FALSE(g.make_move({{0, 1}, SE})); // Player2's square, not their turn
    CHECK(g.make_move({{0, 0}, SE}));       // Player1
    CHECK_FALSE(g.make_move({{2, 2}, SE})); // Player1 again: not allowed
    CHECK(g.make_move({{0, 1}, SE}));       // Player2
    CHECK(g.make_move({{2, 2}, SE}));       // back to Player1
}

TEST_CASE("Illegal move is rejected and keeps the turn")
{
    Game g;
    CHECK_FALSE(g.make_move({{0, 0}, NW})); // off the board
    CHECK_FALSE(g.make_move({{0, 0}, E}));  // run with a single piece
    CHECK(g.make_move({{0, 0}, SE}));       // still Player1's turn
}

TEST_CASE("Captures are added to the moving player's score")
{
    Game g;
    REQUIRE(g.make_move({{0, 0}, SE})); // P1: (1,1) becomes 2
    REQUIRE(g.make_move({{0, 1}, SE})); // P2: (1,2) becomes 2
    // P1 runs west from (1,1): captures the 1-stack at (1,0), extra returns.
    REQUIRE(g.make_move({{1, 1}, W}));
    CHECK(g.get_player_score(0) == 2);
    CHECK(g.get_player_score(1) == 0);
}

TEST_CASE("Full game played to the end awards the last mover's bonus")
{
    // A shadow Board tracks the position so we know legal moves and the
    // expected scores without peeking at Game's internals.
    Game g;
    Board shadow;
    int expected[2] = {0, 0};
    Turn turn = Player1;
    Turn last_mover = Player1;

    int guard = 0;
    while (shadow.move_available(turn))
    {
        REQUIRE(g.are_moves_left());
        REQUIRE(++guard < 1000);

        auto moves = shadow.get_valid_moves(turn);
        // Prefer captures so the game ends with real scores.
        auto pick = moves.front();
        for (const auto &m : moves)
            if (std::ranges::find(Run, m.dir) != Run.end())
            {
                pick = m;
                break;
            }

        auto captured = shadow.make_move(pick);
        REQUIRE(captured.has_value());
        REQUIRE(g.make_move(pick));
        expected[turn] += *captured;

        last_mover = turn;
        turn = turn == Player1 ? Player2 : Player1;
    }

    CHECK_FALSE(g.are_moves_left());
    CHECK(g.get_player_score(0) == expected[0]);
    CHECK(g.get_player_score(1) == expected[1]);

    int tallest = 0;
    auto grid = shadow.get_board();
    for (int r = 0; r < board_size; ++r)
        for (int c = 0; c < board_size; ++c)
        {
            bool even = (r + c) % 2 == 0;
            if ((last_mover == Player1) == even)
                tallest = std::max(tallest, grid[r][c]);
        }
    expected[last_mover] += tallest;

    CHECK(g.complete_game());
    CHECK(g.get_player_score(0) == expected[0]);
    CHECK(g.get_player_score(1) == expected[1]);
}
