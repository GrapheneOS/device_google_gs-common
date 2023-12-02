#pragma once

#include <map>
#include <string>

#include "android-base/result.h"
#include "android_property_manager.h"

namespace pixel_modem {

/**
 * @brief Fake Implementation of AndroidPropertyManager that mocks some of the
 * property changing behaviour from pixellogger's `modem_logging_control`.
 */
class FakeAndroidPropertyManager : public AndroidPropertyManager {
 public:
  bool GetBoolProperty(const std::string& key, bool default_value) override;

  std::string GetProperty(const std::string& key,
                          const std::string& default_value) override;

  int GetIntProperty(const std::string& key, int default_value) override;

  /**
   * This function needs to copy the behaviour of `modem_logging_control` to
   * ensure that the right properties are being set in order.
   *
   * More specifically, this function will also set the
   * `kModemLoggingStatusProperty` whenever `kModemLoggingEnabledProperty` is
   * set to simulate modem logging stopping / starting.
   */
  bool SetProperty(const std::string& key, const std::string& value) override;

  inline bool ModemLoggingHasRestarted() {
    return modem_logging_has_restarted_;
  }

 private:
  /**
   * @brief Gets android system property if present.
   *
   * @param[in] key Name of property.
   *
   * @return Status of get operation and value if successful.
   * @retval EINVAL Key not present in map.
   */
  android::base::Result<std::string> GetProperty(const std::string& key);

  std::map<std::string, std::string> property_map_;
  bool modem_logging_has_been_off_ = false;
  bool modem_logging_has_restarted_ = false;
};

}  // namespace pixel_modem
