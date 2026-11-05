#include "player.hpp"

Player::Player()
{
    score = 0;
}

void Player::update_score(int value)
{
    score += value;
}