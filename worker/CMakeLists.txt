cmake_minimum_required(VERSION 3.15)
project(fits_read)

set(CMAKE_CXX_STANDARD 20)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    SET(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    SET(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    SET(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
    SET(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
    INCLUDE_DIRECTORIES(/usr/local/include)
    INCLUDE_DIRECTORIES(/opt/homebrew/include)
    INCLUDE_DIRECTORIES(/usr/local/opt/openssl/include)
    INCLUDE_DIRECTORIES(/usr/local/opt/libomp/include)
    LINK_DIRECTORIES(/usr/local/lib)
    LINK_DIRECTORIES(/opt/homebrew/lib)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


FIND_PACKAGE(PkgConfig REQUIRED)
find_package(fmt)

FIND_PACKAGE(CFITSIO QUIET)
if (CFITSIO_FOUND)
    message(STATUS "Found cfitsio using find_package")
else ()
    PKG_SEARCH_MODULE(CFITSIO REQUIRED cfitsio)
    if (CFITSIO_FOUND)
        message(STATUS "Found cfitsio using pkg-config")
        include_directories(${CFITSIO_INCLUDE_DIRS})
    else ()
        message(FATAL_ERROR "Could not find cfitsio")
    endif ()
endif ()
set(LINK_LIBS
        ${LINK_LIBS}
        ${CFITSIO_LIBRARIES}
        fmt
)

add_executable(fits_read main.cpp
        ReaderService.cpp
        ReaderService.h)
target_link_libraries(fits_read
        PRIVATE
        fitsReaderProto
        ${LINK_LIBS}
)
