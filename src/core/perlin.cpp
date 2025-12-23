#include "core/perlin.h"
#include "core/sampling.h"
#include <numeric>
#include <cmath>

namespace hasmet {

const Vec3 Perlin::gradients[16] = {
    Vec3(1, 1, 0), Vec3(-1, 1, 0), Vec3(1, -1, 0), Vec3(-1, -1, 0),
    Vec3(1, 0, 1), Vec3(-1, 0, 1), Vec3(1, 0, -1), Vec3(-1, 0, -1),
    Vec3(0, 1, 1), Vec3(0, -1, 1), Vec3(0, 1, -1), Vec3(0, -1, -1),
    Vec3(1, 1, 0), Vec3(0, -1, 1), Vec3(-1, 1, 0), Vec3(0, -1, -1)
};

Perlin& Perlin::get_instance() {
    static Perlin instance;
    return instance;
}

Perlin::Perlin() {
    perm_x = perlin_generate_perm();
    perm_y = perlin_generate_perm();
    perm_z = perlin_generate_perm();
}
float Perlin::fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float Perlin::grad(int hash, float x, float y, float z) {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float Perlin::noise(const Vec3& p) const {
    float u = p.x - std::floor(p.x);
    float v = p.y - std::floor(p.y);
    float w = p.z - std::floor(p.z);

    int i = static_cast<int>(std::floor(p.x));
    int j = static_cast<int>(std::floor(p.y));
    int k = static_cast<int>(std::floor(p.z));

    float uu = fade(u);
    float vv = fade(v);
    float ww = fade(w);

    float accum = 0.0f;

    for (int di = 0; di < 2; di++) {
        for (int dj = 0; dj < 2; dj++) {
            for (int dk = 0; dk < 2; dk++) {
                int index = perm_x[(i + di) & 255] ^ 
                            perm_y[(j + dj) & 255] ^ 
                            perm_z[(k + dk) & 255];
                
                float dot_val = grad(index, u - di, v - dj, w - dk);
                accum += (di * uu + (1 - di) * (1 - uu)) *
                         (dj * vv + (1 - dj) * (1 - vv)) *
                         (dk * ww + (1 - dk) * (1 - ww)) * dot_val;
            }
        }
    }
    
    return accum;
}

float Perlin::turb(const Vec3& p, int depth) const {
    float accum = 0.0f;
    Vec3 temp_p = p;
    float weight = 1.0f;

    for (int i = 0; i < depth; i++) {
        accum += weight * noise(temp_p);
        weight *= 0.5f;
        temp_p *= 2.0f;
    }

    return accum;
}

std::vector<int> Perlin::perlin_generate_perm() {
    std::vector<int> p(256);
    std::iota(p.begin(), p.end(), 0);
    permute(p, 256);
    return p;
}

void Perlin::permute(std::vector<int>& p, int n) {
    for (int i = n - 1; i > 0; i--) {
        int target = (int)Sampling::_generate_random_float(0, i);
        std::swap(p[i], p[target]);
    }
}

} // namespace hasmet