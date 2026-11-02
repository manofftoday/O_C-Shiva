#pragma once

#include <stdint.h>

namespace spanda {

enum Page : uint8_t { HOME, CORE, FM, RATIO, SPECTRAL, GESTURE, PAGE_COUNT };

inline const char *PageName(Page page) {
  static const char *names[] = {"HOME", "CORE", "FM", "RATIO", "SPECTRAL", "GESTURE"};
  return names[page];
}

inline const char *FieldName(Page page, int field) {
  static const char *core[] = {"MODEL / VIEW", "RHO", "CHAOS", "X AXIS", "Y AXIS"};
  static const char *fm[] = {"BEHAVIOR", "INTENSITY", "DEPTH", "COMPLEXITY"};
  static const char *ratio[] = {"ALGORITHM", "RANGE", "FEEDBACK", "EVOLUTION"};
  static const char *spectral[] = {"FOLD", "SHAPE", "BREATH", "MEMORY"};
  static const char *gesture[] = {"LENGTH", "MUTATION", "STATE", "MEMORY"};
  switch (page) {
    case HOME: return "HOME";
    case CORE: return core[field];
    case FM: return fm[field];
    case RATIO: return ratio[field];
    case SPECTRAL: return spectral[field];
    default: return gesture[field];
  }
}

} // namespace spanda
