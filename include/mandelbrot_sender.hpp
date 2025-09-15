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

    class OperationStateImpl {
    public:
        OperationStateImpl(mandelbrot::ViewPort viewport, RenderSettings settings, PixelRegion region):
            viewport_(std::move(viewport)), settings_(std::move(settings)), region_(std::move(region)) {}

    protected:
        PixelMatrix run();

    private:
        mandelbrot::ViewPort viewport_;
        RenderSettings settings_;
        PixelRegion region_;
    };

    template<typename Receiver>
    struct OperationState : OperationStateImpl {
        using operation_state_concept = stdexec::operation_state_t;

        OperationState(Receiver receiver, mandelbrot::ViewPort viewport, RenderSettings settings, PixelRegion region):
            OperationStateImpl(std::move(viewport), std::move(settings), std::move(region)),
            receiver_(std::move(receiver)) {}

        Receiver receiver_;

        void start() noexcept {
            try {
                stdexec::set_value(std::move(receiver_), OperationStateImpl::run());
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
