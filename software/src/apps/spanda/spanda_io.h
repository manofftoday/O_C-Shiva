#pragma once

#include <stdint.h>

namespace spanda {

class AxisCalibration {
public:
  void Reset() { minimum_ = -2048; maximum_ = 2048; inverted_ = false; active_ = false; }
  void Restore(int16_t minimum, int16_t maximum, bool inverted) {
    minimum_ = minimum;
    maximum_ = maximum;
    inverted_ = inverted;
    if (minimum_ >= maximum_ || maximum_ - minimum_ < 256) Reset();
  }
  void Begin() { sample_min_ = 32767; sample_max_ = -32768; active_ = true; }
  void Observe(int value) {
    if (!active_) return;
    if (value < sample_min_) sample_min_ = value;
    if (value > sample_max_) sample_max_ = value;
  }
  void Finish() {
    if (active_ && sample_max_ - sample_min_ >= 256) {
      minimum_ = sample_min_;
      maximum_ = sample_max_;
    }
    active_ = false;
  }
  bool active() const { return active_; }
  bool inverted() const { return inverted_; }
  void ToggleInvert() { inverted_ = !inverted_; }
  int16_t minimum() const { return minimum_; }
  int16_t maximum() const { return maximum_; }
  float Normalize(int raw) const {
    const int span = maximum_ - minimum_;
    const float center = (int(maximum_) + minimum_) * 0.5f;
    float value = (raw - center) * 2.0f / (span > 0 ? span : 4096);
    if (value < -1.0f) value = -1.0f;
    if (value > 1.0f) value = 1.0f;
    return inverted_ ? -value : value;
  }

private:
  int16_t minimum_ = -2048;
  int16_t maximum_ = 2048;
  int16_t sample_min_ = 32767;
  int16_t sample_max_ = -32768;
  bool inverted_ = false;
  bool active_ = false;
};

} // namespace spanda
