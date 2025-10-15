#pragma once
#include <vector>
#include <string>
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"

class Game {
private:
    int width;
    int height;
    bool running;
    int tick;

    Player player;
    std::vector<Enemy> enemies;
    std::vector<Bullet> bullets;

public:
    Game(int width=20, int height=10);
    void run();

private:
    void processInput();
    void update();
    void render();
    void spawnEnemies();
    void reset();
};
