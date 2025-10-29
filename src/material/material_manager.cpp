#include "material_manager.h"

#include "core/logging.h"

MaterialManager* MaterialManager::instance_ptr_ = nullptr;

MaterialManager* MaterialManager::get_instance() {
  if (instance_ptr_ == nullptr) {
    instance_ptr_ = new MaterialManager();
  }
  return instance_ptr_;
}

Material& MaterialManager::get(int material_id) const {
  auto it = materials_.find(material_id);
  if (it == materials_.end()) {
    throw std::runtime_error("MaterialManager::get: invalid material_id " +
                             std::to_string(material_id));
  }
  return *it->second.get();
}

int MaterialManager::add(const Material& material) {
  int new_index = materials_.size();
  materials_[new_index] = std::make_unique<Material>(material);
  return new_index;
}