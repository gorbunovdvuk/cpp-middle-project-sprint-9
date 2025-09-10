#pragma once

#include "mandelbrot_renderer.hpp"

#include <any>

class CalculateMandelbrotAsyncSender {
public:
    using sender_concept = stdexec::sender_t;
    using completion_signatures = stdexec::completion_signatures<
        stdexec::set_value_t(RenderResult),
        stdexec::set_error_t(std::exception_ptr),
        stdexec::set_stopped_t()
    >;

    explicit CalculateMandelbrotAsyncSender(AppState &state, RenderSettings render_settings, MandelbrotRenderer &renderer);

    template <typename Receiver>
    struct OperationState {
        using operation_state_concept = stdexec::operation_state_t;

        Receiver receiver_;
        AppState &state_;
        RenderSettings settings_;
        MandelbrotRenderer &renderer_;

        template<typename sender_t>
        using operation_state_t = decltype(stdexec::connect(
            std::move(std::declval<sender_t>()),
            std::move(std::declval<Receiver>())
        ));

        std::any operation_state_;

        void start() noexcept {
            try {
                if (state_.need_rerender) {
                    auto snd = renderer_.template RenderAsync<8>(state_.viewport, settings_) |
                        stdexec::then([st = &state_](RenderResult&& r) {
                            st->need_rerender = false;
                            return r;
                        });
                    using ptr_type = operation_state_t<decltype(snd)>;
                    operation_state_ = std::shared_ptr<ptr_type>(new (ptr_type) (stdexec::connect(std::move(snd), std::move(receiver_))));
                    stdexec::start(*std::any_cast<std::shared_ptr<ptr_type>>(operation_state_));
                } else {
                    RenderResult out;
                    out.viewport = state_.viewport;
                    out.settings = settings_;
                    out.render_time = std::chrono::milliseconds{0};
                    stdexec::set_value(std::move(receiver_), std::move(out));
                }
            } catch (...) {
                stdexec::set_error(std::move(receiver_), std::current_exception());
            }
        }
    };

    auto connect(auto receiver) noexcept {
        return OperationState<decltype(receiver)>{std::move(receiver), state_, render_settings_, renderer_};
    }

private:
    RenderSettings render_settings_;
    MandelbrotRenderer &renderer_;
    AppState &state_;
};
