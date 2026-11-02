#pragma once

#include <math.h>
#include <stdint.h>

namespace spanda {

inline float Ratio(float phase, float ny, float walk, float density,
                   uint8_t mode, uint8_t range, uint8_t feedback,
                   float previous_ratio, float previous_fold) {
  const float ratio_phase = phase * (0.025f + range * 0.00012f) +
    previous_ratio * feedback / 255.0f;
  float value;
  if (mode == 0) {
    value = 0.5f + 0.5f * sinf(ratio_phase);
  } else if (mode == 1) {
    value = 0.5f + 0.35f * ny + walk * 0.15f;
  } else {
    float fract = ratio_phase / 6.2831853f;
    fract -= floorf(fract);
    value = 1.0f - 2.0f * fabsf(2.0f * fract - 1.0f);
    value = copysignf(powf(fabsf(value), 0.65f + density * 0.8f), value);
  }
  value += previous_fold * feedback / 512.0f;
  return value < -1.0f ? -1.0f : value > 1.0f ? 1.0f : value;
}

} // namespace spanda
