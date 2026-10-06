#pragma once
#include "camera.hpp"
#include <array>
#include <memory>
#include <mutex>

namespace mig::app {
// Last-reader release returns a slot under a mutex: a real happens-before,
// not a relaxed use_count() uniqueness guess. Leases keep storage alive on stop.
class FramePool {
public:
    std::shared_ptr<native::VideoFrame> acquire() {
        std::size_t index = capacity;
        {
            std::lock_guard lock(state_->mutex);
            for (std::size_t candidate = 0; candidate < capacity; ++candidate) {
                if (state_->available[candidate]) {
                    state_->available[candidate] = false;
                    index = candidate;
                    break;
                }
            }
        }
        if (index == capacity) {
            return {};
        }
        // Construct outside the lock: allocation failure invokes this deleter too.
        return {&state_->frames[index], [state = state_, index](native::VideoFrame*) {
                    std::lock_guard lock(state->mutex);
                    state->available[index] = true;
                }};
    }

private:
    static constexpr std::size_t capacity = 4;
    struct State {
        std::mutex mutex;
        std::array<native::VideoFrame, capacity> frames;
        std::array<bool, capacity> available{true, true, true, true};
    };
    std::shared_ptr<State> state_ = std::make_shared<State>();
};
} // namespace mig::app
