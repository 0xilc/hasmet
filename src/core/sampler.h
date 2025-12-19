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
    uint32_t hash = hash_uint32(pixel_id) ^ hash_uint32(dimension * 31692);
    int index = (hash + sample_idx) % TABLE_SIZE;
    return table_[index];
  }

  float get_1d(int pixel_id, int sample_idx, int dimension) const {
    return get_2d(pixel_id, sample_idx, dimension).x;
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