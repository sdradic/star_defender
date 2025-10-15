#pragma once
class Player {
public:
    int x,y;
    Player(int x, int y);
    void moveLeft();
    void moveRight(int maxWidth);
};