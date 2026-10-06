#pragma once
#include "keybindings.hpp"
#include <mig/core/engine.hpp>

namespace mig::ui {
// Independent input runs share virtual-key ownership. Releasing one action must
// not release a modifier still owned by another action.
class ActionOutput {
public:
    bool trigger(std::size_t index, const Motion& input, std::int64_t now) {
        if (index >= slots_.size()) {
            return false;
        }
        auto& slot = slots_[index];
        if (slot.armed) {
            return true;
        }
        if (!slot.sequence.enqueue(input.keyboard, input.action_mode == ActionMode::Hold,
                                   duration(input))) {
            return false;
        }
        slot.armed = input.action_mode != ActionMode::SinglePress;
        slot.failed = false;
        slot.next_repeat = now + input.repeat_interval_ms;
        return true;
    }
    template <class Send, class TextSend>
    bool advance(std::span<const Motion> inputs, std::span<const bool> active, std::int64_t now,
                 Send send, TextSend send_text) {
        bool success = true;
        if (transient_owner_ && !slots_[*transient_owner_].sequence.executing()) {
            transient_owner_.reset();
        }
        for (std::size_t index = 0; index < slots_.size(); ++index) {
            auto& slot = slots_[index];
            const auto key = [&](int vk, bool release) { return route(slot, vk, release, send); };
            if (index >= inputs.size() || slot.failed ||
                (inputs[index].action_mode != ActionMode::SinglePress &&
                 (index >= active.size() || !active[index]))) {
                slot.sequence.cancel(key, send_text);
                slot.armed = false;
                if (transient_owner_ == index) {
                    transient_owner_.reset();
                }
                continue;
            }
            const auto& input = inputs[index];
            if (slot.armed && input.action_mode == ActionMode::Repeat && now >= slot.next_repeat &&
                slot.sequence.empty()) {
                success &= slot.sequence.enqueue(input.keyboard, false, duration(input));
                slot.next_repeat = now + input.repeat_interval_ms;
            }
            bool allow_text = true;
            for (int vk = 1; vk < 256; ++vk) {
                if (modifier(vk) && owners_[vk] > unsigned(slot.keys[vk])) {
                    allow_text = false;
                }
            }
            if (slot.sequence.executing()) {
                if (!transient_owner_) {
                    transient_owner_ = index;
                }
                if (*transient_owner_ != index) {
                    continue;
                }
            }
            if (!slot.sequence.advance(now, key, send_text, allow_text)) {
                slot.failed = true;
                slot.armed = false;
                success = false;
            }
            if (transient_owner_ == index && !slot.sequence.executing()) {
                transient_owner_.reset();
            }
        }
        return success;
    }
    template <class Send, class TextSend> void cancel(Send send, TextSend send_text) {
        transient_owner_.reset();
        for (auto& slot : slots_) {
            slot.sequence.cancel(
                [&](int vk, bool release) { return route(slot, vk, release, send); }, send_text);
            slot.armed = false;
        }
    }
    bool empty() const noexcept {
        return std::all_of(slots_.begin(), slots_.end(),
                           [](const auto& slot) { return slot.sequence.empty(); });
    }

private:
    static int duration(const Motion& input) {
        return input.action_mode == ActionMode::Repeat ? std::min(80, input.repeat_interval_ms / 2)
                                                       : 80;
    }
    struct Slot {
        KeyboardOutput sequence;
        std::array<bool, 256> keys{};
        std::int64_t next_repeat{};
        bool armed{}, failed{};
    };
    template <class Send> bool route(Slot& slot, int vk, bool release, Send send) {
        if (release) {
            if (!slot.keys[vk]) {
                return true;
            }
            if (owners_[vk] == 1 && !send(vk, true)) {
                return false;
            }
            --owners_[vk];
            slot.keys[vk] = false;
        } else if (!slot.keys[vk]) {
            if (owners_[vk] == 0 && !send(vk, false)) {
                return false;
            }
            ++owners_[vk];
            slot.keys[vk] = true;
        }
        return true;
    }
    std::array<Slot, 64> slots_;
    std::array<unsigned, 256> owners_{};
    std::optional<std::size_t> transient_owner_;
};
} // namespace mig::ui
