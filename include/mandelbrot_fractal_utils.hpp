#pragma once

#include <complex>
#include <cstdint>

namespace mandelbrot {

using Complex = std::complex<double>;

struct ViewPort {
    double x_min{-2.5};
    double x_max{1.5};
    double y_min{-2.0};
    double y_max{2.0};

    [[nodiscard]] constexpr double width() const noexcept { return x_max - x_min; }
    [[nodiscard]] constexpr double height() const noexcept { return y_max - y_min; }
};

struct RgbColor {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
};

struct RgbColors {
    RgbColors() = delete;

    static constexpr RgbColor BLACK = RgbColor{0, 0, 0};
};

[[nodiscard]] std::uint32_t CalculateIterationsForPoint(const Complex &c, std::uint32_t max_iterations,
                                                                  double escape_radius) noexcept;

[[nodiscard]] Complex Pixel2DToComplex(std::uint32_t x, std::uint32_t y, const ViewPort &viewport,
                                                 const std::uint32_t screen_width,
                                                 const std::uint32_t screen_height) noexcept;

[[nodiscard]] RgbColor IterationsToColor(std::uint32_t iterations, std::uint32_t max_iterations) noexcept;

}  // namespace mandelbrot