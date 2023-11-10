#pragma once

#include <cstddef>

namespace pixel_modem {

/**
 * @brief Interface for time based operations.
 *
 * This interface was intentionally not called `Clock`, like the Java side
 * counterpart since it's likely that clients would call the local variable
 * `clock(_)`, which would clash with the C defined `clock` method.
 */
struct ClockManager {
  virtual ~ClockManager() = default;

  /**
   * @brief Sleep the thread for a given number of seconds.
   *
   * @param seconds Minimum number of seconds to sleep for. Note, this is
   * different than the Java android clock which accepts seconds. This was done
   * because C++ developers are likely more familiar with the `sleep` command,
   * which accepts seconds.
   */
  virtual void Sleep(size_t seconds) const = 0;
};

}  // namespace pixel_modem
