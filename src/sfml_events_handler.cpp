#include "sfml_events_handler.hpp"

void SfmlEventHandler::OperationStateImpl::HandleEvents() {
    while (auto event = window_.pollEvent()) {
        if (event->template is<sf::Event::Closed>()) {
            state_.should_exit = true;
            window_.close();
        } else if (auto key = event->template getIf<sf::Event::KeyPressed>()) {
            switch (key->code) {
            case sf::Keyboard::Key::Escape: {
                state_.should_exit = true;
                window_.close();
                break;
            }
            case sf::Keyboard::Key::LBracket: {
                render_settings_.max_iterations = std::max(50u, render_settings_.max_iterations - 50);
                state_.need_rerender = true;
                break;
            }
            case sf::Keyboard::Key::RBracket: {
                render_settings_.max_iterations += 50;
                state_.need_rerender = true;
                break;
            }
            case sf::Keyboard::Key::R: {
                state_.viewport = mandelbrot::ViewPort{};
                state_.need_rerender = true;
                break;
            }
            case sf::Keyboard::Key::M: {
                render_settings_.escape_radius = std::max(0.5, render_settings_.escape_radius - 1.0);
                state_.need_rerender = true;
                break;
            }
            case sf::Keyboard::Key::P: {
                render_settings_.escape_radius = std::min(100.0, render_settings_.escape_radius + 1.0);
                state_.need_rerender = true;
                break;
            }
            default: {
                break;
            }
            }
        } else if (auto m = event->template getIf<sf::Event::MouseButtonPressed>()) {
            if (m->button == sf::Mouse::Button::Left) {
                state_.left_mouse_pressed = true;
            } else if (m->button == sf::Mouse::Button::Right) {
                state_.right_mouse_pressed = true;
            }
        } else if (auto m = event->template getIf<sf::Event::MouseButtonReleased>()) {
            if (m->button == sf::Mouse::Button::Left) {
                state_.left_mouse_pressed = false;
            } else if (m->button == sf::Mouse::Button::Right) {
                state_.right_mouse_pressed = false;
            }
        }
    }
}
void SfmlEventHandler::OperationStateImpl::HandleContinuousZoom() {
    if ((state_.left_mouse_pressed || state_.right_mouse_pressed) &&
        zoom_clock_.getElapsedTime().asMilliseconds() >= ZOOM_INTERVAL_MS) {

        sf::Vector2i mouse_pos = sf::Mouse::getPosition(window_);

        if (mouse_pos.x >= 0 && mouse_pos.x < static_cast<int>(render_settings_.width) && mouse_pos.y >= 0 &&
            mouse_pos.y < static_cast<int>(render_settings_.height)) {

            ZoomToPoint(mouse_pos.x, mouse_pos.y, state_.left_mouse_pressed);
            zoom_clock_.restart();
        }
    }
}
void SfmlEventHandler::OperationStateImpl::ZoomToPoint(int pixel_x, int pixel_y, bool zoom_in, double factor) {
    const double target_x =
        state_.viewport.x_min + (static_cast<double>(pixel_x) / render_settings_.width) * state_.viewport.width();
    const double target_y =
        state_.viewport.y_min + (static_cast<double>(pixel_y) / render_settings_.height) * state_.viewport.height();

    const double zoom_factor = zoom_in ? factor : (1.0 / factor);
    const double new_width = state_.viewport.width() * zoom_factor;
    const double new_height = state_.viewport.height() * zoom_factor;

    const double new_x_min = target_x - new_width * 0.5;
    const double new_x_max = target_x + new_width * 0.5;
    const double new_y_min = target_y - new_height * 0.5;
    const double new_y_max = target_y + new_height * 0.5;

    state_.viewport = mandelbrot::ViewPort{
        .x_min = new_x_min,
        .x_max = new_x_max,
        .y_min = new_y_min,
        .y_max = new_y_max,
    };
    state_.need_rerender = true;
}