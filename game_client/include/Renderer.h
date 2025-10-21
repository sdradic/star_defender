#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <unordered_map>

class Renderer {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    int screenWidth;
    int screenHeight;
    
    // Texture cache for loaded images
    std::unordered_map<std::string, SDL_Texture*> textures;
    
    // Font cache
    std::unordered_map<std::string, TTF_Font*> fonts;
    
    // Helper function to load a texture
    SDL_Texture* loadTexture(const std::string& path);

public:
    Renderer(SDL_Window* window, int width, int height);
    ~Renderer();
    
    // Basic rendering functions
    void clear();
    void present();
    void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    
    // Drawing functions
    void drawRect(float x, float y, float width, float height);
    void drawFillRect(float x, float y, float width, float height);
    
    // Texture management
    bool loadTextureFromFile(const std::string& name, const std::string& path);
    void drawTexture(const std::string& textureName, float x, float y, float width = -1, float height = -1);
    void drawTexture(const std::string& textureName, float x, float y, float srcX, float srcY, float srcWidth, float srcHeight, float dstWidth, float dstHeight);
    
    // Font management
    bool loadFont(const std::string& name, const std::string& path, int size);
    void drawText(const std::string& fontName, const std::string& text, float x, float y, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255);
    void drawTextCentered(const std::string& fontName, const std::string& text, float y, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255);
    int getTextWidth(const std::string& fontName, const std::string& text);
    
    // Utility functions
    int getWidth() const { return screenWidth; }
    int getHeight() const { return screenHeight; }
    SDL_Renderer* getSDLRenderer() const { return renderer; }
    
    // Cleanup
    void cleanup();
};
