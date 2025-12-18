#pragma once
#include <random>

namespace hasmet::Sampling{

inline float _generate_random_float(float start, float end) {
  static thread_local std::mt19937 generator(std::random_device{}());
  std::uniform_real_distribution<float> distribution(start, end);
  return distribution(generator);
}

inline std::vector<std::pair<float, float>> generate_jittered_samples(int num_samples) {
  int n = std::sqrt(num_samples);
  std::vector<std::pair<float, float>> samples(num_samples);
  int i = 0;

  // Fill the stratified grid
  for (int y = 0; y < n; y++) {
    for (int x = 0; x < n; x++) {
      float psi1 = _generate_random_float(0, 1);
      float psi2 = _generate_random_float(0, 1);
      samples[i].first = (x + psi1) / n;
      samples[i].second = (y + psi2) / n;
      i++;
    }
  }

  // Fill the remaining samples with pure random noise
  while (i < num_samples) {
    samples[i].first = _generate_random_float(0, 1);
    samples[i].second = _generate_random_float(0, 1);
    i++;
  }

  return samples;
}

} // namespace hasmet::Sampling