#include "mandelbrot.hpp"

CalculateMandelbrotAsyncSender::CalculateMandelbrotAsyncSender(AppState &state, RenderSettings render_settings,
                                                               MandelbrotRenderer &renderer)
    : state_(state), render_settings_{render_settings}, renderer_{renderer} {}