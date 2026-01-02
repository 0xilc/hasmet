#include "image_io.h"

#include <vector>
#include <filesystem>
#include "core/logging.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

namespace hasmet {
namespace {
void create_directory_if_missing(const std::string& filepath)
{
    std::filesystem::path output_path(filepath);

    if (!output_path.parent_path().empty()) {
    std::filesystem::create_directories(output_path.parent_path());
    }
}
} // namespace

bool write_png(const std::string& filename, const std::vector<Color>& pixels,
               int width, int height) {
  create_directory_if_missing(filename);
  std::vector<unsigned char> image_data;
  image_data.reserve(width * height * 3);

  for (const auto& pixel : pixels) {
    unsigned char r = static_cast<unsigned char>(glm::clamp(pixel.r * 255.0f, 0.0f, 255.0f));
    unsigned char g = static_cast<unsigned char>(glm::clamp(pixel.g * 255.0f, 0.0f, 255.0f));
    unsigned char b = static_cast<unsigned char>(glm::clamp(pixel.b * 255.0f, 0.0f, 255.0f));

    image_data.push_back(r);
    image_data.push_back(g);
    image_data.push_back(b);
  }

  int result = stbi_write_png(filename.c_str(), width, height, 3,
                              image_data.data(), width * 3);

  return (result != 0);
}

bool write_exr(const std::string& filename, const std::vector<Color>& pixels,
               int width, int height) {
    create_directory_if_missing(filename);
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