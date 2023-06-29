

#include <map>
#include <string>
#include <string_view>

#include "android_property_manager.h"
#include "modem_log_constants.h"

namespace modem {
namespace logging {

/**
 * @brief Fake Implementation of AndroidPropertyManager that mocks some of the
 * property changing behaviour from pixellogger's `modem_logging_control`.
 */
class FakeAndroidPropertyManager : public AndroidPropertyManager {
 public:
  inline constexpr static std::string_view kTruthString = "true";
  inline constexpr static std::string_view kFalseString = "false";

  bool GetBoolProperty(const std::string& key, bool default_value) override {
    return MapContainsKey(key)
               ? GetPropertyInternal(key) == kTruthString
               : default_value;
  };

  std::string GetProperty(const std::string& key,
                          const std::string& default_value) override {
    return MapContainsKey(key) ? GetPropertyInternal(key) : default_value;
    ;
  };

  int GetIntProperty(const std::string& key, int default_value) override {
    return MapContainsKey(key) ? std::stoi(GetPropertyInternal(key))
                               : default_value;
  };

  /**
   * This function needs to copy the behaviour of `modem_logging_control` to
   * ensure that the right properties are being set in order.
   *
   * More specifically, this function will also set the
   * `kModemLoggingStatusProperty` whenever `kModemLoggingEnabledProperty` is
   * set to simulate modem logging stopping / starting.
   */
  void SetProperty(const std::string& key, const std::string& value) override {
    if (key == kModemLoggingEnabledProperty) {
      property_map_[kModemLoggingStatusProperty.data()] = value;

      // need to track if modem logging has restarted or not
      if (value == kFalseString) {
        modem_logging_has_been_off_ = true;
      }
      if (modem_logging_has_been_off_ && (value == kTruthString)) {
        modem_logging_has_restarted_ = true;
      }
    }
    property_map_[key] = value;
  };

  bool ModemLoggingHasRestarted() { return modem_logging_has_restarted_; }

 private:
  bool MapContainsKey(const std::string& key) {
    return property_map_.find(key) != property_map_.end();
  }
  std::string GetPropertyInternal(const std::string& key) {
    return property_map_.find(key)->second;
  }

  std::map<std::string, std::string> property_map_;
  bool modem_logging_has_been_off_ = false;
  bool modem_logging_has_restarted_ = false;
};
}  // namespace logging
}  // namespace modem
