#pragma once

#include <SFML/Graphics.hpp>
#include <stdexec/execution.hpp>

#include "types.hpp"

class SfmlEventHandler {
public:
    using sender_concept = stdexec::sender_t;
    using completion_signatures = stdexec::completion_signatures<
        stdexec::set_value_t(),
        stdexec::set_error_t(std::exception_ptr),
        stdexec::set_stopped_t()
    >;

    class OperationStateImpl {
    public:
        OperationStateImpl(sf::RenderWindow &window, RenderSettings& render_settings, AppState &state, sf::Clock &zoom_clock):
            window_{window}, render_settings_{render_settings}, state_{state}, zoom_clock_{zoom_clock} {
        }

    protected:
        void run() {
            HandleEvents();
            HandleContinuousZoom();
        }

    private:
        void HandleEvents();
        void HandleContinuousZoom();
        void ZoomToPoint(int pixel_x, int pixel_y, bool zoom_in, double factor = 0.8);

        static constexpr float ZOOM_INTERVAL_MS = 100.0f;

        sf::RenderWindow &window_;
        RenderSettings& render_settings_;
        AppState &state_;
        sf::Clock &zoom_clock_;
    };

    template<typename Receiver>
    struct OperationState : OperationStateImpl {
        using operation_state_concept = stdexec::operation_state_t;

        Receiver receiver_;

        explicit OperationState(auto r, sf::RenderWindow &window, RenderSettings& render_settings, AppState &state,
                                sf::Clock &zoom_clock)
            : OperationStateImpl(window, render_settings, state, zoom_clock), receiver_{std::move(r)} {}

        
        void start() noexcept {
            try {
                OperationStateImpl::run();
                stdexec::set_value(std::move(receiver_));
            } catch (...) {
                stdexec::set_error(std::move(receiver_), std::current_exception());
            }
        }
    };

    SfmlEventHandler(sf::RenderWindow &window, RenderSettings& render_settings, AppState &state, sf::Clock &zoom_clock)
        : window_{window}, render_settings_{render_settings}, state_{state}, zoom_clock_{zoom_clock} {}

    auto connect(auto receiver) noexcept {
        return OperationState<decltype(receiver)>(std::move(receiver), window_, render_settings_, state_, zoom_clock_);
    }

private:
    sf::RenderWindow &window_;
    RenderSettings& render_settings_;
    AppState &state_;
    sf::Clock &zoom_clock_;
};
