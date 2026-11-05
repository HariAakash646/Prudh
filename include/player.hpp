#pragma once

class Player
{
public:
    Player();

    void update_score(int value);

private:
    int score = 0;
};