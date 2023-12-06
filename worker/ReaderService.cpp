#include "ReaderService.h"

#include <iostream>


grpc::Status ReaderService::CheckStatus(grpc::ServerContext* context, const fitsReaderProto::Empty* request,
                                        fitsReaderProto::StatusResponse* response) {
    std::cout << "Server: CheckStatus called" << std::endl;
    response->set_status(true);
    response->set_statusmessage("OK");
    return grpc::Status::OK;
}

grpc::Status ReaderService::GetFileInfo(grpc::ServerContext* context, const fitsReaderProto::FileInfoRequest* request,
                                        fitsReaderProto::FileInfoResponse* response) {
    response->set_filename(request->filename());
    return grpc::Status::OK;
}
