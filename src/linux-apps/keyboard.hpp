#pragma once
#include <memory>
#include <string>
namespace mig::linux_ui {
class Keyboard {
public:
    Keyboard();
    ~Keyboard();
    bool available() const;
    bool key(int virtual_key, bool release);
    bool text(char16_t character);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace mig::linux_ui
