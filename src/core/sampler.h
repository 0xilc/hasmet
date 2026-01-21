#pragma once
#include <algorithm>
#include <glm/glm.hpp>
#include <memory>
#include <numeric>
#include <random>

namespace hasmet {

class Sampler {
 public:
  static constexpr int TABLE_SIZE = 1024;

  Sampler() {
    static const int n = glm::sqrt(TABLE_SIZE);
    std::vector<glm::vec2> temp_samples;
    std::mt19937 gen(31);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < n; ++j) {
        float u = (i + dist(gen)) / n;
        float v = (j + dist(gen)) / n;
        temp_samples.push_back({u, v});
      }
    }

    std::shuffle(temp_samples.begin(), temp_samples.end(), gen);
    table_ = temp_samples;
  }

  glm::vec2 get_2d(int pixel_id, int sample_idx, int dimension) const {
    return glm::vec2(get_1d(pixel_id, sample_idx, dimension), 
                         get_1d(pixel_id, sample_idx, dimension));
  }

  float get_1d(int pixel_id, int sample_idx, int dimension) const {
    static thread_local std::mt19937 gen(std::random_device{}());
        static thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(gen);
  }

 private:
  std::vector<glm::vec2> table_;

  uint32_t hash_uint32(uint32_t x) const {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
  }
};
}  // namespace hasmet