#include "clock_manager.h"

namespace pixel_modem {

/**
 * @brief Fake implementation of clock manager that doesn't actually sleep.
 *
 * A lot of vendor code don't have return values and instead force the client
 * codes to sleep for a specified period of time before checking some system
 * properties. However, since unit tests don't rely on the real vendor
 * implementations, these sleeps should be ignored and so a fake clock should be
 * used.
 *
 * Since this definition is unlikely to change, it will be defined in the header
 * and not an implementation file.
 */
struct FakeClockManager : public ClockManager {
  void Sleep(size_t /*seconds*/) const override{};
};

}  // namespace pixel_modem
