
#pragma once

#include <string>
#include <string_view>

namespace pixel_modem {

// Used to set boolean parameters to true / false
inline constexpr std::string_view kTruthString = "true";
inline constexpr std::string_view kFalseString = "false";

/**
 * @brief Interface for interacting with Android System Properties.
 */
class AndroidPropertyManager {
 public:
  virtual ~AndroidPropertyManager() = default;
  virtual bool GetBoolProperty(const std::string& key, bool default_value) = 0;
  virtual std::string GetProperty(const std::string& key,
                                  const std::string& default_value) = 0;
  virtual int GetIntProperty(const std::string& key, int default_value) = 0;
  virtual bool SetProperty(const std::string& key,
                           const std::string& value) = 0;
};

}  // namespace pixel_modem
