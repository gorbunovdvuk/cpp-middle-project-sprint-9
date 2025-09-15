#include "mandelbrot_sender.hpp"

PixelMatrix MandelbrotSender::OperationStateImpl::run() {
    uint32_t height = region_.end_row - region_.start_row;
    uint32_t width = region_.end_col - region_.start_col;
    PixelMatrix out{height, std::vector<uint32_t>(width, 0)};
    for (uint32_t y = region_.start_row; y < region_.end_row; ++y) {
        for (uint32_t x = region_.start_col; x < region_.end_col; ++x) {
            auto c = mandelbrot::Pixel2DToComplex(x, y, viewport_, settings_.width, settings_.height);
            auto iters = mandelbrot::CalculateIterationsForPoint(c, settings_.max_iterations, settings_.escape_radius);
            out[y - region_.start_row][x - region_.start_col] = iters;
        }
    }
    return out;
}