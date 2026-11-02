#pragma once

#include <math.h>
#include <stdint.h>

namespace spanda {

inline float clamp_unit(float value) {
  if (value < -1.0f) return -1.0f;
  if (value > 1.0f) return 1.0f;
  return value;
}

class Core {
public:
  void Reset(uint16_t seed) {
    random_state_ = seed ? seed : 1;
    x_ = 1.0f + float(random_state_ & 0xFF) / 512.0f;
    y_ = 1.0f + float((random_state_ >> 8) & 0xFF) / 512.0f;
    z_ = 1.0f;
    phase_ = 0.0f;
  }

  uint32_t Random() {
    uint32_t value = random_state_;
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    random_state_ = value ? value : 1;
    return random_state_;
  }

  float phase() const { return phase_; }

  void Step(uint8_t model, float instability, float density, uint8_t rho_control, uint8_t evolution,
            float joystick_x, float joystick_y, float &nx, float &ny, float &nz) {
    const float speed = 0.004f + float(evolution) * 0.000018f;
    float dx, dy, dz;
    if (model != 1) {
      const float sigma = 8.0f + instability * 5.0f;
      const float rho = 20.0f + float(rho_control) * (20.0f / 255.0f) + instability * 8.0f;
      const float beta = 2.3f + density * 0.45f;
      dx = sigma * (y_ - x_);
      dy = x_ * (rho - z_) - y_;
      dz = x_ * y_ - beta * z_;
      if (model == 2) {
        const float rx = -y_ - z_;
        const float ry = x_ + (0.15f + instability * 0.12f) * y_;
        const float rz = 0.2f + density * 0.12f + z_ * (x_ - (4.5f + rho_control * 0.01f + instability * 1.5f));
        dx = (dx + rx) * 0.5f;
        dy = (dy + ry) * 0.5f;
        dz = (dz + rz) * 0.5f;
      }
    } else {
      const float a = 0.15f + instability * 0.12f;
      const float b = 0.2f + density * 0.12f;
      const float c = 5.7f + instability * 2.0f;
      dx = -y_ - z_;
      dy = x_ + a * y_;
      dz = b + z_ * (x_ - c);
    }

    dx += joystick_x * (0.6f + instability * 1.6f);
    dy += joystick_y * (0.6f + density * 1.6f);
    dz += joystick_x * joystick_y * density * 1.5f;
    x_ += dx * speed;
    y_ += dy * speed;
    z_ += dz * speed;

    if (fabsf(x_) > 60.0f || fabsf(y_) > 60.0f || fabsf(z_) > 60.0f) {
      x_ += (1.0f - x_) * 0.15f;
      y_ += (1.0f - y_) * 0.15f;
      z_ += (1.0f - z_) * 0.15f;
    }

    const bool rossler = model == 1;
    nx = clamp_unit(x_ / (rossler ? 14.0f : 24.0f));
    ny = clamp_unit(y_ / (rossler ? 14.0f : 32.0f));
    nz = clamp_unit(z_ / (rossler ? 20.0f : 48.0f));
    phase_ += 0.012f + float(evolution) * 0.00022f;
    if (phase_ > 6.2831853f) phase_ -= 6.2831853f;
  }

private:
  uint32_t random_state_ = 1;
  float x_ = 1.0f;
  float y_ = 1.0f;
  float z_ = 1.0f;
  float phase_ = 0.0f;
};

} // namespace spanda
