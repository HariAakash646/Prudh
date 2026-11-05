#include "doctest.h"
#include "board.hpp"

#include <algorithm>

// Conventions (agreed rules):
//  - row 0 is the top edge; N = row - 1, E = col + 1.
//  - (row + col) even -> owned by Player1, odd -> Player2.
//  - make_move returns nullopt for an illegal move (board unchanged),
//    0 for a slide, and for a run the total pieces removed by captures
//    (enemy stack + the capturing piece, i.e. k + 1 per capture).
//  - Ordinary edge rule: leftover pieces return to the starting square.

using Grid = std::array<std::array<int, board_size>, board_size>;

namespace
{
Grid empty_grid()
{
    Grid g{};
    for (auto &row : g)
        row.fill(0);
    return g;
}

int total_pieces(const Grid &g)
{
    int sum = 0;
    for (const auto &row : g)
        for (int v : row)
            sum += v;
    return sum;
}

bool owned_by(Turn turn, const Cell &c)
{
    bool even = (c.row + c.col) % 2 == 0;
    return turn == Player1 ? even : !even;
}
} // namespace

// ---------------------------------------------------------------- setup

TEST_CASE("Default board has one piece on every square")
{
    Board b;
    Grid g = b.get_board();
    for (const auto &row : g)
        for (int v : row)
            CHECK(v == 1);
    CHECK(total_pieces(g) == 36);
}

TEST_CASE("Grid constructor reproduces the given position")
{
    Grid g = empty_grid();
    g[0][0] = 3;
    g[2][5] = 1;
    g[5][5] = 7;
    Board b(g);
    CHECK(b.get_board() == g);
}

// ---------------------------------------------------------------- slides

TEST_CASE("Slide merges into an adjacent diagonal stack")
{
    Board b;
    auto r = b.make_move({{0, 0}, SE});
    REQUIRE(r.has_value());
    CHECK(*r == 0);
    Grid g = b.get_board();
    CHECK(g[0][0] == 0);
    CHECK(g[1][1] == 2);
    CHECK(total_pieces(g) == 36);
}

TEST_CASE("Slide travels through empty squares to the first occupied one")
{
    Grid g = empty_grid();
    g[0][0] = 2;
    g[3][3] = 1;
    Board b(g);
    auto r = b.make_move({{0, 0}, SE});
    REQUIRE(r.has_value());
    CHECK(*r == 0);
    Grid after = b.get_board();
    CHECK(after[0][0] == 0);
    CHECK(after[3][3] == 3);
}

TEST_CASE("Slide works in every diagonal direction")
{
    struct Case { Cell from; Direction dir; Cell to; };
    Case cases[] = {
        {{2, 2}, NE, {1, 3}},
        {{2, 2}, SE, {3, 3}},
        {{2, 2}, SW, {3, 1}},
        {{2, 2}, NW, {1, 1}},
    };
    for (auto c : cases)
    {
        CAPTURE(c.dir);
        Board b;
        auto r = b.make_move({c.from, c.dir});
        REQUIRE(r.has_value());
        CHECK(b.get_board()[c.from.row][c.from.col] == 0);
        CHECK(b.get_board()[c.to.row][c.to.col] == 2);
    }
}

TEST_CASE("Slide that reaches the edge without hitting a stack is illegal")
{
    Grid g = empty_grid();
    g[0][0] = 1;
    g[0][1] = 1;
    Board b(g);
    CHECK_FALSE(b.make_move({{0, 0}, SE}).has_value());
    CHECK(b.get_board() == g);
}

TEST_CASE("Slide pointing straight off the board is illegal")
{
    Board b;
    CHECK_FALSE(b.make_move({{0, 0}, NE}).has_value());
    CHECK_FALSE(b.make_move({{0, 0}, NW}).has_value());
    CHECK(b.get_board() == Board().get_board());
}

TEST_CASE("Moving from an empty square is illegal")
{
    Grid g = empty_grid();
    g[1][1] = 1;
    Board b(g);
    CHECK_FALSE(b.make_move({{0, 0}, SE}).has_value());
    CHECK(b.get_board() == g);
}

TEST_CASE("Moving from a square outside the board is illegal")
{
    Board b;
    CHECK_FALSE(b.make_move({{-1, 0}, SE}).has_value());
    CHECK_FALSE(b.make_move({{0, 6}, SW}).has_value());
    CHECK_FALSE(b.make_move({{6, 6}, NW}).has_value());
    CHECK(b.get_board() == Board().get_board());
}

// ---------------------------------------------------------------- runs

TEST_CASE("Run with a single piece is illegal")
{
    Board b; // every stack has height 1
    CHECK_FALSE(b.make_move({{2, 2}, E}).has_value());
    CHECK(b.get_board() == Board().get_board());
}

TEST_CASE("Run captures a smaller enemy stack and drops a piece on each square")
{
    // row 0: [3 P1][1 P2][1 P1][0][0][0]
    Grid g = empty_grid();
    g[0][0] = 3;
    g[0][1] = 1;
    g[0][2] = 1;
    Board b(g);

    auto r = b.make_move({{0, 0}, E});
    REQUIRE(r.has_value());
    CHECK(*r == 2); // 1 enemy + 1 capturing piece

    Grid after = b.get_board();
    CHECK(after[0][0] == 0); // start square emptied
    CHECK(after[0][1] == 0); // captured, all removed
    CHECK(after[0][2] == 2); // friendly stack grows
    CHECK(after[0][3] == 1); // empty square gets a piece
    CHECK(after[0][4] == 0);
    CHECK(total_pieces(after) == total_pieces(g) - 2);
}

TEST_CASE("Run compares against the ORIGINAL stack size, not pieces left in hand")
{
    // row 0: [3 P1][2 P2][0][2 P2][0][0]
    // Third drop lands on a 2-stack with only one piece left in hand,
    // but 2 < 3 (original size) so it is still captured.
    Grid g = empty_grid();
    g[0][0] = 3;
    g[0][1] = 2;
    g[0][3] = 2;
    Board b(g);

    auto r = b.make_move({{0, 0}, E});
    REQUIRE(r.has_value());
    CHECK(*r == 6); // (2 + 1) + (2 + 1)

    Grid after = b.get_board();
    CHECK(after[0][0] == 0);
    CHECK(after[0][1] == 0);
    CHECK(after[0][2] == 1);
    CHECK(after[0][3] == 0);
}

TEST_CASE("Dropping on an equal or larger enemy stack just joins it")
{
    // row 0: [3 P1][1 P2][0][3 P2][0][0]
    Grid g = empty_grid();
    g[0][0] = 3;
    g[0][1] = 1;
    g[0][3] = 3;
    Board b(g);

    auto r = b.make_move({{0, 0}, E});
    REQUIRE(r.has_value());
    CHECK(*r == 2);

    Grid after = b.get_board();
    CHECK(after[0][1] == 0);
    CHECK(after[0][2] == 1);
    CHECK(after[0][3] == 4); // equal size: joined, not captured
}

TEST_CASE("Run without any capture is illegal")
{
    SUBCASE("only empty squares")
    {
        Grid g = empty_grid();
        g[0][0] = 2;
        Board b(g);
        CHECK_FALSE(b.make_move({{0, 0}, E}).has_value());
        CHECK(b.get_board() == g);
    }
    SUBCASE("only equal enemy stack")
    {
        Grid g = empty_grid();
        g[0][0] = 2;
        g[0][1] = 2;
        Board b(g);
        CHECK_FALSE(b.make_move({{0, 0}, E}).has_value());
        CHECK(b.get_board() == g);
    }
    SUBCASE("pointing off the board")
    {
        Grid g = empty_grid();
        g[0][0] = 2;
        g[1][0] = 1;
        Board b(g);
        CHECK_FALSE(b.make_move({{0, 0}, N}).has_value());
        CHECK(b.get_board() == g);
    }
}

TEST_CASE("Leftover pieces at the edge return to the starting square")
{
    // row 0: [..][..][..][..][3 P1][1 P2]
    Grid g = empty_grid();
    g[0][4] = 3;
    g[0][5] = 1;
    Board b(g);

    auto r = b.make_move({{0, 4}, E});
    REQUIRE(r.has_value());
    CHECK(*r == 2);

    Grid after = b.get_board();
    CHECK(after[0][5] == 0);
    CHECK(after[0][4] == 2); // two unused pieces came back
}

TEST_CASE("Runs work in every orthogonal direction")
{
    struct Case { Direction dir; Cell enemy; };
    // P1 stack of 2 at (2,2); enemy single pieces at odd squares around it.
    Case cases[] = {
        {N, {1, 2}},
        {E, {2, 3}},
        {S, {3, 2}},
        {W, {2, 1}},
    };
    for (auto c : cases)
    {
        CAPTURE(c.dir);
        Grid g = empty_grid();
        g[2][2] = 2;
        g[c.enemy.row][c.enemy.col] = 1;
        Board b(g);
        auto r = b.make_move({{2, 2}, c.dir});
        REQUIRE(r.has_value());
        CHECK(*r == 2);
        Grid after = b.get_board();
        CHECK(after[c.enemy.row][c.enemy.col] == 0);
        CHECK(after[2][2] == 0);
        CHECK(total_pieces(after) == 1); // second piece landed one square further
    }
}

TEST_CASE("Player2 can run too")
{
    // row 0: [1 P1][2 P2] ; P2 runs west into the edge
    Grid g = empty_grid();
    g[0][0] = 1;
    g[0][1] = 2;
    Board b(g);
    auto r = b.make_move({{0, 1}, W});
    REQUIRE(r.has_value());
    CHECK(*r == 2);
    Grid after = b.get_board();
    CHECK(after[0][0] == 0);
    CHECK(after[0][1] == 1); // one leftover returned
}

// ---------------------------------------------------------------- move generation

TEST_CASE("Initial position: only slides, all on the player's own squares")
{
    for (Turn t : {Player1, Player2})
    {
        CAPTURE(t);
        Board b;
        auto moves = b.get_valid_moves(t);

        int expected = 0;
        for (int r = 0; r < board_size; ++r)
            for (int c = 0; c < board_size; ++c)
                if (owned_by(t, {r, c}))
                    for (int dr : {-1, 1})
                        for (int dc : {-1, 1})
                            if (r + dr >= 0 && r + dr < board_size && c + dc >= 0 && c + dc < board_size)
                                ++expected;
        CHECK(moves.size() == expected);

        for (const auto &m : moves)
        {
            CHECK(owned_by(t, m.cell));
            CHECK(std::ranges::find(Slide, m.dir) != Slide.end());
        }
        CHECK(b.move_available(t));
    }
}

TEST_CASE("Every move returned by get_valid_moves is accepted by make_move")
{
    Grid g = empty_grid();
    g[0][0] = 3;
    g[0][1] = 1;
    g[1][1] = 1;
    g[2][2] = 2;
    g[2][3] = 1;
    g[4][4] = 1;
    g[5][0] = 2;
    for (Turn t : {Player1, Player2})
    {
        Board probe(g);
        for (const auto &m : probe.get_valid_moves(t))
        {
            Board b(g);
            CHECK(b.make_move(m).has_value());
        }
    }
}

TEST_CASE("get_valid_moves includes runs that capture")
{
    Grid g = empty_grid();
    g[0][0] = 2;
    g[0][1] = 1;
    Board b(g);
    auto moves = b.get_valid_moves(Player1);
    bool found = std::ranges::any_of(moves, [](const Move &m)
                                     { return m.cell.row == 0 && m.cell.col == 0 && m.dir == E; });
    CHECK(found);
}

TEST_CASE("No moves when nothing can merge or capture")
{
    Grid g = empty_grid();
    g[0][0] = 1; // P1
    g[0][1] = 1; // P2
    Board b(g);
    CHECK(b.get_valid_moves(Player1).empty());
    CHECK(b.get_valid_moves(Player2).empty());
    CHECK_FALSE(b.move_available(Player1));
    CHECK_FALSE(b.move_available(Player2));
}

TEST_CASE("Moves can be available for one player only")
{
    Grid g = empty_grid();
    g[0][0] = 2; // P1 can run east and capture
    g[0][1] = 1; // P2 single piece, no diagonal partner
    Board b(g);
    CHECK(b.move_available(Player1));
    CHECK_FALSE(b.move_available(Player2));
}

// ---------------------------------------------------------------- Cell

TEST_CASE("Cell += Movement shifts the cell")
{
    Cell c{2, 3};
    c += Movement{-1, 1};
    CHECK(c.row == 1);
    CHECK(c.col == 4);
    (c += Movement{1, 0}) += Movement{0, -2};
    CHECK(c.row == 2);
    CHECK(c.col == 2);
}

// ---------------------------------------------------------------- tallest stack

TEST_CASE("get_tallest_cell_value: initial board is 1 for both players")
{
    Board b;
    CHECK(b.get_tallest_cell_value(Player1) == 1);
    CHECK(b.get_tallest_cell_value(Player2) == 1);
}

TEST_CASE("get_tallest_cell_value only looks at the player's own squares")
{
    Grid g = empty_grid();
    g[0][0] = 3; // P1
    g[4][2] = 5; // P1 (tallest for P1)
    g[0][1] = 9; // P2 (taller, but not P1's)
    g[3][2] = 2; // P2
    Board b(g);
    CHECK(b.get_tallest_cell_value(Player1) == 5);
    CHECK(b.get_tallest_cell_value(Player2) == 9);
}

TEST_CASE("get_tallest_cell_value is 0 when the player has no pieces")
{
    Grid g = empty_grid();
    g[0][1] = 4; // only P2 has pieces
    Board b(g);
    CHECK(b.get_tallest_cell_value(Player1) == 0);
    CHECK(b.get_tallest_cell_value(Player2) == 4);
}

TEST_CASE("get_tallest_cell_value updates after a move")
{
    Board b;
    REQUIRE(b.make_move({{0, 0}, SE}).has_value()); // (1,1) becomes 2
    CHECK(b.get_tallest_cell_value(Player1) == 2);
    CHECK(b.get_tallest_cell_value(Player2) == 1);
}
