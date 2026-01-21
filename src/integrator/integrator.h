#pragma once
#include "core/sampler.h"
#include "core/medium.h"

namespace hasmet {
class Scene;
class Film;
class Camera;

struct SamplingContext {
  Sampler &sampler;
  int pixel_id;
  int sample_index;
  int num_samples;
};

struct PathState {
  int depth;
  const Medium* current_medium;

  PathState(int d) : depth(d), current_medium(nullptr) {}
};

class Integrator {
 public:
  virtual ~Integrator() = default;
  virtual void render(const Scene& scene, Film& film, const Camera& camera) const = 0;
};
} // namespace hasmet