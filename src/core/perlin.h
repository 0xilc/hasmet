#pragma once

#include "core/types.h"
#include <vector>
#include <cmath>

namespace hasmet {

class Perlin {
public:
    static Perlin* get_instance();

    float noise(const Vec3& p) const;
    float turb(const Vec3& p, int depth = 7) const;
private:
    Perlin();
    static const int point_count = 256;
    std::vector<Vec3> ranvec;
    std::vector<int> perm_x;
    std::vector<int> perm_y;
    std::vector<int> perm_z;

    static std::vector<int> perlin_generate_perm();
    static void permute(std::vector<int>& p, int n);
    static float trilinear_interp(float c[2][2][2], float u, float v, float w);
    
    static Perlin* instance_;
};

} // namespace hasmet
