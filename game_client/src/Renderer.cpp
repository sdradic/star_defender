#include "../include/Renderer.h"
#include <iostream>

Renderer::Renderer(SDL_Window* window, int width, int height) 
    : window(window), screenWidth(width), screenHeight(height) {
    
    // Create renderer for the window
    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "Error creating renderer: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Initialize TTF
    if (!TTF_Init()) {
        std::cerr << "Error initializing TTF: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Set default draw color to white
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    std::cout << "Renderer initialized successfully" << std::endl;
}

Renderer::~Renderer() {
    cleanup();
}

void Renderer::cleanup() {
    // Destroy all textures
    for (auto& pair : textures) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
        }
    }
    textures.clear();
    
    // Destroy all fonts
    for (auto& pair : fonts) {
        if (pair.second) {
            TTF_CloseFont(pair.second);
        }
    }
    fonts.clear();
    
    // Destroy renderer
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    // Quit TTF
    TTF_Quit();
}

void Renderer::clear() {
    SDL_RenderClear(renderer);
}

void Renderer::present() {
    SDL_RenderPresent(renderer);
}

void Renderer::setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void Renderer::drawRect(float x, float y, float width, float height) {
    SDL_FRect rect = {x, y, width, height};
    SDL_RenderRect(renderer, &rect);
}

void Renderer::drawFillRect(float x, float y, float width, float height) {
    SDL_FRect rect = {x, y, width, height};
    SDL_RenderFillRect(renderer, &rect);
}

SDL_Texture* Renderer::loadTexture(const std::string& path) {
    // Load image surface
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Error loading image " << path << ": " << SDL_GetError() << std::endl;
        return nullptr;
    }
    
    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Error creating texture from " << path << ": " << SDL_GetError() << std::endl;
    }
    
    // Free the surface as we no longer need it
    SDL_DestroySurface(surface);
    
    return texture;
}

bool Renderer::loadTextureFromFile(const std::string& name, const std::string& path) {
    SDL_Texture* texture = loadTexture(path);
    if (texture) {
        textures[name] = texture;
        std::cout << "Loaded texture: " << name << " from " << path << std::endl;
        return true;
    }
    return false;
}

void Renderer::drawTexture(const std::string& textureName, float x, float y, float width, float height) {
    auto it = textures.find(textureName);
    if (it == textures.end()) {
        std::cerr << "Texture not found: " << textureName << std::endl;
        return;
    }
    
    SDL_Texture* texture = it->second;
    
    // If width/height not specified, use texture's original size
    if (width < 0 || height < 0) {
        float texWidth, texHeight;
        SDL_GetTextureSize(texture, &texWidth, &texHeight);
        if (width < 0) width = texWidth;
        if (height < 0) height = texHeight;
    }
    
    SDL_FRect dstRect = {x, y, width, height};
    SDL_RenderTexture(renderer, texture, nullptr, &dstRect);
}

void Renderer::drawTexture(const std::string& textureName, float x, float y, 
                          float srcX, float srcY, float srcWidth, float srcHeight, 
                          float dstWidth, float dstHeight) {
    auto it = textures.find(textureName);
    if (it == textures.end()) {
        std::cerr << "Texture not found: " << textureName << std::endl;
        return;
    }
    
    SDL_Texture* texture = it->second;
    
    SDL_FRect srcRect = {srcX, srcY, srcWidth, srcHeight};
    SDL_FRect dstRect = {x, y, dstWidth, dstHeight};
    SDL_RenderTexture(renderer, texture, &srcRect, &dstRect);
}

bool Renderer::loadFont(const std::string& name, const std::string& path, int size) {
    TTF_Font* font = TTF_OpenFont(path.c_str(), size);
    if (!font) {
        std::cerr << "Error loading font " << path << ": " << SDL_GetError() << std::endl;
        return false;
    }
    
    fonts[name] = font;
    std::cout << "Loaded font: " << name << " from " << path << " (size " << size << ")" << std::endl;
    return true;
}

void Renderer::drawText(const std::string& fontName, const std::string& text, float x, float y, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    auto it = fonts.find(fontName);
    if (it == fonts.end()) {
        std::cerr << "Font not found: " << fontName << std::endl;
        return;
    }
    
    TTF_Font* font = it->second;
    
    // Create text surface with better rendering
    SDL_Color color = {r, g, b, a};
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), text.length(), color);
    if (!surface) {
        std::cerr << "Error rendering text: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Error creating texture from text: " << SDL_GetError() << std::endl;
        SDL_DestroySurface(surface);
        return;
    }
    
    // Render the texture
    SDL_FRect dstRect = {x, y, static_cast<float>(surface->w), static_cast<float>(surface->h)};
    SDL_RenderTexture(renderer, texture, nullptr, &dstRect);
    
    // Clean up
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

void Renderer::drawTextCentered(const std::string& fontName, const std::string& text, float y, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int textWidth = getTextWidth(fontName, text);
    float x = (screenWidth - textWidth) / 2.0f;
    drawText(fontName, text, x, y, r, g, b, a);
}

int Renderer::getTextWidth(const std::string& fontName, const std::string& text) {
    auto it = fonts.find(fontName);
    if (it == fonts.end()) {
        std::cerr << "Font not found: " << fontName << std::endl;
        return 0;
    }
    
    // Estimate width based on font size and character count
    int fontSize = 16; // Default
    if (fontName == "pixel_large") fontSize = 48;
    else if (fontName == "pixel_medium") fontSize = 24;
    else if (fontName == "pixel_small") fontSize = 16;
    
    // Rough estimation: average character width is about 40% of font size
    int estimatedWidth = static_cast<int>(text.length() * fontSize * 0.4);
    return estimatedWidth;
}
