#include "fits_util.h"

#include <fitsio.h>
#include <fmt/format.h>

grpc::Status StatusFromFitsError(grpc::StatusCode grpc_code, int fits_code, const std::string& message) {
  char fits_message[30];
  fits_get_errstatus(fits_code, fits_message);
  return {grpc_code, fmt::format("{} (FITS error code {}: {})", message, fits_code, fits_message)};
}
