#pragma once

class Scene;
class Film;

class Integrator {
 public:
  virtual ~Integrator() = default;
  virtual void render(const Scene& scene, Film& film) const = 0;
};