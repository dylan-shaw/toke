#include "exceptions.h"

#include <stdexcept>

namespace toke {

void
throw_if_error(const toke_error_z err)
{
  if (err == TOKE_ERROR_NONE) {
    return;
  }

  throw std::runtime_error(toke_strerror(err));
}

} // namespace toke
