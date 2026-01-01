#include "image_manager.h"
#include "core/logging.h"
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "tinyexr.h" 

namespace hasmet {

ImageManager* ImageManager::instance_ptr_ = nullptr;

ImageManager* ImageManager::get_instance() {
  if (instance_ptr_ == nullptr) {
    instance_ptr_ = new ImageManager();
  }
  return instance_ptr_;
}

const Image& ImageManager::get(int image_id) const {
  auto it = images_.find(image_id);
  if (it == images_.end()) {
    throw std::runtime_error("ImageManager::get: Image ID not found: " +
                             std::to_string(image_id));
  }
  return *it->second;
}

bool ImageManager::load_image(int image_id, const std::string& filename) {
  int width, height, channels;
  float* data = nullptr;
  std::string ext = filename.substr(filename.find_last_of(".") + 1);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  // Read EXR
  if (ext == "exr") {
    const char* err = nullptr;
    int result = LoadEXR(&data, &width, &height, filename.c_str(), &err);

    if (result != TINYEXR_SUCCESS) {
      if (err) {
        LOG_ERROR("TINYEXR Error: " << err << "(" << filename << ")");
        FreeEXRErrorMessage(err);
      }
      return false;
    }
    channels = 4;
  } 
  // Read other formats
  else {
    stbi_ldr_to_hdr_gamma(2.2f);
    data = stbi_loadf(filename.c_str(), &width, &height, &channels, 3);
    if (!data) {
      LOG_ERROR("Failed to load image: " << filename);
      return false;
    }
    channels = 3;
  }

  // Register image
  auto new_image = std::make_unique<Image>(width, height, channels, data);
  images_[image_id] = std::move(new_image);

  // Deallocate image buffer
  if (ext == "exr") {
    free(data);
  }
  else {
    stbi_image_free(data);
  }

  LOG_INFO("Loaded image ID " << image_id << ": " << filename << " (" << width << "x" << height << ")");
  return true;
}

} // namespace hasmet