#include "doctest.h"
#include "board.hpp"

#include <algorithm>

TEST_CASE("Board can be constructed")
{
    Board board;
    CHECK(board_size > 2);
}

TEST_CASE("Run and Slide directions don't overlap")
{
    for (auto dir : Run)
        CHECK_FALSE(std::ranges::contains(Slide, dir));
}
