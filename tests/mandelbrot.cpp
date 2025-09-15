#include "mandelbrot.hpp"

#include "mandelbrot_renderer.hpp"
#include "mandelbrot_sender.hpp"
#include "types.hpp"

#include <gtest/gtest.h>

TEST(Mandelbrot, MandelbrotSender) {
    auto result = stdexec::sync_wait(MandelbrotSender{
        mandelbrot::ViewPort{},
        RenderSettings{.width = 64, .height = 32},
        PixelRegion{.start_row = 0, .end_row = 32, .start_col = 0, .end_col = 64}
    });
    EXPECT_TRUE(result.has_value());
    auto [pixel_matrix] = *result;
    EXPECT_EQ(pixel_matrix.size(), 32);
    EXPECT_EQ(pixel_matrix.begin()->size(), 64);
}

TEST(Mandelbrot, MandelbrotRenderer) {
    auto result = stdexec::sync_wait(MandelbrotRenderer{8}.RenderAsync<4>(mandelbrot::ViewPort{}, RenderSettings{.width = 64, .height = 32}));
    EXPECT_TRUE(result.has_value());
    auto [render_result] = *result;
    EXPECT_EQ(render_result.pixel_data.size(), 32);
    EXPECT_EQ(render_result.pixel_data.begin()->size(), 64);
    EXPECT_EQ(render_result.color_data.size(), 32);
    EXPECT_EQ(render_result.color_data.begin()->size(), 64);
}

TEST(Mandelbrot, CalculateMandelbrotAsyncSender) {
    AppState app_state;
    MandelbrotRenderer renderer{8};
    {
        auto result = stdexec::sync_wait(CalculateMandelbrotAsyncSender{app_state, RenderSettings{.width = 64, .height = 32}, renderer});
        EXPECT_TRUE(result.has_value());
        auto [render_result] = *result;
        EXPECT_EQ(render_result.pixel_data.size(), 32);
        EXPECT_EQ(render_result.pixel_data.begin()->size(), 64);
        EXPECT_EQ(render_result.color_data.size(), 32);
        EXPECT_EQ(render_result.color_data.begin()->size(), 64);
        EXPECT_FALSE(app_state.need_rerender);
    }
    {
        auto result = stdexec::sync_wait(CalculateMandelbrotAsyncSender{app_state, RenderSettings{.width = 64, .height = 32}, renderer});
        EXPECT_TRUE(result.has_value());
        auto [render_result] = *result;
        EXPECT_TRUE(render_result.pixel_data.empty());
        EXPECT_TRUE(render_result.color_data.empty());
        EXPECT_FALSE(app_state.need_rerender);
    }
    app_state.need_rerender = true;
    {
        auto result = stdexec::sync_wait(CalculateMandelbrotAsyncSender{app_state, RenderSettings{.width = 64, .height = 32}, renderer});
        EXPECT_TRUE(result.has_value());
        auto [render_result] = *result;
        EXPECT_EQ(render_result.pixel_data.size(), 32);
        EXPECT_EQ(render_result.pixel_data.begin()->size(), 64);
        EXPECT_EQ(render_result.color_data.size(), 32);
        EXPECT_EQ(render_result.color_data.begin()->size(), 64);
        EXPECT_FALSE(app_state.need_rerender);
    }
}