#pragma once

class Player
{
public:
    Player();

    void update_score(int value);
    int get_score();

private:
    int score = 0;
};