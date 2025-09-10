#pragma once

#include <exec/static_thread_pool.hpp>
#include <stdexec/execution.hpp>

#include "mandelbrot_sender.hpp"
#include "types.hpp"

class MandelbrotRenderer {
public:
    explicit MandelbrotRenderer(std::uint32_t num_threads = std::thread::hardware_concurrency())
        : thread_pool_{num_threads} {}

    template <size_t N>
    requires (N > 0)
    [[nodiscard]] auto RenderAsync(mandelbrot::ViewPort viewport, RenderSettings settings) {
        auto whenAllSender = [&]<size_t... Is>(std::index_sequence<Is...>) {
            auto scheduler = thread_pool_.get_scheduler();
            auto make_for_index = []<size_t I>(RenderSettings& settings, auto& scheduler, mandelbrot::ViewPort& viewport) {
                size_t base_rows = settings.height / N, remaining_rows = settings.height % N;
                std::uint32_t start = base_rows * I + std::min(I, remaining_rows), size = base_rows + (I < remaining_rows ? 1 : 0);
                PixelRegion region{start, start + size, 0u, settings.width};
                return stdexec::on(scheduler, MandelbrotSender{viewport, settings, region}) |
                    stdexec::then([region, settings](PixelMatrix pixel_matrix) {
                        size_t height = pixel_matrix.size(), width = pixel_matrix.begin()->size();
                        ColorMatrix color_matrix(height, std::vector<mandelbrot::RgbColor>(width));
                        for (size_t y = 0; y < height; ++y) {
                            for (size_t x = 0; x < width; ++x) {
                                color_matrix[y][x] = mandelbrot::IterationsToColor(pixel_matrix[y][x], settings.max_iterations);
                            }
                        }
                        return std::tuple{region, pixel_matrix, color_matrix};
                    });
            };
            return stdexec::when_all(make_for_index.template operator()<Is>(settings, scheduler, viewport)...);
        }(std::make_index_sequence<N>{});
        return stdexec::just(std::chrono::steady_clock::now()) |
            stdexec::let_value([=](auto t0) {
                return std::move(whenAllSender) |
                    stdexec::then([=](auto&&... parts) {
                        RenderResult result;
                        result.viewport = viewport;
                        result.settings = settings;
                        result.pixel_data.assign(settings.height, std::vector<std::uint32_t>(settings.width));
                        result.color_data.assign(settings.height, std::vector<mandelbrot::RgbColor>(settings.width));
                        (
                            std::invoke([&]() {
                                auto [region, pixel_matrix, color_matrix] = std::move(parts);
                                for (std::size_t y = 0; y < pixel_matrix.size(); ++y) {
                                    const std::size_t gy = static_cast<std::size_t>(region.start_row) + y;
                                    for (std::size_t x = 0; x < pixel_matrix[y].size(); ++x) {
                                        const std::size_t gx = static_cast<std::size_t>(region.start_col) + x;
                                        result.pixel_data[gy][gx] = pixel_matrix[y][x];
                                        result.color_data[gy][gx] = color_matrix[y][x];
                                    }
                                }
                            }), ...
                        );
                        result.render_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - t0
                        );
                        return result;
                    });
            });
    }

private:
    exec::static_thread_pool thread_pool_;
};
