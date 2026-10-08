#include "example_usage.hpp"
#include "hud.hpp"
#include "support/drawing.hpp"
#include "support/source.hpp"
#include <SDL.h>

class CameraTexture {
    SDL_Texture* texture_{};
    int width_{}, height_{};

public:
    CameraTexture() = default;
    CameraTexture(const CameraTexture&) = delete;
    CameraTexture& operator=(const CameraTexture&) = delete;
    ~CameraTexture() {
        SDL_DestroyTexture(texture_);
    }
    void draw(SDL_Renderer* renderer, const mig::native::VideoFrame& frame) {
        if (frame.rgb.empty()) {
            return;
        }
        if (frame.width != width_ || frame.height != height_) {
            SDL_DestroyTexture(texture_);
            texture_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
                                         SDL_TEXTUREACCESS_STREAMING, frame.width, frame.height);
            if (!texture_) {
                throw std::runtime_error(SDL_GetError());
            }
            width_ = frame.width;
            height_ = frame.height;
        }
        SDL_UpdateTexture(texture_, nullptr, frame.rgb.data(), frame.width * 3);
        SDL_RenderCopyEx(renderer, texture_, nullptr, nullptr, 0, nullptr, SDL_FLIP_HORIZONTAL);
    }
};

void draw_frame(SDL_Renderer* renderer, CameraTexture& camera, const demo::Source& source,
                const mig::Engine& engine, ActionHud& hud, const std::string& status) {
    SDL_RenderSetViewport(renderer, nullptr);
    SDL_SetRenderDrawColor(renderer, 25, 27, 32, 255);
    SDL_RenderClear(renderer);
    int width = 800, height = 600;
    SDL_GetRendererOutputSize(renderer, &width, &height);
    const float aspect = source.body.aspect;
    const int w = int(std::min(float(width), height * aspect));
    const int h = int(w / aspect);
    SDL_Rect view{(width - w) / 2, (height - h) / 2, w, h};
    SDL_RenderSetViewport(renderer, &view);
    camera.draw(renderer, source.video());
    if (!demo::profile_mode) {
        demo::draw_regions(engine, aspect, [&](const auto& points, bool trigger) {
            SDL_SetRenderDrawColor(renderer, trigger ? 255 : 70, trigger ? 210 : 215,
                                   trigger ? 60 : 160, 255);
            for (unsigned i = 0; i < 4; ++i) {
                const auto a = points[i], b = points[(i + 1) % 4];
                SDL_RenderDrawLineF(renderer, a.x * w, a.y * h, b.x * w, b.y * h);
            }
        });
    }
    SDL_SetRenderDrawColor(renderer, 70, 215, 160, 255);
    demo::draw(
        source,
        [&](float x, float y, float a, float b) {
            SDL_RenderDrawLineF(renderer, (1 - x) * w, y * h, (1 - a) * w, b * h);
        },
        [&](float x, float y) {
            SDL_FRect rect{(1 - x) * w - 3, y * h - 3, 6, 6};
            SDL_RenderFillRectF(renderer, &rect);
        });
    // Use XYZ directly for custom controls, independently of configured motions.
    for (int joint : {15, 16}) {
        if (const auto wrist = mig::body_coordinate(source.body, joint)) {
            SDL_SetRenderDrawColor(renderer, joint == 15 ? 70 : 100, joint == 15 ? 215 : 181,
                                   joint == 15 ? 160 : 255, 255);
            const float x = (1 - wrist->x) * w, y = wrist->y * h;
            const SDL_FPoint prop[] = {
                {x, y - 18}, {x + 24, y + 10}, {x, y + 3}, {x - 24, y + 10}, {x, y - 18}};
            SDL_RenderDrawLinesF(renderer, prop, 5);
        }
    }
    hud.draw(renderer, status);
    SDL_RenderPresent(renderer);
}

int run_application(int argc, char** argv) {
    try {
        demo::Options options(argc, argv);
        demo::ProfilePicker picker(argc, argv);
        auto engine = tutorial::initialize_mig(options);
        demo::Source source(options, engine.configuration().track_hands, !demo::profile_mode);
        const auto enable_hands = [&](bool enabled) { source.set_hands_enabled(enabled); };
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw std::runtime_error(SDL_GetError());
        }
        struct Quit {
            ~Quit() {
                SDL_Quit();
            }
        } quit;
        const auto destroy_window = [](SDL_Window* p) { SDL_DestroyWindow(p); };
        std::unique_ptr<SDL_Window, decltype(destroy_window)> window(
            SDL_CreateWindow(options.synthetic ? "MIG SDL2 - synthetic example"
                                               : "MIG SDL2 - camera",
                             SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0),
            destroy_window);
        if (!window) {
            throw std::runtime_error(SDL_GetError());
        }
        const auto destroy_renderer = [](SDL_Renderer* p) { SDL_DestroyRenderer(p); };
        std::unique_ptr<SDL_Renderer, decltype(destroy_renderer)> renderer(
            SDL_CreateRenderer(window.get(), -1, options.smoke ? SDL_RENDERER_SOFTWARE : 0),
            destroy_renderer);
        if (!renderer) {
            throw std::runtime_error(SDL_GetError());
        }
        CameraTexture camera;
        ActionHud hud(options);
        std::string status = demo::profile_mode
                                 ? "Choose a JSON profile, then keep shoulders visible."
                                 : "Lower your hands, then raise either hand.";
        std::cout << "Keep shoulders visible. Lower your hands, then raise either hand."
                  << std::endl;
        bool running = true;
        unsigned frames = 0, accepted = 0;
        while (running && (!options.smoke || frames < 90)) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                    break;
                } else if (demo::profile_mode && event.type == SDL_MOUSEBUTTONUP &&
                           event.button.button == SDL_BUTTON_LEFT && event.button.x >= 12 &&
                           event.button.x <= 260 && event.button.y >= 12 && event.button.y < 52) {
                    demo::import_profile(engine, picker.choose(), status, enable_hands);
                } else if (event.type == SDL_DROPFILE) {
                    std::unique_ptr<char, decltype(&SDL_free)> file(event.drop.file, SDL_free);
                    if (demo::profile_mode) {
                        demo::import_profile(engine, demo::utf8_path(file.get()), status,
                                             enable_hands);
                    }
                }
            }
            if (!running) {
                break;
            }
            if (!source.sample()) {
                continue;
            }
            ++frames;
            accepted +=
                tutorial::process_tracking_frame(engine, source.body, &status, !demo::profile_mode);
            draw_frame(renderer.get(), camera, source, engine, hud, status);
            if (!options.smoke && options.synthetic) {
                SDL_Delay(20);
            }
        }
        std::cout << "SDL2 frames=" << frames << " actions=" << accepted << '\n';
        // Reverse construction order stops capture before destroying the MIG engine.
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
