#pragma once

#include <SFML/Graphics.hpp>
#include <print>
#include <stdexec/execution.hpp>

#include "types.hpp"

class SFMLRender {
public:
    using sender_concept = stdexec::sender_t;
    using completion_signatures = stdexec::completion_signatures<
        stdexec::set_value_t(),
        stdexec::set_error_t(std::exception_ptr),
        stdexec::set_stopped_t()
    >;

    template <typename Receiver>
    struct OperationState {
        using operation_state_concept = stdexec::operation_state_t;

        Receiver receiver_;
        RenderResult render_result_;
        sf::Image &image_;
        sf::Texture &texture_;
        sf::Sprite &sprite_;
        sf::RenderWindow &window_;
        RenderSettings render_settings_;

        void start() noexcept {
            try {
                if (!render_result_.color_data.empty()) {
                    auto w = static_cast<uint32_t>(render_result_.color_data.begin()->size());
                    auto h = static_cast<uint32_t>(render_result_.color_data.size());
                    for (uint32_t y = 0; y < h; ++y) {
                        for (uint32_t x = 0; x < w; ++x) {
                            auto c = render_result_.color_data[y][x];
                            image_.setPixel(sf::Vector2u{x, y}, sf::Color{c.r, c.g, c.b});
                        }
                    }
                    texture_.update(image_);
                    sprite_.setTexture(texture_);
                }
                window_.clear();
                window_.draw(sprite_);
                window_.display();
                stdexec::set_value(std::move(receiver_));
            } catch (...) {
                stdexec::set_error(std::move(receiver_), std::current_exception());
            }
        }
    };

    RenderResult render_result_;
    sf::Image &image_;
    sf::Texture &texture_;
    sf::Sprite &sprite_;
    sf::RenderWindow &window_;
    RenderSettings render_settings_;

    SFMLRender(RenderResult render_result, sf::Image &image, sf::Texture &texture, sf::Sprite &sprite,
               sf::RenderWindow &window, RenderSettings render_settings)
        : render_result_(render_result), image_{image}, texture_{texture}, sprite_{sprite}, window_{window},
          render_settings_{render_settings} {}

    auto connect(auto receiver) noexcept {
        return OperationState<decltype(receiver)>{std::move(receiver), render_result_, image_, texture_, sprite_, window_, render_settings_};
    }
};
