#pragma once

#include <math.h>
#include <stdint.h>

namespace spanda {

class FMEngine {
public:
  void Reset() { envelope_ = 0.0f; }

  float Process(float phase, float nx, float ny, float walk, float density,
                uint8_t mode, uint8_t intensity, uint8_t depth,
                uint8_t irregularity, uint8_t evolution) {
    const float slow_wave = 0.5f + 0.5f * sinf(phase * 0.12f);
    const float attack = 0.004f + float(evolution) * 0.00005f;
    envelope_ += ((0.15f + 0.85f * slow_wave) - envelope_) * attack;
    float behavior = 0.5f + 0.5f * sinf(phase * 0.45f);
    if (mode == 1) behavior = (ny * 0.65f + walk * 0.35f) * 0.5f + 0.5f;
    if (mode == 2) behavior = fabsf(nx) > 0.55f ? 1.0f : envelope_ * 0.55f;
    if (mode == 3) behavior = 0.5f + 0.25f * sinf(phase * 0.13f) + 0.25f * sinf(phase * 0.71f);
    behavior = powf(behavior, 0.55f + irregularity / 255.0f);
    return (behavior * envelope_ * (intensity / 255.0f) +
      (nx * 0.25f + walk * 0.35f) * (depth / 255.0f)) * (0.45f + density * 0.55f);
  }

private:
  float envelope_ = 0.0f;
};

} // namespace spanda
