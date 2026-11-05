#pragma once

class Player
{
public:
    Player();

    void increment_score(int value);
    int get_score();

private:
    int score = 0;
};