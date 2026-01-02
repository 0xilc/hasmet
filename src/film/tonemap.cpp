#include "tonemap.h"

namespace hasmet {
namespace {
float get_luminance(const Color& p) {
    return 0.2126f * p.r + 0.7152f * p.g + 0.0722f * p.b;
}

float compute_log_avg_luminance(const Film& film) {
    float sum_log_lum = 0.0f;
    float eps = 0.00001f;
    int N = film.getWidth() * film.getHeight();

    for (const auto& p : film.pixels_) {
        float Lw = get_luminance(p);
        sum_log_lum += glm::log(eps + Lw);
    }

    return glm::exp(sum_log_lum / N);
}

float compute_l_white(const Film& film, float burn_percent, float scale) {
    if (burn_percent <= 0.0f) {
        return 1e20f;
    }

    std::vector<float> luminances;
    luminances.reserve(film.pixels_.size());

    for (const Color& p : film.pixels_) {
        luminances.push_back(get_luminance(p));
    }

    std::sort(luminances.begin(), luminances.end());

    float fraction = 1.0f - (burn_percent / 100.0f);

    int index = static_cast<int>(fraction * (luminances.size() - 1));
    if (index < 0) index = 0;
    if (index >= luminances.size()) index = luminances.size() - 1;

    return luminances[index] * scale;
}

void apply_saturation(Color& pixel, float saturation, float Ld) {
    float inv_lum = 1.0f / get_luminance(pixel);
    pixel.r = glm::pow(pixel.r * inv_lum, saturation) * Ld;
    pixel.g = glm::pow(pixel.g * inv_lum, saturation) * Ld;
    pixel.b = glm::pow(pixel.b * inv_lum, saturation) * Ld;
}

void apply_gamma_correction(Color& pixel, float gamma) {
    if (gamma <= 0.0f) {
        LOG_INFO("Provided gamma value is not valid. Gamma:" << gamma);
        gamma = 2.2f;
    }

    float inv_gamma = 1.0f / gamma;
    pixel.r = 255.0f * glm::clamp(glm::pow(pixel.r, inv_gamma), 0.0f, 1.0f);
    pixel.g = 255.0f * glm::clamp(glm::pow(pixel.g, inv_gamma), 0.0f, 1.0f);
    pixel.b = 255.0f * glm::clamp(glm::pow(pixel.b, inv_gamma), 0.0f, 1.0f);
}

} // namespace

Film do_tonemapping(const Tonemap& tonemap, const Film& film) {
    Film result = film;
    switch (tonemap.type){
        case Tonemap::Type::PHOTOGRAPHIC: {
            photographic(tonemap, result);
            break;
        }
        case Tonemap::Type::ACES: {
            aces(tonemap, result);
            break;
        }
        case Tonemap::Type::FILMIC: {
            filmic(tonemap, result);
            break;
        }
        case Tonemap::Type::NONE: {
            // Do nothing
            break;
        }
    }
    
    return result;
}

void photographic(const Tonemap& tm, Film& film) {
    float L_avg = compute_log_avg_luminance(film);
    float key = tm.options[0];
    float scale = key / L_avg;
    float L_white = compute_l_white(film, tm.options[1], scale);
    float L_white2 = L_white * L_white2;

    for (Color& p : film.pixels_) {
        float Li = get_luminance(p);
        float L_scaled = Li * scale;
        float Ld = (L_scaled * (1.0f + (L_scaled / L_white2))) / (1.0f + L_scaled);
        
        apply_saturation(p, tm.saturation, Ld);
        apply_gamma_correction(p, tm.gamma);
    }
}
void filmic(const Tonemap& tm, Film& film) {
    LOG_ERROR("Not implemented yet");
    return;
}
void aces(const Tonemap& tm, Film& film)
{
    LOG_ERROR("Not implemented yet");
    return;
}

} // namespace hasmet