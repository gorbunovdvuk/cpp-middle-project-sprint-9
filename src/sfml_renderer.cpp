#include "sfml_renderer.hpp"

void SFMLRender::OperationStateImpl::run() {
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
}