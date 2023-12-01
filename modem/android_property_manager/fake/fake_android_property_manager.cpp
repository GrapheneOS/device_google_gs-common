#include "fake_android_property_manager.h"

#include <android-base/parseint.h>
#include <android-base/result.h>

#include <cerrno>
#include <map>
#include <string>

#include "modem_log_constants.h"

namespace pixel_modem {

bool FakeAndroidPropertyManager::GetBoolProperty(const std::string& key,
                                                 bool default_value) {
  auto value_result = GetProperty(key);
  return value_result.ok() ? (*value_result) == kTruthString : default_value;
}

std::string FakeAndroidPropertyManager::GetProperty(
    const std::string& key, const std::string& default_value) {
  auto value_result = GetProperty(key);
  return value_result.ok() ? *value_result : default_value;
}

int FakeAndroidPropertyManager::GetIntProperty(const std::string& key,
                                               int default_value) {
  int value = default_value;

  auto property_result = GetProperty(key);
  if (property_result.ok()) {
    android::base::ParseInt<int>((*property_result).data(), &value);
  }
  return value;
}

/**
 * This function needs to copy the behaviour of `modem_logging_control` to
 * ensure that the right properties are being set in order.
 *
 * More specifically, this function will also set the
 * `kModemLoggingStatusProperty` whenever `kModemLoggingEnabledProperty` is
 * set to simulate modem logging stopping / starting.
 */
bool FakeAndroidPropertyManager::SetProperty(const std::string& key,
                                             const std::string& value) {
  if (key == logging::kModemLoggingEnabledProperty) {
    property_map_[logging::kModemLoggingStatusProperty.data()] = value;

    // need to track if modem logging has restarted or not
    if (value == kFalseString) {
      modem_logging_has_been_off_ = true;
    }
    if (modem_logging_has_been_off_ && (value == kTruthString)) {
      modem_logging_has_restarted_ = true;
    }
  }
  property_map_[key] = value;
  return true;
}

/**
 * @brief Gets android system property if present.
 *
 * @param[in] key Name of property.
 *
 * @return Status of get operation and value if successful.
 * @retval EINVAL Key not present in map.
 */
android::base::Result<std::string> FakeAndroidPropertyManager::GetProperty(
    const std::string& key) {
  const auto it = property_map_.find(key);
  if (it == property_map_.end()) {
    return android::base::Error()
           << "Property: " << key << " not found." << EINVAL;
  }
  return it->second;
}

}  // namespace pixel_modem
