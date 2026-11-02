#pragma once

#include <stdint.h>

namespace spanda {

class Gesture {
public:
  enum State : uint8_t { GESTURE_IDLE, GESTURE_RECORD, GESTURE_PLAY, GESTURE_ATTRACT };
  static constexpr uint16_t Capacity = 600;
  static constexpr uint16_t SampleTicks = 100;

  void Reset() { state_ = GESTURE_IDLE; size_ = position_ = clock_ = 0; }
  void Start() { state_ = GESTURE_RECORD; size_ = position_ = clock_ = 0; }
  State state() const { return state_; }
  uint16_t size() const { return size_; }
  void set_state(State state) { state_ = state; }
  void Capture(float x, float y, uint8_t length) {
    if (state_ != GESTURE_RECORD || ++clock_ < SampleTicks) return;
    clock_ = 0;
    if (size_ < Capacity) {
      samples_[size_][0] = static_cast<int16_t>(x * 32767.0f);
      samples_[size_][1] = static_cast<int16_t>(y * 32767.0f);
      ++size_;
    }
    const uint16_t target = length == 0 ? 100 : length == 1 ? 200 : 600;
    if (size_ >= target || size_ >= Capacity) {
      state_ = GESTURE_PLAY;
      position_ = 0;
    }
  }
  float Axis(int axis) const {
    if (!size_) return 0.0f;
    const uint16_t next = (position_ + 1) % size_;
    const float phase = clock_ < SampleTicks ? float(clock_) / SampleTicks : 1.0f;
    const float current = float(samples_[position_][axis]) / 32767.0f;
    const float following = float(samples_[next][axis]) / 32767.0f;
    return current + (following - current) * phase;
  }
  void Advance() {
    if (state_ != GESTURE_PLAY && state_ != GESTURE_ATTRACT) return;
    if (++clock_ >= SampleTicks) {
      clock_ = 0;
      if (size_ && ++position_ >= size_) position_ = 0;
    }
  }

private:
  int16_t samples_[Capacity][2]{};
  uint16_t size_ = 0;
  uint16_t position_ = 0;
  uint16_t clock_ = 0;
  State state_ = GESTURE_IDLE;
};

} // namespace spanda
