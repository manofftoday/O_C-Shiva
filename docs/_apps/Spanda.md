---
title: Spanda
---

# Spanda

Spanda is a joystick-driven nonlinear CV modulation engine. It couples a
Lorenz, Rossler, or coupled phase-space attractor to four continuous-voltage
outputs. It does not quantize, select scales, or generate pitch sequences. It
controls Shapeshifter parameters at control rate, not at audio rate.

| Output | Destination | Engine |
| --- | --- | --- |
| OUT 1 | INT FM | FM Dynamics |
| OUT 2 | RATIO | Nonlinear Ratio |
| OUT 3 | FOLD CV | Spectral Distortion |
| OUT 4 | SHAPE 1 | Chaotic Morphing |

CV1 and CV2 are independently calibrated joystick X/Y axes. On the Core page,
select X or Y; turn the right encoder or tap B to invert that axis. Hold B,
move the selected axis through its full range, then hold B again to save its
calibration. Core also exposes attractor/view, RHO, and CHAOS. CV3 and CV4 add
external chaos and complexity modulation. Joystick X/Y directly force the
attractor equations. TR1 synchronizes the control phase, TR2 advances the seed,
and TR3 starts gesture capture.

The interface has HOME plus five edit pages: Core, FM, Ratio, Spectral, and
Gesture. The left encoder changes pages. On edit pages, its press selects a
field, the right encoder changes the value, and the right encoder press
confirms or cycles discrete options. A long press of the left encoder returns
to HOME. Button A toggles HOLD; button B captures, stops, or alternates gesture
playback and attractor-following. On HOME, the right encoder press switches
between Instrument and Ritual presentation.

Instrument HOME summarizes the selected model, chaos, joystick position,
evolution, memory, HOLD state, and four output meters. The Core page exposes
Lorenz, Rossler, or coupled Phase Space selection plus RHO, CHAOS, and the two
axis calibration controls.

Gestures are held in RAM and capture up to 10, 20, or 60 seconds at 10 samples
per second and linearly interpolate between samples. They are not retained
across power cycles. The seed and settings are saved, but reproducing a
trajectory also requires the same joystick/CV inputs, button actions, and
trigger sequence. Random mutation is deterministic for the same seed and event
history. HOLD freezes the core, output state, and attractor visualization.

After 20 seconds without interaction, the OLED contrast dims. After 40 seconds,
Spanda enters its attractor screensaver at about 12 FPS. Select Lorenz, Rossler,
or Phase Space from Core. Phase Space plots the combined four-motor trajectory;
the joystick path is dotted alongside it. Screensaver drawing does not pause
or modify CV generation. Ritual uses the same engine and shows only the
attractor with a slowly pulsing center; Instrument keeps status information
visible. The plot shifts occasionally to reduce OLED image retention.

## Firmware components

The engine is divided into focused headers under `software/src/apps/spanda/`:

* `spanda_core.h` - attractor integration, direct joystick forcing, and seeded PRNG.
* `spanda_fm.h` - FM envelope, behavior modes, depth, and irregularity.
* `spanda_ratio.h` - drift, chaotic, and nonlinear folding ratio trajectories.
* `spanda_spectral.h` - smoothed random walk, spectral breathing, folding, and morph.
* `spanda_gesture.h` - capture buffer, interpolation, playback, and attract mode.
* `spanda_io.h` - independent joystick-axis range calibration and inversion.
* `spanda_ui.h` - page model and parameter labels; `Spanda.h` connects controls and rendering.