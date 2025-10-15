#include "../include/Player.h"
Player::Player(int x, int y) : x(x), y(y) {}
void Player::moveLeft() {if (x > 0) { x--; }}
void Player::moveRight(int maxWidth) {if (x < maxWidth - 1) x++;}