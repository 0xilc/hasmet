#include "image_io.h"

#include <vector>
#include "core/logging.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"


namespace hasmet {
bool write_png(const std::string& filename, const std::vector<Color>& pixels,
               int width, int height) {
  std::vector<unsigned char> image_data;
  image_data.reserve(width * height * 3);

  for (const auto& pixel : pixels) {
    Color clamped_pixel = glm::clamp(pixel, 0.0f, 255.0f);

    image_data.push_back(
        static_cast<unsigned char>(clamped_pixel.r));
    image_data.push_back(
        static_cast<unsigned char>(clamped_pixel.g));
    image_data.push_back(
        static_cast<unsigned char>(clamped_pixel.b));
  }

  int result = stbi_write_png(filename.c_str(), width, height, 3,
                              image_data.data(), width * 3);

  return (result != 0);
}

bool write_exr(const std::string& filename, const std::vector<Color>& pixels,
               int width, int height) {
    const float* rgb_data = reinterpret_cast<const float*>(pixels.data());
    const char* err = nullptr;

    int result = SaveEXR(rgb_data, width, height, 3, 0, filename.c_str(), &err);
    if (result != TINYEXR_SUCCESS) {
        if (err) {
            LOG_ERROR("TINYEXR Error: " << err);
            FreeEXRErrorMessage(err);
        }
        return false;
    }
    return true;
}
} // namespace hasmet