#pragma once
#include <algorithm>

// The clock is independent of OpenGL, and advances only with scene delta time.
class DissolveState {
public:
    static constexpr float Duration = 0.9f;
    void reset() { elapsed = 0.f; started = false; }
    void start() { started = true; }
    void update(float dt) {
        if (started) elapsed = std::min(Duration, elapsed + std::max(0.f, dt));
    }
    float progress() const { return elapsed / Duration; }
    bool finished() const { return started && elapsed >= Duration; }
private:
    float elapsed = 0.f;
    bool started = false;
};
