#pragma once

#include <unordered_map>
#include "material.h"

class MaterialManager {
 private:
  MaterialManager() {};
  std::unordered_map<int, std::unique_ptr<Material>> materials_;
  static MaterialManager* instance_ptr_;

 public:
  MaterialManager(MaterialManager& other) = delete;
  void operator=(const MaterialManager&) = delete;

  static MaterialManager* get_instance();
  Material* get(int material_id) const;
  Material* add(const Material& material);

};

MaterialManager* MaterialManager::instance_ptr_ = nullptr;