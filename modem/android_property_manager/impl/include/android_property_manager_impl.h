#pragma once

#include <string>

#include "android_property_manager.h"

namespace pixel_modem {

/**
 * @brief Implementation of AndroidPropertyManager that directly forwards to
 * android base methods.
 */
class AndroidPropertyManagerImpl : public AndroidPropertyManager {
 public:
  bool GetBoolProperty(const std::string& key, bool default_value) override;

  std::string GetProperty(const std::string& key,
                          const std::string& default_value) override;

  int GetIntProperty(const std::string& key, int default_value) override;

  bool SetProperty(const std::string& key, const std::string& value) override;
};

}  // namespace pixel_modem
