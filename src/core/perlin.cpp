#include "core/perlin.h"
#include "core/sampling.h"
#include <cmath>
#include <numeric>

namespace hasmet {

Perlin* Perlin::instance_ = nullptr;

Perlin* Perlin::get_instance() {
    if (instance_ == nullptr) {
        instance_ = new Perlin();
    }
    return instance_;
}

Perlin::Perlin() {
    ranvec.resize(point_count);
    for (int i = 0; i < point_count; ++i) {
        ranvec[i] = glm::normalize(Vec3(
            Sampling::_generate_random_float(-1, 1),
            Sampling::_generate_random_float(-1, 1),
            Sampling::_generate_random_float(-1, 1)
        ));
    }

    perm_x = perlin_generate_perm();
    perm_y = perlin_generate_perm();
    perm_z = perlin_generate_perm();
}

float Perlin::noise(const Vec3& p) const {
    float u = p.x - std::floor(p.x);
    float v = p.y - std::floor(p.y);
    float w = p.z - std::floor(p.z);

    // Hermite cubic smoothing
    u = u * u * (3 - 2 * u);
    v = v * v * (3 - 2 * v);
    w = w * w * (3 - 2 * w);

    int i = static_cast<int>(std::floor(p.x));
    int j = static_cast<int>(std::floor(p.y));
    int k = static_cast<int>(std::floor(p.z));

    float c[2][2][2];

    for (int di = 0; di < 2; di++) {
        for (int dj = 0; dj < 2; dj++) {
            for (int dk = 0; dk < 2; dk++) {
                int index = perm_x[(i + di) & 255] ^
                            perm_y[(j + dj) & 255] ^
                            perm_z[(k + dk) & 255];
                
                Vec3 weight_v(u - di, v - dj, w - dk);
                c[di][dj][dk] = glm::dot(ranvec[index], weight_v);
            }
        }
    }

    return trilinear_interp(c, u, v, w);
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

float Perlin::trilinear_interp(float c[2][2][2], float u, float v, float w) {
    float accum = 0.0f;
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            for (int k = 0; k < 2; k++) {
                accum += (i * u + (1 - i) * (1 - u)) *
                         (j * v + (1 - j) * (1 - v)) *
                         (k * w + (1 - k) * (1 - w)) * c[i][j][k];
            }

    return accum;
}

std::vector<int> Perlin::perlin_generate_perm() {
    std::vector<int> p(point_count);

    std::iota(p.begin(), p.end(), 0);
    permute(p, point_count);
    return p;
}

void Perlin::permute(std::vector<int>& p, int n) {
    for (int i = n - 1; i > 0; i--) {
        int target = (int)Sampling::_generate_random_float(0, i);
        std::swap(p[i], p[target]);
    }
}

} // namespace hasmet
