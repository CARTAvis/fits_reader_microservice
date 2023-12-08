#ifndef READERSERVICE_H
#define READERSERVICE_H

#include <fitsReaderProto/file_service.grpc.pb.h>
#include <fitsReaderProto/file_service.pb.h>

class ReaderService final : public fitsReaderProto::FileService::Service {
 public:
  virtual ::grpc::Status CheckStatus(::grpc::ServerContext* context, const ::fitsReaderProto::Empty* request,
                                     ::fitsReaderProto::StatusResponse* response);
  virtual ::grpc::Status GetFileInfo(::grpc::ServerContext* context, const ::fitsReaderProto::FileInfoRequest* request,
                                     ::fitsReaderProto::FileInfoResponse* response);
};

#endif  // READERSERVICE_H
