#include "android_property_manager_impl.h"

#include <android-base/properties.h>

#include <string>

namespace pixel_modem {

bool AndroidPropertyManagerImpl::GetBoolProperty(const std::string& key,
                                                 bool default_value) {
  return android::base::GetBoolProperty(key, default_value);
}

std::string AndroidPropertyManagerImpl::GetProperty(
    const std::string& key, const std::string& default_value) {
  return android::base::GetProperty(key, default_value);
}

int AndroidPropertyManagerImpl::GetIntProperty(const std::string& key,
                                               int default_value) {
  return android::base::GetIntProperty(key, default_value);
}

bool AndroidPropertyManagerImpl::SetProperty(const std::string& key,
                                             const std::string& value) {
  return android::base::SetProperty(key, value);
}

}  // namespace pixel_modem
