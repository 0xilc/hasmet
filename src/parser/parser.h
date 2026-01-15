#pragma once

#include <string>
#include <vector>
#include <ostream>

namespace hasmet {
namespace Parser {

typedef struct Vec2f_ {
    Vec2f_() : x(0), y(0) {}
    Vec2f_(float x, float y) : x(x), y(y) {}
    float x, y;
} Vec2f_;


typedef struct Vec3f_ {
    Vec3f_() {};
    Vec3f_(float x, float y, float z) : x(x), y(y), z(z) {};
    float x, y, z;
} Vec3f_;

typedef struct Vec4f_ {
    float l, r, b, t;
} Vec4f_;

enum TransformationType {
    TRANSLATION,
    SCALING,
    ROTATION,
    COMPOSITE
};

typedef struct Transformation_ {
    std::string id;
    TransformationType type;
    std::vector<float> data;
} Transformation_;

typedef struct Tonemap_ {
    std::string tmo;
    float tmo_options[2];
    float saturation;
    float gamma;
    std::string extension;
} Tonemap_;

typedef struct Camera_ {
    int id;
    Vec3f_ position;
    Vec3f_ gaze;
    Vec3f_ up;
    Vec4f_ near_plane;
    float near_distance;
    int image_width;
    int image_height;
    std::string image_name;
    std::vector<Transformation_> transformations;
    int num_samples;
    float aperture_size;
    float focus_distance;
    std::vector<Tonemap_> tonemaps;
    std::string renderer;
    std::vector<std::string> renderer_params;
} Camera_;

typedef struct PointLight_ {
    int id;
    Vec3f_ position;
    Vec3f_ intensity;
    std::vector<Transformation_> transformations;
} PointLight_;

typedef struct AreaLight_ {
    int id;
    Vec3f_ position;
    Vec3f_ normal;
    float size;
    Vec3f_ radiance;
    std::vector<Transformation_> transformations;
} AreaLight_;

typedef struct DirectionalLight_ {
    int id;
    Vec3f_ direction;
    Vec3f_ radiance;
} DirectionalLight_;

typedef struct SpotLight_ {
    int id;
    Vec3f_ position;
    Vec3f_ direction;
    Vec3f_ intensity;
    float coverage_angle;
    float falloff_angle;
    std::vector<Transformation_> transformations;
} SpotLight_;

typedef struct SphericalDirectionalLight_ {
    int id;
    std::string type;
    int image_id;
    std::string sampler;
} SphericalDirectionalLight_;

typedef struct BRDF_ {
  int id;
  std::string type;
  bool normalized = false;
  float exponent = 1.0f;
  bool kdfresnel = false;
} BRDF_;
 
typedef struct Material_ {
    int id;
    std::string type;
    Vec3f_ ambient_reflectance;
    Vec3f_ diffuse_reflectance;
    Vec3f_ specular_reflectance;
    Vec3f_ mirror_reflectance;
    float phong_exponent;
    float refraction_index;
    Vec3f_ absorption_coefficient;
    float absorption_index;
    float roughness;
    bool degamma;
} Material_;

struct Image_ {
    int id;
    std::string data;
};

struct TextureMap_ {
    int id;
    std::string type;
    int image_id;
    std::string decal_mode;
    std::string interpolation;
    float bump_factor;

    // Perlin noise params
    float noise_scale = 1.0f;
    std::string noise_conversion;
    float num_octaves = 1.0f;

    // Checkerboard params
    float scale;
    float offset;
    Vec3f_ black_color;
    Vec3f_ white_color;

    float normalizer;
};

typedef struct Triangle_ {
    int material_id;
    int v0_id, v1_id, v2_id;
    std::vector<Transformation_> transformations;
    std::vector<int> texture_ids;
} Triangle_;

typedef struct Mesh_ {
    int id;
    int material_id;
    bool smooth_shading;
    std::vector<Triangle_> faces;
    std::vector<Transformation_> transformations;
    Vec3f_ motion_blur;
    std::vector<int> texture_ids;
    Vec3f_ radiance = {0.0f, 0.0f, 0.0f};
} Mesh_;

typedef struct MeshInstance_ {
    int id;
    int base_mesh_id;
    int material_id;
    bool reset_transform;
    std::vector<Transformation_> transformations;
    Vec3f_ motion_blur;
    std::vector<int> texture_ids;
    Vec3f_ radiance = {0.0f, 0.0f, 0.0f};
} MeshInstance_;

typedef struct Sphere_ {
    int id;
    int material_id;
    int center_vertex_id;
    float radius;
    std::vector<Transformation_> transformations;
    Vec3f_ motion_blur;
    std::vector<int> texture_ids;
    Vec3f_ radiance = {0.0f, 0.0f, 0.0f};
} Sphere_;

typedef struct Plane_ {
    int id;
    int material_id;
    int point_vertex_id;
    Vec3f_ normal;
    std::vector<Transformation_> transformations;
    std::vector<int> texture_ids;
} Plane_;

typedef struct Scene_ {
    float shadow_ray_epsilon;
    float intersection_test_epsilon;
    int max_recursion_depth;
    Vec3f_ background_color;
    Vec3f_ ambient_light;
    std::vector<Camera_> cameras;
    std::vector<PointLight_> point_lights;
    std::vector<AreaLight_> area_lights;
    std::vector<DirectionalLight_> directional_lights;
    std::vector<SphericalDirectionalLight_> spherical_directional_lights;
    std::vector<SpotLight_> spot_lights;
    std::vector<Material_> materials;
    std::vector<Vec3f_> vertex_data;
    std::vector<Vec2f_> tex_coord_data;
    std::vector<Image_> images;
    std::vector<TextureMap_> texture_maps;
    std::vector<Mesh_> meshes;
    std::vector<MeshInstance_> mesh_instances;
    std::vector<Triangle_> triangles;
    std::vector<Sphere_> spheres;
    std::vector<Plane_> planes;
    std::vector<Transformation_> transformations;
    std::vector<BRDF_> brdfs;
} Scene_;

// --- Function Declaration ---
void parseScene(const std::string& filename, Scene_& scene);

inline std::ostream& operator<<(std::ostream& os, const Vec3f_& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Vec4f_& v) {
    os << "(l:" << v.l << ", r:" << v.r << ", b:" << v.b << ", t:" << v.t << ")";
    return os;
}

void printSceneSummary(const Scene_& scene);

}  // namespace Parser
}  // namespace hasmet