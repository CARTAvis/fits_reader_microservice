#include "ReaderService.h"

#include <fitsio.h>

#include "../util/fits_util.h"

grpc::Status ReaderService::CheckStatus(grpc::ServerContext* context, const fitsReaderProto::Empty* request,
                                        fitsReaderProto::StatusResponse* response) {
  response->set_status(true);
  response->set_statusmessage("OK");
  return grpc::Status::OK;
}

grpc::Status ReaderService::GetFileInfo(grpc::ServerContext* context, const fitsReaderProto::FileInfoRequest* request,
                                        fitsReaderProto::FileInfoResponse* response) {
  response->set_filename(request->filename());

  fitsfile* fits_ptr = nullptr;
  int fits_status_code = 0;
  int fits_close_code = 0;

  if (request->hdu().empty() || request->hdu() == "0") {
    fits_open_file(&fits_ptr, request->filename().c_str(), READONLY, &fits_status_code);
  } else {
    return {grpc::StatusCode::UNIMPLEMENTED, "HDU extension support not implemented yet"};
  }
  if (fits_status_code != 0) {
    return StatusFromFitsError(grpc::StatusCode::NOT_FOUND, fits_status_code, "Could not open file");
  }

  int hdu_num = 0;
  fits_get_hdu_num(fits_ptr, &hdu_num);
  if (hdu_num <= 0) {
    fits_close_file(fits_ptr, &fits_close_code);
    return StatusFromFitsError(grpc::StatusCode::INVALID_ARGUMENT, fits_status_code, "Could not get HDU number");
  }
  response->set_hdunum(hdu_num);

  int hdu_type = 0;
  fits_get_hdu_type(fits_ptr, &hdu_type, &fits_status_code);
  if (fits_status_code != 0) {
    fits_close_file(fits_ptr, &fits_close_code);
    return StatusFromFitsError(grpc::StatusCode::INVALID_ARGUMENT, fits_status_code, "Could not get HDU type");
  }
  response->set_hdutype(hdu_type);

  // Only image HDUs can have dimensions and data types
  if (hdu_type == IMAGE_HDU) {
    int num_dims = 0;
    fits_get_img_dim(fits_ptr, &num_dims, &fits_status_code);
    if (fits_status_code != 0) {
      fits_close_file(fits_ptr, &fits_close_code);
      return StatusFromFitsError(grpc::StatusCode::INVALID_ARGUMENT, fits_status_code, "Could not get image dimensions");
    }

    std::vector<long> dims(num_dims);
    fits_get_img_size(fits_ptr, num_dims, dims.data(), &fits_status_code);
    if (fits_status_code != 0) {
      fits_close_file(fits_ptr, &fits_close_code);
      return StatusFromFitsError(grpc::StatusCode::INVALID_ARGUMENT, fits_status_code, "Could not get image dimensions");
    }
    response->mutable_hdushape()->Add(dims.begin(), dims.end());

    int image_type = 0;
    fits_get_img_type(fits_ptr, &image_type, &fits_status_code);
    if (fits_status_code != 0) {
      fits_close_file(fits_ptr, &fits_close_code);
      return StatusFromFitsError(grpc::StatusCode::INVALID_ARGUMENT, fits_status_code, "Could not get image type");
    }
    response->set_datatype(image_type);
  }

  fits_close_file(fits_ptr, &fits_close_code);
  return grpc::Status::OK;
}
