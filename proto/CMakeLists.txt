cmake_minimum_required(VERSION 3.15)
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(PROTO_FILES
        fitsReaderProto/file_service.proto
        ../controller/src/utils/uuid.cpp
        ../controller/src/utils/uuid.h
)

# Add Library target with protobuf sources
add_library(fitsReaderProto ${PROTO_FILES})
target_link_libraries(fitsReaderProto
        PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(fitsReaderProto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET fitsReaderProto LANGUAGE cpp)
protobuf_generate(TARGET fitsReaderProto LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")