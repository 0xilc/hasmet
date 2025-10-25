#include "image_io.h"
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

bool write_png(const std::string& filename, const std::vector<Spectrum>& pixels,
               int width, int height) {
  std::vector<unsigned char> image_data;
  image_data.reserve(width * height * 3);

  for (const auto& pixel : pixels){
    Spectrum clamped_pixel = pixel;
    clamped_pixel.clamp();
    
    image_data.push_back(
        static_cast<unsigned char>(clamped_pixel.r * 255.999f));
    image_data.push_back(
        static_cast<unsigned char>(clamped_pixel.g * 255.999f));
    image_data.push_back(
        static_cast<unsigned char>(clamped_pixel.b * 255.999f));
    
    int result = stbi_write_png(filename.c_str(), width, height, 3,
                                image_data.data(), width * 3);
    
    return (result != 0);
  }
}