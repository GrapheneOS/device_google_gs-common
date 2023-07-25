#pragma once

#include <string>

namespace modem {
namespace logging {

/**
 * @brief Interface for interacting with Android System Properties.
 */
class AndroidPropertyManager {
 public:
  virtual ~AndroidPropertyManager() = default;
  virtual bool GetBoolProperty(const std::string& key, bool default_value);
  virtual std::string GetProperty(const std::string& key,
                                  const std::string& default_value);
  virtual int GetIntProperty(const std::string& key, int default_value);
  virtual void SetProperty(const std::string& key, const std::string& value);
};
}  // namespace logging
}  // namespace modem
