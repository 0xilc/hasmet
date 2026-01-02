#pragma once

#include <memory>
#include <vector>

#include "light/ambient_light.h"
#include "light/point_light.h"
#include "light/area_light.h"
#include "light/directional_light.h"
#include "material/material.h"
#include "accelerator/bvh.h"
#include "camera/pinhole.h"
#include "geometry/plane.h"
#include "camera/thinlens.h"
#include "core/types.h"
#include "accelerator/instance.h"

namespace hasmet {
struct RenderContext{
  Color background_color;
  float shadow_eps;
  float intersection_eps;
  int max_recursion_depth;
  int num_samples;
};

class Scene {
 public:
  Scene();

  bool intersect(Ray& r, HitRecord& rec) const;
  bool is_occluded(const Ray& r) const;

  void add_shape(Instance shape);
  void add_point_light(std::unique_ptr<PointLight> light);
  void add_ambient_light(std::unique_ptr<AmbientLight> light);
  void build_bvh();

  BVH<Instance> bvh_;
  std::vector<Instance> objects_;
  std::vector<Instance> planes_;
  std::vector<std::unique_ptr<PointLight>> point_lights_;
  std::vector<std::unique_ptr<AreaLight>> area_lights_;
  std::vector<std::unique_ptr<DirectionalLight>> directional_lights_;
  std::vector<std::unique_ptr<Camera>> cameras_;
  std::unique_ptr<AmbientLight> ambient_light_;
  RenderContext render_context_;
};

} // namespace hasmet