#include "image_manager.h"
#include "core/logging.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" 

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
  unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 3);

  if (!data) {
    LOG_ERROR("Failed to load image: " << filename);
    return false;
  }

  auto new_image = std::make_unique<Image>(width, height, 3, data);
  images_[image_id] = std::move(new_image);

  stbi_image_free(data);
  
  LOG_INFO("Loaded image ID " << image_id << ": " << filename << " (" << width << "x" << height << ")");
  return true;
}

} // namespace hasmet