#pragma once
#include "support/profile.hpp"
#include <SDL.h>
#include <SDL_ttf.h>

class ActionHud {
    TTF_Font* font_{};
    SDL_Texture* texture_{};
    std::string previous_;
    int width_{}, height_{};

public:
    explicit ActionHud(const demo::Options& options) {
        if (TTF_Init() != 0) {
            throw std::runtime_error(TTF_GetError());
        }
        font_ = TTF_OpenFont(demo::font_path(options).string().c_str(), 20);
        if (!font_) {
            TTF_Quit();
            throw std::runtime_error(TTF_GetError());
        }
    }
    ActionHud(const ActionHud&) = delete;
    ActionHud& operator=(const ActionHud&) = delete;
    ~ActionHud() {
        SDL_DestroyTexture(texture_);
        TTF_CloseFont(font_);
        TTF_Quit();
    }
    void draw(SDL_Renderer* renderer, const std::string& status) {
        if (status != previous_) {
            const auto text = demo::profile_mode ? "[ Import profile ]\n" + status : status;
            int output_width{}, output_height{};
            SDL_GetRendererOutputSize(renderer, &output_width, &output_height);
            auto* surface = TTF_RenderUTF8_Blended_Wrapped(
                font_, text.c_str(), {240, 240, 240, 255}, std::max(1, output_width - 40));
            if (!surface) {
                throw std::runtime_error(TTF_GetError());
            }
            SDL_DestroyTexture(texture_);
            texture_ = SDL_CreateTextureFromSurface(renderer, surface);
            width_ = surface->w;
            height_ = surface->h;
            SDL_FreeSurface(surface);
            if (!texture_) {
                throw std::runtime_error(SDL_GetError());
            }
            previous_ = status;
        }
        SDL_RenderSetViewport(renderer, nullptr);
        SDL_SetRenderDrawColor(renderer, 25, 27, 32, 255);
        SDL_Rect background{12, 12, width_ + 16, height_ + 16};
        SDL_RenderFillRect(renderer, &background);
        SDL_Rect target{20, 20, width_, height_};
        SDL_RenderCopy(renderer, texture_, nullptr, &target);
    }
};
