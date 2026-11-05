#include "player.hpp"

Player::Player()
{
    score = 0;
}

void Player::increment_score(int value)
{
    score += value;
}

int Player::get_score() {
    return score;
}