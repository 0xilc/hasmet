#include "texture_manager.h"
#include "core/logging.h"

namespace hasmet {
TextureManager* TextureManager::instance_ptr_ = nullptr;

TextureManager* TextureManager::get_instance() {
  if (instance_ptr_ == nullptr) {
    instance_ptr_ = new TextureManager();
  }
  return instance_ptr_;
}

Texture& TextureManager::get(int texture_id) const {
  auto it = textures_.find(texture_id);
  if (it == textures_.end()) {
    throw std::runtime_error("TextureManager::get: invalid texture_id " +
                             std::to_string(texture_id));
  }
  return *it->second.get();
}

int TextureManager::add(int texture_id, const Texture& texture) {
  textures_[texture_id] = std::make_unique<Texture>(texture);
  return texture_id;
}
} // namespace hasmet