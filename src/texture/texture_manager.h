#pragma once

#include <map>
#include <memory>
#include "texture.h"

namespace hasmet {
class TextureManager {
 private:
  TextureManager() {};
  std::map<int, std::unique_ptr<Texture>> textures_;
  static TextureManager* instance_ptr_;

 public:
  TextureManager(TextureManager& other) = delete;
  void operator=(const TextureManager&) = delete;

  static TextureManager* get_instance();
  Texture& get(int texture_id) const;
  int add(int texture_id, const Texture& texture);
};
} // namespace hasmet