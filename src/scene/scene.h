#pragma once

#include <memory>
#include <vector>

#include "light/ambient_light.h"
#include "light/point_light.h"
#include "light/area_light.h"
#include "material/material.h"
#include "accelerator/bvh.h"
#include "camera/pinhole.h"
#include "geometry/plane.h"
#include "camera/thinlens.h"

struct RenderConfig{
  Color background_color;
  float shadow_ray_epsilon;
  float intersection_test_epsilon;
  int max_recursion_depth;
};

class Scene {
 public:
  Scene();

  bool intersect(Ray& r, HitRecord& rec) const;
  bool is_occluded(const Ray& r) const;

  void add_shape(std::unique_ptr<Hittable> shape);
  void add_point_light(std::unique_ptr<PointLight> light);
  void add_ambient_light(std::unique_ptr<AmbientLight> light);
  void build_bvh();

  BVH bvh_;
  std::vector<std::shared_ptr<Plane>> planes_;
  std::vector<std::shared_ptr<Hittable>> objects_;
  std::vector<std::unique_ptr<PointLight>> point_lights_;
  std::vector<std::unique_ptr<AreaLight>> area_lights_;
  std::vector<std::unique_ptr<ThinLensCamera>> cameras_;
  std::unique_ptr<AmbientLight> ambient_light_;
  RenderConfig render_config_;
};