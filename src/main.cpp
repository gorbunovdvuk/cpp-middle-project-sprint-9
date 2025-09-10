#include <chrono>
#include <print>
#include <thread>
#include <utility>

#include <SFML/Graphics.hpp>
#include <exec/repeat_effect_until.hpp>
#include <exec/static_thread_pool.hpp>
#include <stdexec/execution.hpp>

#include "mandelbrot.hpp"
#include "mandelbrot_renderer.hpp"
#include "sfml_events_handler.hpp"
#include "sfml_renderer.hpp"

using namespace std::chrono_literals;

class FrameClock {
public:
    FrameClock() { Reset(); }

    void Reset() noexcept { frame_start_ = std::chrono::steady_clock::now(); }
    auto GetFrameTime() const noexcept { return std::chrono::steady_clock::now() - frame_start_; }

private:
    std::chrono::time_point<std::chrono::steady_clock> frame_start_;
};

class WaitForFPS {
public:
    WaitForFPS(FrameClock& frame_clock, uint32_t fps) : frame_clock_(frame_clock), frame_time_(1s / std::max(1u, fps)) {}

    void operator()() const noexcept {
        if (auto elapsed = frame_clock_.GetFrameTime(); elapsed < frame_time_) {
            std::this_thread::sleep_for(frame_time_ - elapsed);
        }
        frame_clock_.Reset();
    }

private:
    FrameClock& frame_clock_;
    std::chrono::milliseconds frame_time_;
};

class MandelbrotApp {
public:
    MandelbrotApp(): window_{sf::VideoMode{sf::Vector2u{render_settings_.width, render_settings_.height}}, "Mandelbrot Fractal"},
        image_{sf::Vector2u{render_settings_.width, render_settings_.height}, sf::Color::Black},
        texture_{sf::Vector2u{render_settings_.width, render_settings_.height}},
        sprite_(texture_),
        renderer_{THREAD_POOL_SIZE}
    {
        window_.setKeyRepeatEnabled(false);
    }

    void Run() {
        FrameClock frame_clock;
        sf::Clock zoom_clock;

        auto pipeline = SfmlEventHandler{window_, render_settings_, state_, zoom_clock} |  //
                        stdexec::let_value([this]() {                                      //
                            return CalculateMandelbrotAsyncSender{state_, render_settings_, renderer_};
                        }) |
                        stdexec::let_value([this](RenderResult data) {
                            return SFMLRender{std::move(data), image_, texture_, sprite_, window_, render_settings_};
                        }) |  //
                        stdexec::then(WaitForFPS{frame_clock, 60});

        auto repeated_pipeline =
            std::move(pipeline) | stdexec::then([this]() { return state_.should_exit; }) | exec::repeat_effect_until();

        stdexec::sync_wait(std::move(repeated_pipeline));
    }

private:
    RenderSettings render_settings_;
    sf::RenderWindow window_;
    sf::Image image_;
    sf::Texture texture_;
    sf::Sprite sprite_;
    MandelbrotRenderer renderer_;
    AppState state_;
};

int main() {
    try {
        MandelbrotApp app;
        app.Run();
    } catch (const std::exception &e) {
        std::println("Error: {}", e.what());
        return 1;
    }
    return 0;
}