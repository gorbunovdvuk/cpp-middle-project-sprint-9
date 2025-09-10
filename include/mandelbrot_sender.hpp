#pragma once

#include <stdexec/execution.hpp>

#include "types.hpp"

struct MandelbrotSender {
    using sender_concept = stdexec::sender_t;
    using completion_signatures = stdexec::completion_signatures<
        stdexec::set_value_t(PixelMatrix&&),
        stdexec::set_error_t(std::exception_ptr),
        stdexec::set_stopped_t()
    >;

    template<typename Receiver>
    struct OperationState {
        using operation_state_concept = stdexec::operation_state_t;

        Receiver receiver_;
        mandelbrot::ViewPort viewport_;
        RenderSettings settings_;
        PixelRegion region_;

        void start() noexcept {
            try {
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
                stdexec::set_value(std::move(receiver_), std::move(out));
            } catch (...) {
                stdexec::set_error(std::move(receiver_), std::current_exception());
            }

        }
    };

    auto connect(auto receiver) {
        return OperationState<decltype(receiver)>(std::move(receiver), viewport_, settings_, region_);
    }

    mandelbrot::ViewPort viewport_;
    RenderSettings settings_;
    PixelRegion region_;
};
