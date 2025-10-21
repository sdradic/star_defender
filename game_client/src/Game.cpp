#include "../include/Game.h"
#include "../include/Utils.h"
#include <iostream>
#include <algorithm>

Game::Game(SDL_Window* window, int w, int h) 
    : window(window), width(w), height(h), running(true), tick(0), score(0), 
      player(w/2/TILE_SIZE, h/TILE_SIZE-1), currentState(MENU) {
    
    // Create renderer
    renderer = new Renderer(window, width, height);
    
    // Load sprite textures
    renderer->loadTextureFromFile("player", "../assets/player_128.png");
    renderer->loadTextureFromFile("enemy", "../assets/enemy_128.png");
    renderer->loadTextureFromFile("bullet", "../assets/bullet_128.png");
    renderer->loadTextureFromFile("background", "../assets/background.png");
    
    // Load fonts
    renderer->loadFont("pixel_large", "../assets/Pixel Game Extrude.otf", 48);
    renderer->loadFont("pixel_medium", "../assets/Pixel Game.otf", 24);
    renderer->loadFont("pixel_small", "../assets/Pixel Game.otf", 16);
    
    // Initialize random seed
    srand(static_cast<unsigned>(time(0)));
    
    std::cout << "Game initialized with " << width/TILE_SIZE << "x" << height/TILE_SIZE << " game grid" << std::endl;
}

Game::~Game() {
    delete renderer;
}

void Game::run() {
    Uint64 lastTime = SDL_GetTicks();
    const Uint64 targetFrameTime = 1000 / 12; // 12 FPS for retro feel
    
    while (running) {
        Uint64 currentTime = SDL_GetTicks();
        Uint64 deltaTime = currentTime - lastTime;
        
        if (deltaTime >= targetFrameTime) {
            processInput();
            update();
            render();
            lastTime = currentTime;
        }
        
        // Small delay to prevent excessive CPU usage
        SDL_Delay(1);
    }
}

void Game::processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
                
            case SDL_EVENT_KEY_DOWN:
                switch (currentState) {
                    case MENU:
                        if (event.key.key == SDLK_SPACE || event.key.key == SDLK_RETURN) {
                            setGameState(PLAYING);
                            reset();
                        }
                        break;
                        
                    case PLAYING:
                        switch (event.key.key) {
                            case SDLK_ESCAPE:
                                setGameState(PAUSED);
                                break;
                            case SDLK_SPACE:
                                // Shoot bullet
                                bullets.push_back(Bullet(player.x, player.y - 1));
                                break;
                        }
                        
                        // Use scancodes for movement
                        switch (event.key.scancode) {
                            case SDL_SCANCODE_A:
                                player.moveLeft();
                                break;
                            case SDL_SCANCODE_D:
                                player.moveRight(width / TILE_SIZE);
                                break;
                            default:
                                // Do nothing for other scancodes
                                break;
                        }
                        break;
                        
                    case PAUSED:
                        if (event.key.key == SDLK_ESCAPE) {
                            setGameState(PLAYING);
                        }
                        break;
                        
                    case GAME_OVER:
                        if (event.key.key == SDLK_SPACE || event.key.key == SDLK_RETURN) {
                            setGameState(MENU);
                        }
                        break;
                }
                break;
        }
    }
}

void Game::reset() {
    // Clear all enemies, bullets, and particles
    enemies.clear();
    bullets.clear();
    particles.clear();
    
    // Reset player to initial position (center bottom)
    player.x = (width / TILE_SIZE) / 2;
    player.y = (height / TILE_SIZE) - 1;
    
    // Reset game state
    tick = 0;
    score = 0;
    difficulty = 1.0;
    enemiesSpawned = 0;
}

void Game::update() {
    if (currentState != PLAYING) return;
    
    tick++;
    spawnEnemies();

    // Move bullets upward
    for (auto &b: bullets) b.y--;

    // Move enemies downward (slower - every 2 ticks instead of every tick)
    if (tick % 2 == 0) {
        for (auto &e: enemies) e.y++;
    }

    // Enhanced collision detection
    for (auto &b : bullets) {
        for (auto &e: enemies) {
            if (b.x == e.x && b.y == e.y) {
                e.dead = true;
                b.dead = true;
                score += 10;
                // Create hit effect particles
                createHitEffect(e.x, e.y);
            }
        }
    }

    // Remove bullets that are off-screen
    removeOffScreenBullets();

    // Remove dead entities
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                                [this](Enemy &e){ 
                                    if (e.dead) return true;
                                    if (e.y >= height / TILE_SIZE - 1) { 
                                        setGameState(GAME_OVER);
                                        return true; 
                                    }
                                    return false;
                                }),
                enemies.end());
    
    // Remove dead bullets
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
                                [this](Bullet &b){ 
                                    return b.dead; 
                                }),
                bullets.end());
    
    // Update particles
    updateParticles();
}

void Game::render() {
    // Clear screen with dark background
    renderer->clear();
    renderer->setDrawColor(0, 0, 50); // Dark blue background
    
    // Render based on current game state
    switch (currentState) {
        case MENU:
            renderMenu();
            break;
        case PLAYING:
            renderGameplay();
            break;
        case PAUSED:
            renderGameplay();
            renderPaused();
            break;
        case GAME_OVER:
            renderGameplay();
            renderGameOver();
            break;
    }
    
    // Present the frame
    renderer->present();
}

void Game::renderGameplay() {
    // Add downward-scrolling background
    static float backgroundOffset = 0;
    backgroundOffset += 0.5f; // Scroll speed in pixels per frame
    if (backgroundOffset >= height) backgroundOffset -= height;
    renderer->drawTexture("background", 0, backgroundOffset - height, width, height);
    renderer->drawTexture("background", 0, backgroundOffset, width, height);
    
    // Draw title area with enhanced score display
    renderer->setDrawColor(100, 100, 255); // Light blue for title
    renderer->drawFillRect(0, 0, width, 60);
    
    // Draw score bar with gradient effect
    int maxScoreBarWidth = width - 40;
    int scoreBarWidth = std::min(score * 2, maxScoreBarWidth);
    
    // Background for score bar
    renderer->setDrawColor(50, 50, 100);
    renderer->drawFillRect(20, 20, maxScoreBarWidth, 20);
    
    // Score bar with color gradient based on score
    if (scoreBarWidth > 0) {
        Uint8 r = std::min(255, score / 2);
        Uint8 g = std::max(0, 255 - score / 4);
        Uint8 b = 255;
        renderer->setDrawColor(r, g, b);
        renderer->drawFillRect(20, 20, scoreBarWidth, 20);
    }
    
    // Draw score text
    std::string scoreText = "Score: " + std::to_string(score);
    renderer->drawText("pixel_small", scoreText, 20, 45, 255, 255, 255);
    
    // Draw game area border
    renderer->setDrawColor(255, 255, 255); // White border
    renderer->drawRect(0, 60, width, height - 60);
    
    // Draw player
    renderer->setDrawColor(0, 255, 0); // Green player
    float playerPixelX = gameToPixelX(player.x);
    float playerPixelY = gameToPixelY(player.y); // Offset for title area

    // Prevent drawing player outside the bottom of the window
    if (playerPixelY + TILE_SIZE <= height) {
        renderer->drawTexture("player", playerPixelX, playerPixelY, TILE_SIZE, TILE_SIZE);
    }
    
    // Draw bullets
    for (auto &b : bullets) {
        if (b.y >= 0 && b.y < height / TILE_SIZE) {
            float bulletPixelX = gameToPixelX(b.x);
            float bulletPixelY = gameToPixelY(b.y);
            renderer->drawTexture("bullet", bulletPixelX + TILE_SIZE/4, bulletPixelY + TILE_SIZE/4, TILE_SIZE/4, TILE_SIZE/2);
        }
    }
    
    // Draw enemies
    renderer->setDrawColor(255, 0, 0); // Red enemies
    for (auto &e : enemies) {
        if (e.y >= 0 && e.y < height / TILE_SIZE) {
            float enemyPixelX = gameToPixelX(e.x);
            float enemyPixelY = gameToPixelY(e.y);
            renderer->drawTexture("enemy", enemyPixelX, enemyPixelY, TILE_SIZE, TILE_SIZE);
        }
    }
    
    // Render particles
    renderParticles();
}

void Game::spawnEnemies() {
    // Increase difficulty every 10 enemies
    difficulty = 1.0f + (enemiesSpawned / 10) * 0.1f;

    // Decrease spawn interval as difficulty increases
    int spawnInterval = std::max(10, 30 - (int)(difficulty * 5));

    if (tick % 30 == 0) { // Spawn every 30 ticks (about every 0.5 seconds at 60 FPS)
        int gameWidth = width / TILE_SIZE;
        enemies.push_back(Enemy(random_int(0, gameWidth - 1), 0));
        enemiesSpawned++;
    }
}

// Game state management
void Game::setGameState(GameState newState) {
    currentState = newState;
}

void Game::renderMenu() {
    // Draw title area
    renderer->setDrawColor(100, 100, 255);
    renderer->drawFillRect(0, 0, width, height);
    
    // Draw title text - centered
    renderer->drawTextCentered("pixel_large", "STAR DEFENDER", height/2 - 80, 255, 255, 255);
    
    // Draw instructions - centered
    renderer->drawTextCentered("pixel_medium", "Press SPACE or ENTER to Start", height/2 + 20, 200, 200, 200);
    renderer->drawTextCentered("pixel_small", "Use A/D to move, SPACE to shoot", height/2 + 60, 180, 180, 180);
    renderer->drawTextCentered("pixel_small", "Press ESC to pause during game", height/2 + 90, 180, 180, 180);
}

void Game::renderPaused() {
    // Draw semi-transparent overlay
    renderer->setDrawColor(0, 0, 0, 128);
    renderer->drawFillRect(0, 0, width, height);
    
    // Draw pause text - centered
    renderer->drawTextCentered("pixel_large", "PAUSED", height/2 - 30, 255, 255, 255);
    renderer->drawTextCentered("pixel_medium", "Press ESC to Resume", height/2 + 20, 200, 200, 200);
}

void Game::renderGameOver() {
    // Draw semi-transparent overlay
    renderer->setDrawColor(0, 0, 0, 128);
    renderer->drawFillRect(0, 0, width, height);
    
    // Draw game over text - centered
    renderer->drawTextCentered("pixel_large", "GAME OVER", height/2 - 50, 255, 0, 0);
    
    // Draw final score - centered
    std::string scoreText = "Final Score: " + std::to_string(score);
    renderer->drawTextCentered("pixel_medium", scoreText, height/2 + 10, 255, 255, 255);
    
    // Draw restart instruction - centered
    renderer->drawTextCentered("pixel_medium", "Press SPACE to Return to Menu", height/2 + 50, 200, 200, 200);
}

// Particle system
void Game::addParticle(float x, float y, float vx, float vy, Uint8 r, Uint8 g, Uint8 b, int life) {
    Particle p;
    p.x = x;
    p.y = y;
    p.vx = vx;
    p.vy = vy;
    p.r = r;
    p.g = g;
    p.b = b;
    p.life = life;
    particles.push_back(p);
}

void Game::updateParticles() {
    for (auto it = particles.begin(); it != particles.end();) {
        it->x += it->vx;
        it->y += it->vy;
        it->life--;
        
        if (it->life <= 0) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::renderParticles() {
    for (const auto& p : particles) {
        renderer->setDrawColor(p.r, p.g, p.b);
        renderer->drawFillRect(p.x - 2, p.y - 2, 4, 4);
    }
}

// Enhanced collision detection
void Game::removeOffScreenBullets() {
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
                                [this](Bullet &b){ 
                                    return b.y < 0; // Remove bullets that have gone off the top
                                }),
                bullets.end());
}

void Game::createHitEffect(int x, int y) {
    float pixelX = gameToPixelX(x) + TILE_SIZE/2;
    float pixelY = gameToPixelY(y) + TILE_SIZE/2;
    
    // Create explosion particles
    for (int i = 0; i < 8; i++) {
        float angle = (i * 45.0f) * 3.14159f / 180.0f;
        float speed = 2.0f + (rand() % 3);
        float vx = cos(angle) * speed;
        float vy = sin(angle) * speed;
        
        Uint8 r = 255;
        Uint8 g = 100 + (rand() % 155);
        Uint8 b = 0;
        
        addParticle(pixelX, pixelY, vx, vy, r, g, b, 20 + (rand() % 20));
    }
}