#pragma once
#include "core/types.h"
#include <vector>
#include <cmath>

namespace hasmet {

class Perlin {
public:
    static Perlin& get_instance();

    static float fade(float t);
    float noise(const Vec3& p) const;
    float turb(const Vec3& p, int depth = 7) const;

    Perlin(const Perlin&) = delete;
    void operator=(const Perlin&) = delete;

private:
    Perlin();
    
    static const Vec3 gradients[16];
    
    std::vector<int> perm_x;
    std::vector<int> perm_y;
    std::vector<int> perm_z;

    static std::vector<int> perlin_generate_perm();
    static void permute(std::vector<int>& p, int n);
    static float grad(int hash, float x, float y, float z);
    static float trilinear_interp(float c[2][2][2], float u, float v, float w);
};

} // namespace hasmet