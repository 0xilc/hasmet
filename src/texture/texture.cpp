#include "texture.h"
#include "image/image_manager.h"
#include <cmath>
#include <algorithm>
#include "core/logging.h"
#include "core/perlin.h"
#include "core/types.h"

namespace hasmet {

Color Texture::lookup(const Vec2& uv) const {
    if (type == TextureType::IMAGE) {
        if (image_id == -1){
            LOG_ERROR("texture.cpp :: could not locate the image with id" << image_id);
            return Color(1.0f, 0.0f, 1.0f); 
        }

        const Image& img = ImageManager::get_instance()->get(image_id);
        int w = img.get_width();
        int h = img.get_height();

        float u = uv.x - std::floor(uv.x);
        float v = uv.y - std::floor(uv.y);
        
        int x = std::min((int)(u * w), w - 1);
        int y = std::min((int)(v * h), h - 1);

        return img.get_pixel(x, y);
    }
    return Color(1.0f);
}

Color Texture::evaluate(const Vec2& uv, const Vec3& p) const {
    if (type == TextureType::CHECKERBOARD) {
        int x = (int)std::floor((p.x + offset) * scale);
        int y = (int)std::floor((p.y + offset) * scale);
        int z = (int)std::floor((p.z + offset) * scale);

        bool is_black = ((x + y + z) % 2) != 0;
        
        if (is_black) return black_color;
        else return white_color;
    }
    else if (type == TextureType::IMAGE) {
        
        if (image_id == -1){
            LOG_ERROR("texture.cpp :: could not locate the image with id" << image_id);
            return Color(1.0f, 0.0f, 1.0f); 
        }

        const Image& img = ImageManager::get_instance()->get(image_id);
        int w = img.get_width();
        int h = img.get_height();

        float u = uv.x - std::floor(uv.x);
        float v = uv.y - std::floor(uv.y);
        
        float x = u * w;
        float y = v * h;

        if (interpolation == InterpolationType::NEAREST) {
            int i = std::min((int)x, w - 1);
            int j = std::min((int)y, h - 1);
            return img.get_pixel(i, j);
        }
        else if (interpolation == InterpolationType::BILINEAR) {
            float x_center = x - 0.5f;
            float y_center = y - 0.5f;

            int x_low = (int)std::floor(x_center);
            int y_low = (int)std::floor(y_center);

            float dx = x_center - x_low;
            float dy = y_center - y_low;

            auto get_clamped = [&](int i, int j) {
                if (i < 0) i = 0;
                if (j < 0) j = 0;
                if (i >= w) i = w - 1;
                if (j >= h) j = h - 1;
                return img.get_pixel(i, j);
            };

            Color c00 = get_clamped(x_low, y_low);
            Color c10 = get_clamped(x_low + 1, y_low);
            Color c01 = get_clamped(x_low, y_low + 1);
            Color c11 = get_clamped(x_low + 1, y_low + 1);

            Color c_bottom = c00 * (1.0f - dx) + c10 * dx;
            Color c_top    = c01 * (1.0f - dx) + c11 * dx;

            return c_bottom * (1.0f - dy) + c_top * dy;
        }
    }
    else if (type == TextureType::PERLIN) {
        Vec3 scaled_p = p * noise_scale;

        float noise_val = Perlin::get_instance().turb(scaled_p, (int)num_octaves);
        if (noise_conversion == NoiseConversionType::LINEAR) {
            noise_val = 0.5f * (1.0f + noise_val);
        } else {
            noise_val = std::abs(noise_val);
        }
        noise_val = std::max(0.0f, std::min(1.0f, noise_val));
        
        return Color(noise_val, noise_val, noise_val);
    }

    return Color(1.0f);
}

Vec2 Texture::get_height_derivative(const Vec2& uv, const Vec3& p, const Vec3& T, const Vec3& B) const {
    float delta_u = 0.001f;
    float delta_v = 0.001f;
    if (type == TextureType::IMAGE && image_id != -1) {
         const Image& img = ImageManager::get_instance()->get(image_id);
         delta_u = 1.0f / img.get_width(); 
         delta_v = 1.0f / img.get_height();
    
        Color c_center = evaluate(uv, p);
        float h_center = (c_center.r + c_center.g + c_center.b) / (3.0f);

        Color c_u = lookup(uv + Vec2(delta_u, 0.0f));
        float h_u = (c_u.r + c_u.g + c_u.b) / (3.0f);

        Color c_v = lookup(uv + Vec2(0.0f, delta_v));
        float h_v = (c_v.r + c_v.g + c_v.b) / (3.0f);

        float dh_du = (h_u - h_center);
        float dh_dv = (h_v - h_center);

        return Vec2(dh_du, dh_dv);
    } else {
        Color c_center = evaluate(uv, p);
        float h_center = (c_center.r + c_center.g + c_center.b) / (3.0f);

        Color c_u = evaluate(uv + Vec2(delta_u, 0.0f), p + T * delta_u);
        float h_u = (c_u.r + c_u.g + c_u.b) / (3.0f);

        Color c_v = evaluate(uv + Vec2(0.0f, delta_v), p + B * delta_v);
        float h_v = (c_v.r + c_v.g + c_v.b) / (3.0f);

        float dh_du = (h_u - h_center) / delta_u;
        float dh_dv = (h_v - h_center) / delta_v;
        return Vec2(dh_du, dh_dv);
    }
}

} // namespace hasmet