#include "ReaderService.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/random_access_file.hpp>

#include <fitsio.h>
#include <fmt/format.h>
#include <math.h>

#include "../util/fits_util.h"

grpc::Status ReaderService::CheckStatus(grpc::ServerContext* context, const fitsReaderProto::Empty* request,
                                        fitsReaderProto::StatusResponse* response) {
  response->set_status(true);
  response->set_statusmessage("OK");
  return grpc::Status::OK;
}
grpc::Status ReaderService::OpenFile(grpc::ServerContext* context, const fitsReaderProto::FileOpenRequest* request,
                                     fitsReaderProto::StatusResponse* response) {
  fitsfile* fits_ptr = nullptr;
  int fits_status_code = 0;
  int fits_close_code = 0;

  if (request->uuid().empty() || request->filename().empty()) {
    return {grpc::StatusCode::INVALID_ARGUMENT, "UUID and filename must be specified"};
  }

  if (request->hdu().empty() || request->hdu() == "0") {
    fits_open_file(&fits_ptr, request->filename().c_str(), READONLY, &fits_status_code);
  } else {
    return {grpc::StatusCode::UNIMPLEMENTED, "HDU extension support not implemented yet"};
  }
  if (fits_status_code != 0) {
    return StatusFromFitsError(grpc::StatusCode::NOT_FOUND, fits_status_code, "Could not open file");
  }

  fits_files[request->uuid()] = fits_ptr;
  response->set_status(true);
  return grpc::Status::OK;
}

grpc::Status ReaderService::CloseFile(grpc::ServerContext* context, const fitsReaderProto::FileCloseRequest* request,
                                      fitsReaderProto::StatusResponse* response) {
  if (request->uuid().empty()) {
    return {grpc::StatusCode::INVALID_ARGUMENT, "UUID must be specified"};
  }

  if (fits_files.find(request->uuid()) == fits_files.end()) {
    return {grpc::StatusCode::NOT_FOUND, fmt::format("File with UUID {} not found", request->uuid())};
  }

  int fits_status_code = 0;
  fits_close_file(fits_files[request->uuid()], &fits_status_code);

  if (fits_status_code != 0) {
    return StatusFromFitsError(grpc::StatusCode::INTERNAL, fits_status_code, "Could not close file");
  }
  fits_files.erase(request->uuid());
  response->set_status(true);


  boost::asio::io_context io_context;

  boost::asio::random_access_file file(
      io_context, "/path/to/file",
      boost::asio::random_access_file::read_only);

  file.async_read_some_at(1234, my_buffer,
      [](error_code e, size_t n)
      {
        // ...
      });


  return grpc::Status::OK;
}

grpc::Status ReaderService::GetFileInfo(grpc::ServerContext* context, const fitsReaderProto::FileInfoRequest* request,
                                        fitsReaderProto::FileInfoResponse* response) {
  if (request->uuid().empty()) {
    return {grpc::StatusCode::INVALID_ARGUMENT, "UUID must be specified"};
  }

  auto it = fits_files.find(request->uuid());
  if (it == fits_files.end()) {
    return {grpc::StatusCode::NOT_FOUND, fmt::format("File with UUID {} not found", request->uuid())};
  }
  fitsfile* fits_ptr = it->second;

  int fits_status_code = 0;

  int hdu_num = 0;
  fits_get_hdu_num(fits_ptr, &hdu_num);
  if (hdu_num <= 0) {
    return StatusFromFitsError(grpc::StatusCode::INVALID_ARGUMENT, fits_status_code, "Could not get HDU number");
  }
  response->set_hdunum(hdu_num);

  int hdu_type = 0;
  fits_get_hdu_type(fits_ptr, &hdu_type, &fits_status_code);
  if (fits_status_code != 0) {
    return StatusFromFitsError(grpc::StatusCode::INVALID_ARGUMENT, fits_status_code, "Could not get HDU type");
  }
  response->set_hdutype(hdu_type);

  // Only image HDUs can have dimensions and data types
  if (hdu_type == IMAGE_HDU) {
    int num_dims = 0;
    int max_dims = 64;
    std::vector<long> dims(max_dims);
    int bit_pix = 0;
    fits_get_img_param(fits_ptr, max_dims, &bit_pix, &num_dims, dims.data(), &fits_status_code);
    if (fits_status_code != 0) {
      return StatusFromFitsError(grpc::StatusCode::INVALID_ARGUMENT, fits_status_code, "Could not get image parameters");
    }

    dims.resize(num_dims);
    response->mutable_hdushape()->Add(dims.begin(), dims.end());
    response->set_datatype(bit_pix);
  }

  return grpc::Status::OK;
}

grpc::Status ReaderService::GetImageData(grpc::ServerContext* context, const fitsReaderProto::ImageDataRequest* request,
                                         fitsReaderProto::ImageDataResponse* response) {
  if (request->uuid().empty()) {
    return {grpc::StatusCode::INVALID_ARGUMENT, "UUID must be specified"};
  }

  auto it = fits_files.find(request->uuid());
  if (it == fits_files.end()) {
    return {grpc::StatusCode::NOT_FOUND, fmt::format("File with UUID {} not found", request->uuid())};
  }
  fitsfile* fits_ptr = it->second;

  int fits_status_code = 0;
  int fits_close_code = 0;

  int num_pixels = request->numpixels();
  // TODO: support other data types!
  int num_bytes = num_pixels * sizeof(float);
  response->mutable_data()->resize(num_bytes);

  std::vector<long> start_pix(request->start().begin(), request->start().end());
  fits_read_pix(fits_ptr, TFLOAT, start_pix.data(), num_pixels, nullptr, response->mutable_data()->data(), nullptr, &fits_status_code);

  if (fits_status_code != 0) {
    return StatusFromFitsError(grpc::StatusCode::INTERNAL, fits_status_code, "Could not read image data");
  }

  return grpc::Status::OK;
}

grpc::Status ReaderService::GetSpectralProfile(grpc::ServerContext* context, const fitsReaderProto::SpectralProfileRequest* request,
                                               fitsReaderProto::SpectralProfileResponse* response) {
  if (request->uuid().empty()) {
    return {grpc::StatusCode::INVALID_ARGUMENT, "UUID must be specified"};
  }

  auto width = std::max(request->width(), 1);
  auto height = std::max(request->height(), 1);

  auto it = fits_files.find(request->uuid());
  if (it == fits_files.end()) {
    return {grpc::StatusCode::NOT_FOUND, fmt::format("File with UUID {} not found", request->uuid())};
  }
  fitsfile* fits_ptr = it->second;
  int fits_status_code = 0;

  const auto num_pixels = request->numpixels();
  // TODO: support other data types!
  const auto num_bytes = num_pixels * sizeof(float);
  response->mutable_data()->resize(num_bytes);

  if (width <= 1 && height <= 1) {
    std::vector<long> start_pix = {request->x(), request->y(), request->z(), 1};
    std::vector<long> last_pix = {request->x() + width - 1, request->y() + height - 1, request->z() + request->numpixels() - 1, 1};
    std::vector<long> increment = {1, 1, 1, 1};
    fits_read_subset(fits_ptr, TFLOAT, start_pix.data(), last_pix.data(), increment.data(), nullptr, response->mutable_data()->data(),
                     nullptr, &fits_status_code);
    if (fits_status_code != 0) {
      return StatusFromFitsError(grpc::StatusCode::INTERNAL, fits_status_code, "Could not read image data");
    }
  } else if (request->buffered()) {
    int num_dims = 0;
    int max_dims = 2;
    std::vector<long> dims(max_dims);
    int bit_pix = 0;
    fits_get_img_param(fits_ptr, max_dims, &bit_pix, &num_dims, dims.data(), &fits_status_code);
    if (fits_status_code != 0) {
      return StatusFromFitsError(grpc::StatusCode::INVALID_ARGUMENT, fits_status_code, "Could not get image parameters");
    }
    const auto required_buffer_size = dims[0] * (height - 1) + width;
    std::vector<float> required_buffer(required_buffer_size);
    auto* data_ptr = reinterpret_cast<float*>(response->mutable_data()->data());

    for (auto i = 0; i < request->numpixels(); i++) {
      const auto channel = request->z() + i;

      std::vector<long> start_pix = {request->x(), request->y(), channel, 1};
      std::vector<long> last_pix = {request->x() + width - 1, request->y() + height - 1, channel, 1};
      std::vector<long> increment = {1, 1, 1, 1};
      fits_read_pix(fits_ptr, TFLOAT, start_pix.data(), required_buffer_size, nullptr, required_buffer.data(), nullptr, &fits_status_code);
      if (fits_status_code != 0) {
        return StatusFromFitsError(grpc::StatusCode::INTERNAL, fits_status_code, "Could not read image data");
      }

      int count = 0;
      float sum = 0;
      long offset = 0;

      for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
          const auto value = required_buffer[offset + col];
          if (std::isfinite(value)) {
            sum += value;
            count++;
          }
        }
        offset += dims[0];
      }

      const float channel_mean = count > 0 ? sum / count : NAN;
      data_ptr[i] = channel_mean;
    }
  } else {
    const auto slice_size_pixels = width * height;
    std::vector<float> channel_buffer(slice_size_pixels);
    auto* data_ptr = reinterpret_cast<float*>(response->mutable_data()->data());

    for (auto i = 0; i < request->numpixels(); i++) {
      const auto channel = request->z() + i;

      std::vector<long> start_pix = {request->x(), request->y(), channel, 1};
      std::vector<long> last_pix = {request->x() + width - 1, request->y() + height - 1, channel, 1};
      std::vector<long> increment = {1, 1, 1, 1};

      fits_read_subset(fits_ptr, TFLOAT, start_pix.data(), last_pix.data(), increment.data(), nullptr, channel_buffer.data(),
      nullptr,
                       &fits_status_code);
      if (fits_status_code != 0) {
        return StatusFromFitsError(grpc::StatusCode::INTERNAL, fits_status_code, "Could not read image data");
      }

      int count = 0;
      float sum = 0;
      for (const auto& value : channel_buffer) {
        if (std::isfinite(value)) {
          sum += value;
          count++;
        }
      }

      const float channel_mean = count > 0 ? sum / count : NAN;
      data_ptr[i] = channel_mean;
    }
  }

  return grpc::Status::OK;
}
