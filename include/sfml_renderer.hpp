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

    class OperationStateImpl {
    public:
        OperationStateImpl(RenderResult render_result, sf::Image &image, sf::Texture &texture, sf::Sprite &sprite, sf::RenderWindow &window, RenderSettings render_settings):
            render_result_(std::move(render_result)), image_(image), texture_(texture), sprite_(sprite), window_(window), render_settings_(std::move(render_settings)) {}

    protected:
        void run();

    private:
        RenderResult render_result_;
        sf::Image &image_;
        sf::Texture &texture_;
        sf::Sprite &sprite_;
        sf::RenderWindow &window_;
        RenderSettings render_settings_;
    };

    template <typename Receiver>
    struct OperationState : public OperationStateImpl {
        using operation_state_concept = stdexec::operation_state_t;

        OperationState(Receiver receiver, RenderResult render_result, sf::Image &image, sf::Texture &texture, sf::Sprite &sprite, sf::RenderWindow &window, RenderSettings render_settings):
            OperationStateImpl(std::move(render_result), image, texture, sprite, window, std::move(render_settings)),
            receiver_(std::move(receiver)) {}

        Receiver receiver_;

        void start() noexcept {
            try {
                OperationStateImpl::run();
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
