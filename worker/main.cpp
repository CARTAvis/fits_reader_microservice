#include <vector>
#include <chrono>
#include <cstdio>
#include <fstream>

#include <fmt/core.h>
#include <fitsio.h>

inline float big2little(float f) {
    union {
        float f;
        char b[4];
    } src, dst;

    src.f = f;
    dst.b[3] = src.b[0];
    dst.b[2] = src.b[1];
    dst.b[1] = src.b[2];
    dst.b[0] = src.b[3];
    return dst.f;
}

int main(int argc, char **argv) {
    int status = 0;
    fitsfile *fptr;
    int dims = 4;
    std::vector<long> imageSize(dims);
    std::vector<float> data_vector;
    float *data_ptr = nullptr;

    std::vector<long> startPix(dims);
    for (int i = 0; i < dims; i++) {
        startPix[i] = 1;
    }

    if (fits_is_reentrant()) {
        fmt::print("cfitsio compiled with thread safety\n");
    } else {
        fmt::print("cfitsio compiled without thread safety\n");
    }

    // Open image and get data size. File is assumed to be 2D
    bool use_custom_read = argc > 2 && argv[2][0] == '1';

    fits_open_file(&fptr, argv[1], READONLY, &status);
    fits_get_img_size(fptr, dims, imageSize.data(), &status);
    const int64_t num_pixels = imageSize[0] * imageSize[1] * (imageSize[2] ? imageSize[2] : 1) * (
            imageSize[3] ? imageSize[3] : 1);
    float test_val = 0;

    // Frame::FillImageCache timing is just the memory allocation and reading part
    auto t_start = std::chrono::high_resolution_clock::now();
    data_ptr = new float[num_pixels];

    if (use_custom_read) {
        float scale = 1.0001f;
        float offset = 0.00001f;
        int64_t data_start;
        fits_get_hduaddrll(fptr, nullptr, &data_start, nullptr, &status);

        auto file_ptr = fopen(argv[1], "rb");
        fseek(file_ptr, data_start, SEEK_SET);
        fread(data_ptr, sizeof(float) * num_pixels, 1, file_ptr);

        for (auto i = 0; i < num_pixels; i++) {
            data_ptr[i] = big2little(data_ptr[i]) * scale + offset;
        }
        fclose(file_ptr);

        test_val = data_ptr[num_pixels / 2];
    } else {
        fits_read_pix(fptr, TFLOAT, startPix.data(), num_pixels, nullptr, data_ptr, nullptr, &status);
        test_val = data_ptr[num_pixels / 2];
    }
    auto t_end = std::chrono::high_resolution_clock::now();

    auto dt = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();
    auto rate = num_pixels / (double) dt;
    auto rate_bytes = num_pixels * sizeof(float) / (double) dt;
    fmt::print("{:.1f} MB file read using {} in {:.1f} ms at {:.1f} MPix/s ({:.1f} MB/s). Test val={}\n",
               num_pixels * sizeof(float) * 1.0e-6,
               use_custom_read ? "custom read" : "fits_read_pix",
               dt * 1.0e-3, rate, rate_bytes, test_val);

    fits_close_file(fptr, &status);
    delete[] data_ptr;

    return status;
}
