#include "doctest.h"
#include "player.hpp"

TEST_CASE("Player starts with zero score")
{
    Player p;
    CHECK(p.get_score() == 0);
}

TEST_CASE("update_score adds to the score")
{
    Player p;
    p.increment_score(3);
    CHECK(p.get_score() == 3);
    p.increment_score(4);
    CHECK(p.get_score() == 7);
    p.increment_score(0);
    CHECK(p.get_score() == 7);
}
