#pragma once
#include "support/profile.hpp"
#include <SFML/Graphics.hpp>

class ActionHud {
    sf::Font font_;
    sf::Text text_;
    sf::RectangleShape panel_;
    std::string previous_;

public:
    explicit ActionHud(const demo::Options& options) {
        if (!font_.loadFromFile(demo::font_path(options).string())) {
            throw std::runtime_error("Cannot load HUD font");
        }
        text_.setFont(font_);
        text_.setCharacterSize(20);
        text_.setPosition(20, 20);
        panel_.setPosition(12, 12);
        panel_.setFillColor(sf::Color(25, 27, 32));
    }
    void draw(sf::RenderWindow& window, const std::string& status) {
        if (previous_ != status) {
            const auto message = demo::profile_mode ? "[ Import profile ]\n" + status : status;
            auto text = sf::String::fromUtf8(message.begin(), message.end());
            unsigned column{};
            for (std::size_t i = 0; i < text.getSize(); ++i) {
                if (text[i] == '\n') {
                    column = 0;
                } else if (++column >= 60) {
                    text.insert(i + 1, "\n");
                    column = 0;
                }
            }
            text_.setString(text);
            panel_.setSize({760, text_.getLocalBounds().height + 30});
            previous_ = status;
        }
        window.draw(panel_);
        window.draw(text_);
    }
};
