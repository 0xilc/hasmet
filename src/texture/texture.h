#pragma once

#include "core/types.h"

namespace hasmet {
enum class TextureType {
	IMAGE,
	PERLIN,
	CHECKERBOARD
};

enum class DecalMode {
    REPLACE_KD,
    REPLACE_KS,
    REPLACE_BACKGROUND,
    REPLACE_NORMAL,
    REPLACE_ALL,
    BUMP_NORMAL,
    BLEND_KD
};

enum class InterpolationType {
    NEAREST,
    BILINEAR
};

enum class NoiseConversionType {
    LINEAR,
    ABSVAL
};

struct Texture{
    int id;
    TextureType type = TextureType::IMAGE;
    DecalMode decal_mode = DecalMode::REPLACE_KD;
    InterpolationType interpolation = InterpolationType::BILINEAR;

    int image_id = -1;
    float normalizer = 255.0f;

    float bump_factor = 1.0f;
    float noise_scale = 1.0f;
    NoiseConversionType noise_conversion = NoiseConversionType::LINEAR;
    float num_octaves = 1.0f;

    float scale = 1.0f;
    float offset = 0.0f;
    Color black_color{0.0f};
    Color white_color{255.0f};

    Texture() = default;

    Color evaluate(const Vec2& uv, const Vec3& p) const;

    Vec2 get_height_derivative(const Vec2& uv, const Vec3& p, const Vec3& T, const Vec3& B) const;
};
} // namespace hasmet