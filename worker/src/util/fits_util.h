#ifndef FITS_UTIL_H
#define FITS_UTIL_H

#include <string>
#include <grpcpp/support/status.h>


grpc::Status StatusFromFitsError(grpc::StatusCode grpc_code, int fits_code, const std::string& message);

#endif //FITS_UTIL_H
