#pragma once

#include <string>
#include <vector>
#include <ostream>

namespace Parser{

typedef struct Vec3f_ {
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
} Camera_;

typedef struct PointLight_ {
    int id;
    Vec3f_ position;
    Vec3f_ intensity;
} PointLight_;

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
} Material_;

typedef struct Triangle_ {
    int material_id;
    int v0_id, v1_id, v2_id;
    std::vector<Transformation_> transformations;
} Triangle_;

typedef struct Mesh_ {
    int id;
    int material_id;
    std::vector<Triangle_> faces;
    bool smooth_shading;
    std::vector<Transformation_> transformations;
} Mesh_;

typedef struct MeshInstance_ {
    int id;
    int base_mesh_id;
    int material_id;
    bool reset_transform;
    std::vector<Transformation_> transformations;
} MeshInstance_;

typedef struct Sphere_ {
    int id;
    int material_id;
    int center_vertex_id;
    float radius;
    std::vector<Transformation_> transformations;
} Sphere_;

typedef struct Plane_ {
    int id;
    int material_id;
    int point_vertex_id;
    Vec3f_ normal;
    std::vector<Transformation_> transformations;
} Plane_;

typedef struct Scene_ {
    Vec3f_ background_color;
    float shadow_ray_epsilon;
    float intersection_test_epsilon;
    int max_recursion_depth;         
    std::vector<Camera_> cameras;
    Vec3f_ ambient_light;
    std::vector<PointLight_> point_lights;
    std::vector<Material_> materials;
    std::vector<Vec3f_> vertex_data;
    std::vector<Mesh_> meshes;
    std::vector<MeshInstance_> mesh_instances;
    std::vector<Triangle_> triangles;
    std::vector<Sphere_> spheres;
    std::vector<Plane_> planes;
    std::vector<Transformation_> transformations;
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
void printScene(const Scene_& scene);

}  // namespace Parser