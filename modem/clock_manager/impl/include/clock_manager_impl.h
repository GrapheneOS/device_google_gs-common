#pragma once

#include <cstddef>

#include "clock_manager.h"

namespace pixel_modem {

struct ClockManagerImpl : public ClockManager {
  void Sleep(size_t seconds) const override;
};

}  // namespace pixel_modem
