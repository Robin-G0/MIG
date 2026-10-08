#include "example_usage.hpp"
#include "hud.hpp"
#include "support/drawing.hpp"
#include "support/profile.hpp"
#include "support/source.hpp"
#include <SFML/Graphics.hpp>

class CameraTexture {
    sf::Texture texture_;
    std::vector<std::uint8_t> rgba_;

public:
    void draw(sf::RenderWindow& window, const mig::native::VideoFrame& frame, float x, float y,
              float w, float h) {
        if (frame.rgb.empty()) {
            return;
        }
        if (texture_.getSize() != sf::Vector2u(unsigned(frame.width), unsigned(frame.height))) {
            if (!texture_.create(unsigned(frame.width), unsigned(frame.height))) {
                throw std::runtime_error("Cannot create camera texture");
            }
            rgba_.resize(std::size_t(frame.width) * frame.height * 4);
        }
        for (std::size_t pixel = 0; pixel < rgba_.size() / 4; ++pixel) {
            std::copy_n(frame.rgb.data() + pixel * 3, 3, rgba_.data() + pixel * 4);
            rgba_[pixel * 4 + 3] = 255;
        }
        texture_.update(rgba_.data());
        sf::Sprite camera(texture_);
        camera.setPosition(x + w, y);
        camera.setScale(-w / frame.width, h / frame.height);
        window.draw(camera);
    }
};

void draw_frame(sf::RenderWindow& window, CameraTexture& camera, const demo::Source& source,
                const mig::Engine& engine, ActionHud& hud, const std::string& status) {
    sf::CircleShape point(3);
    point.setFillColor(sf::Color(70, 215, 160));
    window.clear(sf::Color(25, 27, 32));
    const float w = std::min(800.f, 600 * source.body.aspect), h = w / source.body.aspect;
    const float ox = (800 - w) / 2, oy = (600 - h) / 2;
    camera.draw(window, source.video(), ox, oy, w, h);
    if (!demo::profile_mode) {
        demo::draw_regions(engine, source.body.aspect, [&](const auto& points, bool trigger) {
            sf::Vertex outline[5];
            for (unsigned i = 0; i < 5; ++i) {
                const auto p = points[i % 4];
                outline[i] = sf::Vertex({ox + p.x * w, oy + p.y * h},
                                        trigger ? sf::Color::Yellow : sf::Color::Green);
            }
            window.draw(outline, 5, sf::LineStrip);
        });
    }
    demo::draw(
        source,
        [&](float x, float y, float a, float b) {
            const sf::Vertex line[] = {
                sf::Vertex({ox + (1 - x) * w, oy + y * h}, sf::Color::Green),
                sf::Vertex({ox + (1 - a) * w, oy + b * h}, sf::Color::Green)};
            window.draw(line, 2, sf::Lines);
        },
        [&](float x, float y) {
            point.setPosition(ox + (1 - x) * w - 3, oy + y * h - 3);
            window.draw(point);
        });
    for (int joint : {15, 16}) {
        if (const auto wrist = mig::body_coordinate(source.body, joint)) {
            sf::ConvexShape player(4);
            player.setPoint(0, {0, -18});
            player.setPoint(1, {24, 10});
            player.setPoint(2, {0, 3});
            player.setPoint(3, {-24, 10});
            player.setFillColor(joint == 15 ? sf::Color(70, 215, 160) : sf::Color(100, 181, 255));
            player.setPosition(ox + (1 - wrist->x) * w, oy + wrist->y * h);
            window.draw(player);
        }
    }
    hud.draw(window, status);
    window.display();
}

int run_application(int argc, char** argv) {
    try {
        demo::Options options(argc, argv);
        demo::ProfilePicker picker(argc, argv);
        auto engine = tutorial::initialize_mig(options);
        demo::Source source(options, engine.configuration().track_hands, !demo::profile_mode);
        const auto enable_hands = [&](bool enabled) { source.set_hands_enabled(enabled); };
        if (options.smoke && !std::getenv("DISPLAY")) {
            unsigned accepted = 0;
            for (int i = 0; i < 90; ++i) {
                source.sample();
                accepted += tutorial::process_tracking_frame(engine, source.body, nullptr,
                                                             !demo::profile_mode);
            }
            std::cout << "SFML host input frames=90 actions=" << accepted << '\n';
            return 0;
        }
        sf::RenderWindow window(sf::VideoMode(800, 600), options.synthetic
                                                             ? "MIG SFML - synthetic example"
                                                             : "MIG SFML - camera");
        window.setFramerateLimit(50);
        if (options.smoke) {
            window.setVisible(false);
        }
        CameraTexture camera;
        ActionHud hud(options);
        std::string status = demo::profile_mode
                                 ? "Choose a JSON profile, then keep shoulders visible."
                                 : "Lower your hands, then raise either hand.";
        std::cout << "Keep shoulders visible. Lower your hands, then raise either hand."
                  << std::endl;
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                    break;
                } else if (demo::profile_mode && event.type == sf::Event::MouseButtonReleased &&
                           event.mouseButton.button == sf::Mouse::Left &&
                           event.mouseButton.x >= 12 && event.mouseButton.x <= 260 &&
                           event.mouseButton.y >= 12 && event.mouseButton.y < 52) {
                    demo::import_profile(engine, picker.choose(), status, enable_hands);
                }
            }
            if (!window.isOpen()) {
                break;
            }
            if (!source.sample()) {
                continue;
            }
            tutorial::process_tracking_frame(engine, source.body, &status, !demo::profile_mode);
            draw_frame(window, camera, source, engine, hud, status);
            if (options.smoke && source.body.sequence >= 90) {
                window.close();
            }
        }
        // Reverse construction order stops capture before destroying the MIG engine.
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
