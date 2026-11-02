#pragma once

#include "../OC_apps.h"
#include "../src/drivers/display.h"
#include <math.h>
#include "spanda/spanda_core.h"
#include "spanda/spanda_fm.h"
#include "spanda/spanda_ratio.h"
#include "spanda/spanda_spectral.h"
#include "spanda/spanda_gesture.h"
#include "spanda/spanda_io.h"
#include "spanda/spanda_ui.h"

OC_APP_CLASS(AppSpanda, TWOCCS("SP"), "Spanda", "Nonlinear CV Engine") {
public:
  OC_APP_INTERFACE_DECLARE(AppSpanda, 30);
  uint32_t screensaver_timeout_override_ms() const final { return 40000; }
  uint32_t screensaver_redraw_interval_ms() const final { return 83; }
  bool left_long_press_opens_screensaver() const final { return false; }

private:
  using Page = spanda::Page;
  using GestureState = spanda::Gesture::State;
  static constexpr Page CORE = spanda::CORE;
  static constexpr Page HOME = spanda::HOME;
  static constexpr Page FM = spanda::FM;
  static constexpr Page RATIO = spanda::RATIO;
  static constexpr Page SPECTRAL = spanda::SPECTRAL;
  static constexpr Page GESTURE = spanda::GESTURE;
  static constexpr int PAGE_COUNT = spanda::PAGE_COUNT;
  static constexpr GestureState GESTURE_IDLE = spanda::Gesture::GESTURE_IDLE;
  static constexpr GestureState GESTURE_RECORD = spanda::Gesture::GESTURE_RECORD;
  static constexpr GestureState GESTURE_PLAY = spanda::Gesture::GESTURE_PLAY;
  static constexpr GestureState GESTURE_ATTRACT = spanda::Gesture::GESTURE_ATTRACT;
  static constexpr int32_t kFiveVoltsPitch = 5 * 12 * 128;

  uint8_t chaos_ = 72;
  uint8_t rho_ = 102;
  uint8_t complexity_ = 112;
  uint8_t evolution_ = 96;
  uint8_t memory_ = 160;
  uint8_t fm_mode_ = 0;
  uint8_t fm_intensity_ = 160;
  uint8_t fm_depth_ = 160;
  uint8_t fm_irregularity_ = 72;
  uint8_t ratio_mode_ = 0;
  uint8_t ratio_range_ = 160;
  uint8_t ratio_feedback_ = 48;
  uint8_t fold_amount_ = 112;
  uint8_t shape_amount_ = 128;
  uint8_t breath_ = 144;
  uint8_t attractor_ = 0;
  uint8_t personality_ = 0;
  uint8_t gesture_length_ = 1;
  uint8_t mutation_ = 24;
  uint16_t seed_ = 0x4D31;

  Page page_ = CORE;
  uint8_t field_ = 0;
  bool hold_ = false;
  int8_t calibrating_axis_ = -1;
  spanda::Gesture gesture_;
  spanda::AxisCalibration axis_[2];
  spanda::Core core_;
  spanda::FMEngine fm_engine_;
  spanda::Spectral spectral_engine_;
  uint16_t control_divider_ = 0;
  uint16_t engine_divider_ = 0;
  uint8_t update_stride_ = 4;
  float previous_[4]{};
  float output_[4]{};
  float gesture_x_ = 0.0f;
  float gesture_y_ = 0.0f;
  float live_joystick_x_ = 0.0f;
  float live_joystick_y_ = 0.0f;
  volatile int8_t orbit_x_[64]{};
  volatile int8_t orbit_y_[64]{};
  volatile int8_t joystick_x_trail_[64]{};
  volatile int8_t joystick_y_trail_[64]{};
  volatile uint8_t orbit_pos_ = 0;
  volatile uint8_t orbit_count_ = 0;
  uint16_t burn_in_timer_ = 0;
  int8_t burn_in_shift_ = -1;
  mutable uint8_t applied_contrast_ = 0xFF;

  void ResetCore();
  void UpdateEngine(float joystick_x, float joystick_y, float external_chaos, float external_complexity);
  int FieldCount() const;
  const char *PageName() const;
  const char *FieldName(int field) const;
  const char *FieldOption(int field) const;
  int FieldValue(int field) const;
  void DrawFieldValue(int field) const;
  void AdjustField(int field, int delta);
  void StartGestureRecording();
  void ToggleAxisCalibration(int axis);
  void DrawHome() const;
  void DrawEditPage() const;
  void DrawAttractor(bool ritual) const;
  void ApplyContrast(uint8_t contrast) const;
  static float ClampUnit(float value);
};

FLASHMEM void AppSpanda::Init() {
  page_ = HOME;
  field_ = 0;
  hold_ = false;
  gesture_.Reset();
  calibrating_axis_ = -1;
  control_divider_ = 0;
  engine_divider_ = 0;
  live_joystick_x_ = live_joystick_y_ = 0.0f;
  axis_[0].Reset();
  axis_[1].Reset();
  memset(previous_, 0, sizeof(previous_));
  memset(output_, 0, sizeof(output_));
  orbit_pos_ = orbit_count_ = 0;
  burn_in_timer_ = 0;
  burn_in_shift_ = -1;
  for (int i = 0; i < 64; ++i) {
    orbit_x_[i] = orbit_y_[i] = 0;
    joystick_x_trail_[i] = joystick_y_trail_[i] = 0;
  }
  personality_ = 0;
  applied_contrast_ = 0xFF;
  ResetCore();
}

void AppSpanda::ResetCore() {
  core_.Reset(seed_);
  fm_engine_.Reset();
  spectral_engine_.Reset();
}

float AppSpanda::ClampUnit(float value) {
  if (value < -1.0f) return -1.0f;
  if (value > 1.0f) return 1.0f;
  return value;
}

FLASHMEM size_t AppSpanda::SaveAppData(util::StreamBufferWriter &stream_buffer) const {
  stream_buffer.Write(chaos_);
  stream_buffer.Write(complexity_);
  stream_buffer.Write(evolution_);
  stream_buffer.Write(memory_);
  stream_buffer.Write(fm_mode_);
  stream_buffer.Write(fm_intensity_);
  stream_buffer.Write(fm_depth_);
  stream_buffer.Write(fm_irregularity_);
  stream_buffer.Write(ratio_mode_);
  stream_buffer.Write(ratio_range_);
  stream_buffer.Write(ratio_feedback_);
  stream_buffer.Write(fold_amount_);
  stream_buffer.Write(shape_amount_);
  stream_buffer.Write(breath_);
  stream_buffer.Write(attractor_);
  stream_buffer.Write(gesture_length_);
  stream_buffer.Write(mutation_);
  stream_buffer.Write(seed_);
  stream_buffer.Write(axis_[0].minimum());
  stream_buffer.Write(axis_[0].maximum());
  stream_buffer.Write(axis_[1].minimum());
  stream_buffer.Write(axis_[1].maximum());
  const uint8_t invert = (axis_[0].inverted() ? 1 : 0) | (axis_[1].inverted() ? 2 : 0);
  stream_buffer.Write(invert);
  stream_buffer.Write(personality_);
  stream_buffer.Write(rho_);
  return stream_buffer.written();
}

FLASHMEM size_t AppSpanda::RestoreAppData(util::StreamBufferReader &stream_buffer) {
  stream_buffer.Read(chaos_);
  stream_buffer.Read(complexity_);
  stream_buffer.Read(evolution_);
  stream_buffer.Read(memory_);
  stream_buffer.Read(fm_mode_);
  stream_buffer.Read(fm_intensity_);
  stream_buffer.Read(fm_depth_);
  stream_buffer.Read(fm_irregularity_);
  stream_buffer.Read(ratio_mode_);
  stream_buffer.Read(ratio_range_);
  stream_buffer.Read(ratio_feedback_);
  stream_buffer.Read(fold_amount_);
  stream_buffer.Read(shape_amount_);
  stream_buffer.Read(breath_);
  stream_buffer.Read(attractor_);
  stream_buffer.Read(gesture_length_);
  stream_buffer.Read(mutation_);
  stream_buffer.Read(seed_);
  int16_t minimum[2], maximum[2];
  uint8_t invert = 0;
  stream_buffer.Read(minimum[0]);
  stream_buffer.Read(maximum[0]);
  stream_buffer.Read(minimum[1]);
  stream_buffer.Read(maximum[1]);
  stream_buffer.Read(invert);
  stream_buffer.Read(personality_);
  stream_buffer.Read(rho_);
  fm_mode_ &= 3;
  ratio_mode_ %= 3;
  attractor_ %= 3;
  personality_ &= 1;
  if (!rho_) rho_ = 102;
  gesture_length_ = constrain(gesture_length_, 0, 2);
  axis_[0].Restore(minimum[0], maximum[0], invert & 1);
  axis_[1].Restore(minimum[1], maximum[1], invert & 2);
  ResetCore();
  return stream_buffer.read();
}

void AppSpanda::HandleAppEvent(OC::AppEvent event) {
  if (event == OC::APP_EVENT_RESUME) {
    hold_ = false;
    gesture_.set_state(GESTURE_IDLE);
    ApplyContrast(0x0CF);
  } else if (event == OC::APP_EVENT_SUSPEND) {
    ApplyContrast(0x0CF);
  }
}

void AppSpanda::StartGestureRecording() {
  gesture_.Start();
}

void AppSpanda::ToggleAxisCalibration(int axis) {
  if (calibrating_axis_ == axis && axis_[axis].active()) {
    axis_[axis].Finish();
    calibrating_axis_ = -1;
    return;
  }

  calibrating_axis_ = axis;
  axis_[axis].Begin();
}

void FASTRUN AppSpanda::Process(OC::IOFrame *ioframe) {
  if (!hold_ && ioframe->digital_inputs.triggered<OC::DIGITAL_INPUT_1>())
    control_divider_ = 0;
  if (!hold_ && ioframe->digital_inputs.triggered<OC::DIGITAL_INPUT_2>()) {
    ++seed_;
    ResetCore();
  }

  if (!hold_ && ++control_divider_ >= 16) {
    control_divider_ = 0;
    if (++burn_in_timer_ >= 30000) {
      burn_in_timer_ = 0;
      burn_in_shift_ = burn_in_shift_ >= 1 ? -1 : burn_in_shift_ + 1;
    }
    const int raw_x = ioframe->cv.values[0];
    const int raw_y = ioframe->cv.values[1];
    if (calibrating_axis_ == 0) {
      axis_[0].Observe(raw_x);
    } else if (calibrating_axis_ == 1) {
      axis_[1].Observe(raw_y);
    }
    const float joystick_x = axis_[0].Normalize(raw_x);
    const float joystick_y = axis_[1].Normalize(raw_y);
    live_joystick_x_ = joystick_x;
    live_joystick_y_ = joystick_y;
    const float external_chaos = ClampUnit(float(ioframe->cv.values[2]) / 2048.0f);
    const float external_complexity = ClampUnit(float(ioframe->cv.values[3]) / 2048.0f);

    gesture_.Capture(joystick_x, joystick_y, gesture_length_);

    float x = joystick_x;
    float y = joystick_y;
    if (gesture_.state() == GESTURE_PLAY || gesture_.state() == GESTURE_ATTRACT) {
      const float gx = gesture_.Axis(0);
      const float gy = gesture_.Axis(1);
      const float jitter = float(int32_t(core_.Random() & 0xFFFF) - 32768) / 32768.0f;
      if (gesture_.state() == GESTURE_PLAY) {
        x = gx + jitter * mutation_ / 1020.0f;
        y = gy + float(int32_t(core_.Random() & 0xFFFF) - 32768) / 32768.0f * mutation_ / 1020.0f;
      } else {
        gesture_x_ += (gx - gesture_x_) * 0.04f;
        gesture_y_ += (gy - gesture_y_) * 0.04f;
        x = joystick_x * 0.35f + gesture_x_ * 0.65f + jitter * mutation_ / 2040.0f;
        y = joystick_y * 0.35f + gesture_y_ * 0.65f;
      }
      gesture_.Advance();
    }

    if (ioframe->digital_inputs.triggered<OC::DIGITAL_INPUT_3>())
      StartGestureRecording();

    if (++engine_divider_ >= update_stride_) {
      engine_divider_ = 0;
      UpdateEngine(ClampUnit(x), ClampUnit(y), external_chaos, external_complexity);
    }
  }

  for (int output = 0; output < 4; ++output)
    ioframe->outputs.set_pitch_value(output, static_cast<int32_t>(output_[output] * kFiveVoltsPitch));
}

void AppSpanda::UpdateEngine(float joystick_x, float joystick_y, float external_chaos, float external_complexity) {
  const float chaos = ClampUnit(float(chaos_) / 127.5f - 1.0f + external_chaos * 0.35f);
  const float complexity = ClampUnit(float(complexity_) / 127.5f - 1.0f + external_complexity * 0.35f);
  const float instability = ClampUnit((chaos + joystick_x + 2.0f) * 0.25f);
  const float density = ClampUnit((complexity + joystick_y + 2.0f) * 0.25f);
  float nx, ny, nz;
  core_.Step(attractor_, instability, density, rho_, evolution_, joystick_x, joystick_y,
             nx, ny, nz);
  const float phase = core_.phase();
  const float random_value = float(int32_t(core_.Random() & 0xFFFF) - 32768) / 32768.0f;
  const float fold = spectral_engine_.Process(phase, nx, density, previous_[0],
    random_value, fm_irregularity_, fold_amount_, shape_amount_, breath_);
  const float fm = fm_engine_.Process(phase, nx, ny, spectral_engine_.walk(), density,
    fm_mode_, fm_intensity_, fm_depth_, fm_irregularity_, evolution_);
  const float ratio = spanda::Ratio(phase, ny, spectral_engine_.walk(), density,
    ratio_mode_, ratio_range_, ratio_feedback_, previous_[1], previous_[2]);
  const float shape = spanda::Spectral::Morph(ny, nz, previous_[2], shape_amount_,
    gesture_y_, mutation_);
  float targets[4] = {
    ClampUnit(fm * 2.0f - 0.5f),
    ClampUnit(ratio),
    ClampUnit(fold * 0.5f + 0.5f),
    ClampUnit(shape * 0.5f + 0.5f)
  };

  const float slew = 0.02f + (255 - memory_) * 0.0025f;
  for (int output = 0; output < 4; ++output) {
    output_[output] += (targets[output] - output_[output]) * slew;
    previous_[output] = output_[output] * 2.0f - 1.0f;
  }
  const uint8_t trail_index = orbit_pos_;
  const float plot_x = attractor_ == 2 ? (previous_[0] + previous_[1]) * 0.5f : nx;
  const float plot_y = attractor_ == 2 ? (previous_[2] + previous_[3]) * 0.5f : ny;
  orbit_x_[trail_index] = static_cast<int8_t>(ClampUnit(plot_x) * 127.0f);
  orbit_y_[trail_index] = static_cast<int8_t>(ClampUnit(plot_y) * 127.0f);
  joystick_x_trail_[trail_index] = static_cast<int8_t>(joystick_x * 127.0f);
  joystick_y_trail_[trail_index] = static_cast<int8_t>(joystick_y * 127.0f);
  orbit_pos_ = (trail_index + 1) & 63;
  if (orbit_count_ < 64) ++orbit_count_;
  update_stride_ = static_cast<uint8_t>(1 + (255 - evolution_) / 32);
}

int AppSpanda::FieldCount() const {
  if (page_ == HOME) return 0;
  return page_ == CORE ? 5 : 4;
}

const char *AppSpanda::PageName() const {
  return spanda::PageName(page_);
}

const char *AppSpanda::FieldName(int field) const {
  return spanda::FieldName(page_, field);
}

const char *AppSpanda::FieldOption(int field) const {
  static const char *attractors[] = {"LORENZ", "ROSSLER", "PHASE"};
  static const char *fm_modes[] = {"ORGANIC", "UNST", "AGGR", "FRACTAL"};
  static const char *ratio_modes[] = {"DRIFT", "CHAOS", "FOLD"};
  static const char *gesture_lengths[] = {"10s", "20s", "60s"};
  if (page_ == CORE && field == 0) return attractors[attractor_];
  if (page_ == CORE && (field == 3 || field == 4)) {
    if (axis_[field - 3].active()) return "CAL";
    return axis_[field - 3].inverted() ? "INV" : "NORM";
  }
  if (page_ == FM && field == 0) return fm_modes[fm_mode_];
  if (page_ == RATIO && field == 0) return ratio_modes[ratio_mode_];
  if (page_ == GESTURE && field == 0) return gesture_lengths[gesture_length_];
  if (page_ == GESTURE && field == 2) {
    static const char *gesture_states[] = {"IDLE", "REC", "PLAY", "ATTR"};
    return gesture_states[gesture_.state()];
  }
  return nullptr;
}

int AppSpanda::FieldValue(int field) const {
  switch (page_) {
    case CORE:
      if (field == 0) return attractor_;
      if (field == 1) return rho_;
      if (field == 2) return chaos_;
      return axis_[field - 3].inverted();
    case FM:
      if (field == 0) return fm_mode_;
      if (field == 1) return fm_intensity_;
      if (field == 2) return fm_depth_;
      return complexity_;
    case RATIO:
      if (field == 0) return ratio_mode_;
      if (field == 1) return ratio_range_;
      if (field == 2) return ratio_feedback_;
      return evolution_;
    case SPECTRAL:
      if (field == 0) return fold_amount_;
      if (field == 1) return shape_amount_;
      if (field == 2) return breath_;
      return memory_;
    default:
      if (field == 0) return gesture_length_;
      if (field == 1) return mutation_;
      if (field == 2) return gesture_.state();
      return memory_;
  }
}

void AppSpanda::AdjustField(int field, int delta) {
  switch (page_) {
    case CORE:
      if (field == 0) {
        attractor_ = (attractor_ + delta + 3) % 3;
        orbit_count_ = 0;
        burn_in_shift_ = -1;
        ResetCore();
      }
      else if (field == 1) rho_ = constrain(int(rho_) + delta, 1, 255);
      else if (field == 2) chaos_ = constrain(int(chaos_) + delta, 0, 255);
      else if (delta) axis_[field - 3].ToggleInvert();
      break;
    case FM:
      if (field == 0) fm_mode_ = (fm_mode_ + delta + 4) % 4;
      else if (field == 1) fm_intensity_ = constrain(int(fm_intensity_) + delta, 0, 255);
      else if (field == 2) fm_depth_ = constrain(int(fm_depth_) + delta, 0, 255);
      else complexity_ = constrain(int(complexity_) + delta, 0, 255);
      break;
    case RATIO:
      if (field == 0) ratio_mode_ = (ratio_mode_ + delta + 3) % 3;
      else if (field == 1) ratio_range_ = constrain(int(ratio_range_) + delta, 0, 255);
      else if (field == 2) ratio_feedback_ = constrain(int(ratio_feedback_) + delta, 0, 255);
      else evolution_ = constrain(int(evolution_) + delta, 0, 255);
      break;
    case SPECTRAL:
      if (field == 0) fold_amount_ = constrain(int(fold_amount_) + delta, 0, 255);
      else if (field == 1) shape_amount_ = constrain(int(shape_amount_) + delta, 0, 255);
      else if (field == 2) breath_ = constrain(int(breath_) + delta, 0, 255);
      else memory_ = constrain(int(memory_) + delta, 0, 255);
      break;
    case GESTURE:
      if (field == 0) gesture_length_ = constrain(int(gesture_length_) + delta, 0, 2);
      else if (field == 1) mutation_ = constrain(int(mutation_) + delta, 0, 255);
      else if (field == 2) gesture_.set_state(static_cast<GestureState>((gesture_.state() + delta + 4) % 4));
      else memory_ = constrain(int(memory_) + delta, 0, 255);
      break;
    default: break;
  }
}

FLASHMEM void AppSpanda::HandleButtonEvent(const UI::Event &event) {
  if (event.type == UI::EVENT_BUTTON_LONG_PRESS && event.control == OC::CONTROL_BUTTON_L) {
    page_ = HOME;
    field_ = 0;
    return;
  }
  if (event.type == UI::EVENT_BUTTON_LONG_PRESS &&
      event.control == OC::CONTROL_BUTTON_B && page_ == CORE && field_ >= 3) {
    ToggleAxisCalibration(field_ - 3);
    return;
  }
  if (event.type != UI::EVENT_BUTTON_PRESS) return;
  if (event.control == OC::CONTROL_BUTTON_A) {
    hold_ = !hold_;
  } else if (event.control == OC::CONTROL_BUTTON_B) {
    if (page_ == CORE && field_ >= 3) {
      axis_[field_ - 3].ToggleInvert();
    } else if (gesture_.state() == GESTURE_RECORD)
      gesture_.set_state(gesture_.size() ? GESTURE_PLAY : GESTURE_IDLE);
    else if (gesture_.state() == GESTURE_IDLE)
      StartGestureRecording();
    else if (gesture_.state() == GESTURE_PLAY)
      gesture_.set_state(GESTURE_ATTRACT);
    else
      gesture_.set_state(GESTURE_PLAY);
  } else if (event.control == OC::CONTROL_BUTTON_L) {
    if (page_ != HOME && FieldCount()) field_ = (field_ + 1) % FieldCount();
  } else if (event.control == OC::CONTROL_BUTTON_R) {
    if (page_ == HOME) personality_ ^= 1;
    else if (FieldOption(field_)) AdjustField(field_, 1);
  }
}

FLASHMEM void AppSpanda::HandleEncoderEvent(const UI::Event &event) {
  if (event.control == OC::CONTROL_ENCODER_L) {
    page_ = static_cast<Page>((page_ + event.value + PAGE_COUNT) % PAGE_COUNT);
    field_ = 0;
  } else if (event.control == OC::CONTROL_ENCODER_R && page_ != HOME) {
    AdjustField(field_, event.value);
  }
}

void AppSpanda::ApplyContrast(uint8_t contrast) const {
  if (applied_contrast_ != contrast) {
    display::SetContrast(contrast);
    applied_contrast_ = contrast;
  }
}

void AppSpanda::DrawMenu() const {
  ApplyContrast(OC::ui.idle_time() >= 20000 ? 48 : 0x0CF);
  if (page_ == HOME) {
    DrawHome();
    return;
  }
  DrawEditPage();
}

void AppSpanda::DrawHome() const {
  if (personality_) {
    DrawAttractor(true);
    return;
  }

  static const char *models[] = {"LORENZ", "ROSSLER", "PHASE"};
  graphics.setPrintPos(1, 8);
  graphics.print("SPANDA");
  graphics.setPrintPos(52, 8);
  graphics.print(models[attractor_]);
  graphics.setPrintPos(100, 8);
  graphics.print("INSTR");
  graphics.setPrintPos(1, 17);
  graphics.print("CORE ON E");
  graphics.print((evolution_ * 100) / 255);
  graphics.print(" M");
  graphics.print((memory_ * 100) / 255);
  graphics.print(" H");
  graphics.print(hold_ ? "ON" : "OFF");

  graphics.setPrintPos(1, 25);
  graphics.print("C");
  graphics.print((chaos_ * 100) / 255);
  graphics.print("% X");
  int x = static_cast<int>(live_joystick_x_ * 100.0f);
  graphics.print(x >= 0 ? '+' : '-');
  x = abs(x);
  graphics.print(x / 100);
  graphics.print('.');
  graphics.print(x % 100, 2);
  graphics.print(" Y");
  int y = static_cast<int>(live_joystick_y_ * 100.0f);
  graphics.print(y >= 0 ? '+' : '-');
  y = abs(y);
  graphics.print(y / 100);
  graphics.print('.');
  graphics.print(y % 100, 2);

  static const char *labels[] = {"A FM", "B RATIO", "C FOLD", "D SHAPE"};
  for (int i = 0; i < 4; ++i) {
    const int row_y = 32 + i * 8;
    graphics.setPrintPos(1, row_y);
    graphics.print(labels[i]);
    const float level = i == 0 ? (output_[i] + 1.0f) * 0.5f : output_[i];
    const int width = constrain(static_cast<int>(level * 82.0f), 0, 82);
    graphics.drawFrame(43, row_y, 83, 7);
    if (width) graphics.drawRect(44, row_y + 1, width, 5);
  }
}

void AppSpanda::DrawEditPage() const {
  graphics.setPrintPos(1, 8);
  graphics.print(PageName());
  graphics.setPrintPos(91, 8);
  graphics.print(static_cast<int>(page_));
  graphics.print("/5");
  graphics.setPrintPos(120, 8);
  graphics.print(hold_ ? "H" : "-");
  graphics.drawHLine(0, 12, 128);

  graphics.setPrintPos(2, 20);
  graphics.print(">");
  graphics.print(FieldName(field_));
  graphics.setPrintPos(2, 31);
  DrawFieldValue(field_);

  int row = 0;
  for (int field = 0; field < FieldCount(); ++field) {
    if (field == field_) continue;
    const int y = page_ == CORE ? 35 + row++ * 7 : 39 + row++ * 8;
    graphics.setPrintPos(3, y);
    graphics.print(FieldName(field));
    graphics.setPrintPos(94, y);
    DrawFieldValue(field);
  }
}

void AppSpanda::DrawAttractor(bool ritual) const {
  if (!ritual) {
    graphics.setPrintPos(1, 8);
    graphics.print("SPANDA");
    graphics.setPrintPos(65, 8);
    if (attractor_ == 0) graphics.print("LORENZ");
    else if (attractor_ == 1) graphics.print("ROSSLER");
    else graphics.print("PHASE SPACE");
    graphics.setPrintPos(1, 16);
    graphics.print(hold_ ? "CORE HOLD" : "CORE ACTIVE");
  }

  const int shift = burn_in_shift_;
  const int top = ritual ? 14 : 20;
  const int center_y = (top + 63) / 2;
  const int range_y = (63 - top) / 2 - 1;
  const uint8_t count = orbit_count_;
  const uint8_t start = (orbit_pos_ - count) & 63;
  int last_x = 64, last_y = center_y;
  for (int i = 0; i < count; ++i) {
    const uint8_t index = (start + i) & 63;
    const int px = 64 + (int(orbit_x_[index]) * 55) / 127 + shift;
    const int py = center_y - (int(orbit_y_[index]) * range_y) / 127;
    const int age = count - i;
    const bool draw_segment = age <= 20 || (age <= 42 ? !(i & 1) : !(i & 3));
    if (i && draw_segment) graphics.drawLine(last_x, last_y, px, py);
    if (!ritual && !(i & 1)) {
      const int jx = 64 + (int(joystick_x_trail_[index]) * 55) / 127 + shift;
      const int jy = center_y - (int(joystick_y_trail_[index]) * range_y) / 127;
      if (age <= 20 || !(i & 3)) graphics.drawRect(jx, jy, 1, 1);
    }
    last_x = px;
    last_y = py;
  }

  if (count) graphics.drawRect(last_x - 1, last_y - 1, 3, 3);
  if (ritual) {
    graphics.setPrintPos(1, 8);
    graphics.print("TANTRA");
    graphics.drawCircle(64, center_y, 2 + static_cast<int>(core_.phase() * 2.0f) % 3);
  }
}

void AppSpanda::DrawScreensaver() const {
  ApplyContrast(24);
  DrawAttractor(personality_ != 0);
}

void AppSpanda::GetIOConfig(OC::IOConfig &ioconfig) const {
  ioconfig.digital_inputs[0].set("Clock sync");
  ioconfig.digital_inputs[1].set("New seed");
  ioconfig.digital_inputs[2].set("Gesture record");
  ioconfig.digital_inputs[3].set("Unused");
  ioconfig.cv[0].set("Joystick X");
  ioconfig.cv[1].set("Joystick Y");
  ioconfig.cv[2].set("Chaos CV");
  ioconfig.cv[3].set("Complexity CV");
  ioconfig.outputs[0].set("INT FM", OC::OUTPUT_MODE_PITCH);
  ioconfig.outputs[1].set("RATIO", OC::OUTPUT_MODE_PITCH);
  ioconfig.outputs[2].set("FOLD CV", OC::OUTPUT_MODE_PITCH);
  ioconfig.outputs[3].set("SHAPE 1", OC::OUTPUT_MODE_PITCH);
}

void AppSpanda::Loop() {
}

void AppSpanda::DrawDebugInfo() const {
}

void AppSpanda::DrawFieldValue(int field) const {
  const char *option = FieldOption(field);
  if (option) {
    graphics.print(option);
  } else if (page_ == CORE && field == 1) {
    graphics.print(20 + (rho_ * 20) / 255);
    graphics.print(".0");
  } else {
    graphics.print((FieldValue(field) * 100) / 255);
    graphics.print("%");
  }
}