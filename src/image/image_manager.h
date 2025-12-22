#pragma once

#include <map>
#include <memory>
#include <string>
#include "image.h"

namespace hasmet {

class ImageManager {
 private:
  ImageManager() {};
  std::map<int, std::unique_ptr<Image>> images_;
  static ImageManager* instance_ptr_;

 public:
  ImageManager(ImageManager& other) = delete;
  void operator=(const ImageManager&) = delete;

  static ImageManager* get_instance();
  
  const Image& get(int image_id) const;

  bool load_image(int image_id, const std::string& filename);
};

} // namespace hasmet