#include "clock_manager_impl.h"

#include <unistd.h>

namespace pixel_modem {

void ClockManagerImpl::Sleep(size_t seconds) const { sleep(seconds); }

}  // namespace pixel_modem
