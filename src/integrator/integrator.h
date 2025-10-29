#pragma once

class Scene;
class Film;
class Camera;

class Integrator {
 public:
  virtual ~Integrator() = default;
  virtual void render(const Scene& scene, Film& film, const Camera& camera) const = 0;
};