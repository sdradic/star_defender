#include "Game.h"
#include "Utils.h"
#include <iostream>
#include <sys/ioctl.h>
#ifdef _WIN32
#include <conio.h> // for _kbhit and _getch on Windows
#endif
#if defined(__linux__) || defined(__APPLE__)
#include <termios.h>
#include <unistd.h>
#endif

#if defined(__linux__) || defined(__APPLE__)
bool kbhit() {
    termios term;
    tcgetattr(0,&term);

    termios term2=term;
    term2.c_lflag&=~ICANON;
    tcsetattr(0, TCSANOW, &term2);

    int bytesWaiting;
    ioctl(0, FIONREAD, &bytesWaiting);

    tcsetattr(0, TCSANOW, &term);
    return bytesWaiting > 0;
}

char getch() {
    char buf = 0;
    termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return buf;
}
#endif

Game::Game(int w, int h) 
    : width(w), height(h), running(true), tick(0), player(w/2, h-1) {
        srand(static_cast<unsigned>(time(0)));   
}

void Game::run() {
    while (running) {
        processInput();
        update();
        render();
        sleep_ms(100);
    }
}

void Game::processInput() {
    if (kbhit()) {
        char c = getch();
        if (c == 'a') player.moveLeft();
        if (c == 'd') player.moveRight(width);
        if (c == ' ') bullets.push_back(Bullet(player.x,player.y-1));
        if (c == 'q') running = false;
    }
}

void Game::update() {
    tick++;
    spawnEnemies();

    //Move bullets
    for (auto &b: bullets) b.y--;

    //Move enemies
    for (auto &e: enemies) e.y++;

    //Colission check
    for (auto &b : bullets) {
        for (auto &e: enemies) {
            if (b.x == e.x && b.y == e.y) {
                e.dead = true;
                b.dead = true;
            }
        }
    }

    //Remove dead entities
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                                 [this](Enemy &e){ 
                                     if (e.dead) return true;
                                     if (e.y >= height - 1) { running = false; return true; }
                                     return false;
                                 }),
                  enemies.end());
}

void Game::render() {
    system("clear");
    std::vector<std::string> screen(height, std::string(width, ' '));
    screen[player.y][player.x] = '^';
    for (auto &b : bullets) screen[b.y][b.x] = '|';
    for (auto &e : enemies) screen[e.y][e.x] = 'V';

    for (auto &row : screen) std::cout << row << '\n';
    std::cout << "Press [A][D] to move, [Space] to shoot, [Q] to quit.\n";
}

void Game::spawnEnemies() {
    if (tick % 10 == 0) {
        enemies.push_back(Enemy(random_int(0, width - 1), 0));
    }
}