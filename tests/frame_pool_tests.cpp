#include "../src/apps/frame_pool.hpp"

int main() {
    mig::app::FramePool pool;
    std::array<std::shared_ptr<mig::native::VideoFrame>, 4> leases;
    for (auto& lease : leases) {
        lease = pool.acquire();
        if (!lease) {
            return 1;
        }
    }
    if (pool.acquire()) {
        return 2;
    }
    leases[0]->width = 123;
    const auto* slot = leases[0].get();
    std::shared_ptr<const mig::native::VideoFrame> reader = leases[0];
    leases[0].reset();
    if (pool.acquire()) {
        return 3; // Reader still owns the slot.
    }
    reader.reset();
    auto recycled = pool.acquire();
    if (!recycled || recycled.get() != slot || recycled->width != 123) {
        return 4;
    }
    std::shared_ptr<mig::native::VideoFrame> survivor;
    {
        mig::app::FramePool temporary;
        survivor = temporary.acquire();
    }
    survivor->width = 456; // Outstanding lease keeps the whole backing state alive.
    if (survivor->width != 456) {
        return 5;
    }
}
