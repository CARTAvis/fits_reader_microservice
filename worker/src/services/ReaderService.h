#ifndef READERSERVICE_H
#define READERSERVICE_H

#include <fitsReaderProto/file_service.grpc.pb.h>
#include <fitsReaderProto/file_service.pb.h>
#include <fitsio.h>

class ReaderService final : public fitsReaderProto::FileService::Service {
 protected:
  std::unordered_map<std::string, fitsfile*> fits_files;

 public:
  virtual ::grpc::Status CheckStatus(::grpc::ServerContext* context, const ::fitsReaderProto::Empty* request,
                                     ::fitsReaderProto::StatusResponse* response) override;
  virtual ::grpc::Status OpenFile(grpc::ServerContext* context, const fitsReaderProto::FileOpenRequest* request,
                                  fitsReaderProto::StatusResponse* response) override;
  virtual grpc::Status CloseFile(grpc::ServerContext* context, const fitsReaderProto::FileCloseRequest* request,
                                 fitsReaderProto::StatusResponse* response) override;

  virtual ::grpc::Status GetFileInfo(::grpc::ServerContext* context, const ::fitsReaderProto::FileInfoRequest* request,
                                     ::fitsReaderProto::FileInfoResponse* response) override;
  virtual ::grpc::Status GetImageData(::grpc::ServerContext* context, const ::fitsReaderProto::ImageDataRequest* request,
                                      ::fitsReaderProto::ImageDataResponse* response) override;
  virtual ::grpc::Status GetSpectralProfile(grpc::ServerContext* context, const fitsReaderProto::SpectralProfileRequest* request,
                                            fitsReaderProto::SpectralProfileResponse* response) override;
};

#endif  // READERSERVICE_H
