#pragma once

#include <math.h>
#include <stdint.h>

namespace spanda {

class Spectral {
public:
  void Reset() { walk_ = 0.0f; breath_ = 0.0f; }

  void UpdateWalk(float random_value, uint8_t irregularity) {
    walk_ = walk_ * 0.94f + random_value * (0.02f + irregularity / 6500.0f);
    walk_ = walk_ < -1.0f ? -1.0f : walk_ > 1.0f ? 1.0f : walk_;
  }

  void UpdateBreath(float phase, uint8_t breath_amount) {
    const float target = 0.5f + 0.5f * sinf(phase * 0.035f);
    breath_ += (target - breath_) * 0.006f;
    (void)breath_amount;
  }

  float Process(float phase, float nx, float density, float previous_fm,
                float random_value, uint8_t irregularity, uint8_t fold_amount,
                uint8_t shape_amount, uint8_t breath_amount) {
    UpdateWalk(random_value, irregularity);
    UpdateBreath(phase, breath_amount);
    const float input = (nx + walk_ * 0.45f + previous_fm * 0.25f) *
      (1.0f + fold_amount / 48.0f) * (0.35f + breath_ * breath_amount / 180.0f);
    const float folded = (sinf(input * 2.8f) * 0.55f +
      SoftLimit(input * (0.75f + shape_amount / 128.0f)) * 0.45f) * (0.45f + density * 0.55f);
    return folded < -1.0f ? -1.0f : folded > 1.0f ? 1.0f : folded;
  }

  float walk() const { return walk_; }
  float breath() const { return breath_; }

  static float Morph(float ny, float nz, float previous_ratio,
                     uint8_t shape_amount, float gesture_y, uint8_t mutation) {
    const float value = (nz * 0.62f + ny * 0.22f + previous_ratio * 0.16f) *
      (0.4f + shape_amount / 180.0f) + gesture_y * mutation / 1020.0f;
    return value < -1.0f ? -1.0f : value > 1.0f ? 1.0f : value;
  }

private:
  static float SoftLimit(float value) { return value / (1.0f + fabsf(value)); }
  float walk_ = 0.0f;
  float breath_ = 0.0f;
};

} // namespace spanda
