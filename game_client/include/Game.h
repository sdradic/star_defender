#pragma once
#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Renderer.h"

class Game {
private:
    int width;
    int height;
    bool running;
    int tick;
    int score;
    float difficulty;
    int enemiesSpawned;
    
    // Game state
    enum GameState {
        MENU,
        PLAYING,
        GAME_OVER,
        PAUSED
    };
    GameState currentState;
    
    // SDL3 components
    SDL_Window* window;
    Renderer* renderer;
    
    // Game entities
    Player player;
    std::vector<Enemy> enemies;
    std::vector<Bullet> bullets;
    
    // Particle system for visual effects
    struct Particle {
        float x, y;
        float vx, vy;
        int life;
        Uint8 r, g, b;
    };
    std::vector<Particle> particles;
    
    // Coordinate conversion
    static const int TILE_SIZE = 48; // Each game tile is 48x48 pixels

public:
    Game(SDL_Window* window, int width=800, int height=600);
    ~Game();
    void run();

private:
    void processInput();
    void update();
    void render();
    void spawnEnemies();
    void reset();
    
    // Game state management
    void setGameState(GameState newState);
    void renderMenu();
    void renderGameplay();
    void renderGameOver();
    void renderPaused();
    
    // Particle system
    void addParticle(float x, float y, float vx, float vy, Uint8 r, Uint8 g, Uint8 b, int life = 30);
    void updateParticles();
    void renderParticles();
    
    // Enhanced collision detection
    void removeOffScreenBullets();
    void createHitEffect(int x, int y);
    
    // Helper functions for coordinate conversion
    float gameToPixelX(int gameX) const { return gameX * TILE_SIZE; }
    float gameToPixelY(int gameY) const { return gameY * TILE_SIZE; }
    int pixelToGameX(float pixelX) const { return static_cast<int>(pixelX / TILE_SIZE); }
    int pixelToGameY(float pixelY) const { return static_cast<int>(pixelY / TILE_SIZE); }
};
