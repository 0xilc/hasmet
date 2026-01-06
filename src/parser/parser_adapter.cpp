#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <filesystem>

#include "camera/pinhole.h"
#include "camera/thinlens.h"
#include "geometry/mesh.h"
#include "geometry/plane.h"
#include "geometry/sphere.h"
#include "geometry/triangle.h"
#include "light/area_light.h"
#include "light/spot_light.h"
#include "light/environment_light.h"
#include "material/material_manager.h"
#include "parser/parser.h"
#include "scene/scene.h"
#include "core/types.h"
#include "accelerator/instance.h"
#include "core/logging.h"
#include "texture/texture.h"
#include "texture/texture_manager.h"
#include "image/image_manager.h"

namespace hasmet
{
  namespace Parser
  {
    namespace ParserAdapter
    {
      Vec3 create_vec3(const Parser::Vec3f_ &v_)
      {
        return Vec3(v_.x, v_.y, v_.z);
      }

      Vec2 create_vec2(const Parser::Vec2f_ &v_)
      {
        return Vec2(v_.x, v_.y);
      }

      glm::mat4 create_transformation_matrix(
          const std::vector<Parser::Transformation_> &transforms)
      {
        glm::mat4 composite_matrix = glm::mat4(1.0f);

        for (const auto &tf : transforms)
        {
          glm::mat4 t_matrix = glm::mat4(1.0f);
          switch (tf.type)
          {
          case Parser::TransformationType::TRANSLATION:
            t_matrix = glm::translate(
                glm::mat4(1.0f), Vec3(tf.data[0], tf.data[1], tf.data[2]));
            break;
          case Parser::TransformationType::SCALING:
            t_matrix = glm::scale(glm::mat4(1.0f),
                                  Vec3(tf.data[0], tf.data[1], tf.data[2]));
            break;
          case Parser::TransformationType::ROTATION:
            t_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(tf.data[0]),
                                   Vec3(tf.data[1], tf.data[2], tf.data[3]));
            break;
          case Parser::TransformationType::COMPOSITE:
            t_matrix = glm::transpose(glm::make_mat4(tf.data.data()));
            break;
          }
          composite_matrix = t_matrix * composite_matrix;
        }
        return composite_matrix;
      }

      float get_triangle_area(Vec3 v1, Vec3 v2, Vec3 v3)
      {
        Vec3 edge1 = v2 - v1;
        Vec3 edge2 = v3 - v1;
        Vec3 cross_product = glm::cross(edge1, edge2);
        return 0.5f * glm::length(cross_product);
      }

      Color create_color(const Parser::Vec3f_ v_) { return Color(v_.x, v_.y, v_.z); }

      Texture create_texture(const Parser::TextureMap_& tm_) {
        Texture tex;
        tex.id = tm_.id;

        std::string type_str = tm_.type;
        std::transform(type_str.begin(), type_str.end(), type_str.begin(), ::tolower);
        
        if (type_str == "image") {
              tex.type = TextureType::IMAGE;
              tex.image_id = tm_.image_id;
        } 
        else if (type_str == "perlin") {
            tex.type = TextureType::PERLIN;
            tex.noise_scale = tm_.noise_scale;
            if (tm_.noise_conversion == "absval") {
              tex.noise_conversion = NoiseConversionType::ABSVAL;
            } else {
              tex.noise_conversion = NoiseConversionType::LINEAR;
            }
            tex.num_octaves = tm_.num_octaves;
        } 
        else if (type_str == "checkerboard") {
            tex.type = TextureType::CHECKERBOARD;
            tex.scale = tm_.scale;
            tex.offset = tm_.offset;
            tex.black_color = create_color(tm_.black_color);
            tex.white_color = create_color(tm_.white_color);
        }
        
        std::string decal_str = tm_.decal_mode;
        std::transform(decal_str.begin(), decal_str.end(), decal_str.begin(), ::tolower);

        if (decal_str == "replace_kd") tex.decal_mode = DecalMode::REPLACE_KD;
        else if (decal_str == "replace_ks") tex.decal_mode = DecalMode::REPLACE_KS;
        else if (decal_str == "replace_background") tex.decal_mode = DecalMode::REPLACE_BACKGROUND;
        else if (decal_str == "replace_normal") tex.decal_mode = DecalMode::REPLACE_NORMAL;
        else if (decal_str == "replace_all") tex.decal_mode = DecalMode::REPLACE_ALL;
        else if (decal_str == "bump_normal") tex.decal_mode = DecalMode::BUMP_NORMAL;
        else if (decal_str == "blend_kd") tex.decal_mode = DecalMode::BLEND_KD;

        std::string interp_str = tm_.interpolation;
        std::transform(interp_str.begin(), interp_str.end(), interp_str.begin(), ::tolower);

        if (interp_str == "bilinear") tex.interpolation = InterpolationType::BILINEAR;
        else tex.interpolation = InterpolationType::NEAREST;

        tex.bump_factor = tm_.bump_factor;
        tex.inv_normalizer = 255.0f / tm_.normalizer;

        return tex;
      }

      Triangle create_triangle(const Parser::Triangle_ &triangle_,
                               const std::vector<Parser::Vec3f_> &vertex_data_,
                               const std::vector<Parser::Vec2f_> &tex_coord_data_,
                               const Vec3 vertex_normals[3] = nullptr,
                               const Vec3 tangents[3] = nullptr,
                               bool smooth_shading = false)
      {
        Vec3 vertices[3] = {
            create_vec3(vertex_data_[triangle_.v0_id]),
            create_vec3(vertex_data_[triangle_.v1_id]),
            create_vec3(vertex_data_[triangle_.v2_id])};
        
        Vec2 uvs[3];
        bool has_uv = !tex_coord_data_.empty();
        if (has_uv)
        {
          uvs[0] = create_vec2(tex_coord_data_[triangle_.v0_id]);
          uvs[1] = create_vec2(tex_coord_data_[triangle_.v1_id]);
          uvs[2] = create_vec2(tex_coord_data_[triangle_.v2_id]);
        }
        
        return Triangle(vertices, vertex_normals, has_uv ? uvs : nullptr, tangents, smooth_shading);
      }

      PointLight create_point_light(const Parser::PointLight_ light_)
      {
        glm::mat4 transform = create_transformation_matrix(light_.transformations);
        Vec3 position =
            transform * glm::vec4(create_vec3(light_.position), 1.0f);
        return PointLight{position, create_color(light_.intensity)};
      }

      AreaLight create_area_light(const Parser::AreaLight_ light_)
      {
        glm::mat4 transform = create_transformation_matrix(light_.transformations);
        Vec3 position =
            transform * glm::vec4(create_vec3(light_.position), 1.0f);
        return AreaLight{position, create_vec3(light_.normal), light_.size,
                         create_color(light_.radiance)};
      }

      DirectionalLight create_directional_light(const Parser::DirectionalLight_ light_) {
        Vec3 direction = create_vec3(light_.direction);
        Color radiance = create_vec3(light_.radiance);
        return DirectionalLight(direction, radiance);
      }

      SpotLight create_spot_light(const Parser::SpotLight_ light_)
      {
        Vec3 position = create_vec3(light_.position);
        Vec3 direction = create_vec3(light_.direction);
        Vec3 intensity = create_vec3(light_.intensity);    
        return SpotLight{position, direction, intensity, light_.coverage_angle, light_.falloff_angle};
      }

      EnvironmentLight create_spherical_directional_light(const Parser::SphericalDirectionalLight_& light_) {
        return EnvironmentLight(light_.image_id, light_.type, light_.sampler);
      }

      Material create_material(const Parser::Material_ &material_)
      {
        Material mat;

        std::string type_str = material_.type;
        std::transform(type_str.begin(), type_str.end(), type_str.begin(), ::tolower);

        if (type_str == "mirror")
          mat.type = MaterialType::Mirror;
        else if (type_str == "conductor")
          mat.type = MaterialType::Conductor;
        else if (type_str == "dielectric")
          mat.type = MaterialType::Dielectric;
        else
          mat.type = MaterialType::BlinnPhong;

        mat.ambient_reflectance = create_color(material_.ambient_reflectance);
        mat.diffuse_reflectance = create_color(material_.diffuse_reflectance);
        mat.specular_reflectance = create_color(material_.specular_reflectance);

        if (material_.degamma) {
          mat.ambient_reflectance = glm::pow(mat.ambient_reflectance, Color(2.2f));
          mat.diffuse_reflectance = glm::pow(mat.diffuse_reflectance, Color(2.2f));
          mat.specular_reflectance = glm::pow(mat.specular_reflectance, Color(2.2f));
        }

        mat.mirror_reflectance = create_color(material_.mirror_reflectance);
        mat.absorption_coefficient = create_color(material_.absorption_coefficient);
        mat.absorption_index = material_.absorption_index;
        mat.phong_exponent = material_.phong_exponent;
        mat.refraction_index = material_.refraction_index;
        mat.roughness = material_.roughness;

        return mat;
      }

      PinholeCamera create_pinhole_camera(const Parser::Camera_ &camera_)
      {
        Vec3 position(camera_.position.x, camera_.position.y,
                      camera_.position.z);
        Vec3 gaze(camera_.gaze.x, camera_.gaze.y, camera_.gaze.z);
        Vec3 up(camera_.up.x, camera_.up.y, camera_.up.z);

        Vec3 look_at = position + gaze;

        float top = camera_.near_plane.t;
        float bottom = camera_.near_plane.b;

        float vertical_fov_radians =
            2.0f * atan((top - bottom) * 0.5f / camera_.near_distance);
        float vertical_fov_degrees = glm::degrees(vertical_fov_radians);

        // Apply transformations
        glm::mat4 transform = create_transformation_matrix(camera_.transformations);
        return PinholeCamera(position, look_at, up, vertical_fov_degrees,
                             camera_.image_width, camera_.image_height,
                             camera_.image_name, camera_.num_samples, transform);
      }

      ThinLensCamera create_thinlens_camera(const Parser::Camera_ &camera_)
      {
        Vec3 position(camera_.position.x, camera_.position.y,
                      camera_.position.z);
        Vec3 gaze(camera_.gaze.x, camera_.gaze.y, camera_.gaze.z);
        Vec3 up(camera_.up.x, camera_.up.y, camera_.up.z);
        Vec3 look_at = position + gaze;

        float top = camera_.near_plane.t;
        float bottom = camera_.near_plane.b;
        float vertical_fov_radians =
            2.0f * atan((top - bottom) * 0.5f / camera_.near_distance);
        float vertical_fov_degrees = glm::degrees(vertical_fov_radians);

        glm::mat4 transform = create_transformation_matrix(camera_.transformations);

        return ThinLensCamera(position, look_at, up, vertical_fov_degrees,
                              camera_.image_width, camera_.image_height,
                              camera_.image_name, transform, camera_.num_samples,
                              camera_.aperture_size, camera_.focus_distance);
      }

      Scene read_scene(std::string filename)
      {
        Parser::Scene_ parsed_scene;
        Parser::parseScene(filename, parsed_scene);

        Scene scene;
        scene.render_context_ = RenderContext{
            create_color(parsed_scene.background_color),
            parsed_scene.shadow_ray_epsilon, parsed_scene.intersection_test_epsilon,
            parsed_scene.max_recursion_depth};
        
        std::filesystem::path scene_path(filename);
        std::filesystem::path base_dir = scene_path.parent_path();

        // Read Images
        ImageManager* image_manager = ImageManager::get_instance();
        for (const Parser::Image_& img : parsed_scene.images) {
          std::filesystem::path image_path = base_dir / img.data;
          image_manager->load_image(img.id, image_path.string());
        }

        // Read Textures
        TextureManager* texture_manager = TextureManager::get_instance();
        for (const Parser::TextureMap_& tm : parsed_scene.texture_maps) {
          texture_manager->add(tm.id, create_texture(tm));
        }

        // Read Cameras
        for (const Parser::Camera_ &camera_ : parsed_scene.cameras)
        {
          std::unique_ptr<Camera> camera_ptr;
          if (camera_.aperture_size > 0)
          {
            camera_ptr = std::make_unique<ThinLensCamera>(create_thinlens_camera(camera_));
          }
          else
          {
            camera_ptr = std::make_unique<PinholeCamera>(create_pinhole_camera(camera_));
          }
          // Read tonemaps
          camera_ptr->tonemaps_.reserve(camera_.tonemaps.size());
          for (const Parser::Tonemap_& tm : camera_.tonemaps) {
            hasmet::Tonemap t;
            if (tm.tmo == "Photographic") {
              t.type = hasmet::Tonemap::Type::PHOTOGRAPHIC;
            } else if (tm.tmo == "Filmic") {
              t.type = hasmet::Tonemap::Type::FILMIC;
            } else if (tm.tmo == "ACES") {
              t.type = hasmet::Tonemap::Type::ACES;
            } else {
              t.type = hasmet::Tonemap::Type::LDR_LEGACY;
            }
            t.options[0] = tm.tmo_options[0];
            t.options[1] = tm.tmo_options[1];
            t.saturation = tm.saturation;
            t.gamma = tm.gamma;
            t.extension = tm.extension;
            camera_ptr->tonemaps_.push_back(t);
          }

          scene.cameras_.push_back(std::move(camera_ptr));
        }

        MaterialManager *material_manager = MaterialManager::get_instance();
        for (const Parser::Material_ material_ : parsed_scene.materials)
        {
          material_manager->add(material_.id, create_material(material_));
        }

        for (const Parser::Sphere_ &sphere_ : parsed_scene.spheres)
        {
          Vec3 position = create_vec3(parsed_scene.vertex_data[sphere_.center_vertex_id]);
          auto geometry = std::make_shared<Sphere>(position, sphere_.radius);
          auto inst = Instance(geometry);
          inst.set_transform(create_transformation_matrix(sphere_.transformations));
          inst.set_motion_blur(create_vec3(sphere_.motion_blur));
          inst.set_material_id(sphere_.material_id);
          inst.set_texture_ids(sphere_.texture_ids);
          scene.objects_.push_back(std::move(inst));
        }

        scene.ambient_light_ =
            std::make_unique<AmbientLight>(create_color(parsed_scene.ambient_light));

        for (const Parser::PointLight_ &light_ : parsed_scene.point_lights)
        {
          scene.point_lights_.push_back(
              std::make_unique<PointLight>(create_point_light(light_)));
        }

        for (const Parser::AreaLight_ &light_ : parsed_scene.area_lights)
        {
          scene.area_lights_.push_back(
              std::make_unique<AreaLight>(create_area_light(light_)));
        }

        for (const Parser::DirectionalLight_& light_ : parsed_scene.directional_lights) 
        {
          scene.directional_lights_.push_back(
            std::make_unique<DirectionalLight>(create_directional_light(light_)));
        }

        for (const Parser::SpotLight_& light_ : parsed_scene.spot_lights) 
        {
          scene.spot_lights_.push_back(
            std::make_unique<SpotLight>(create_spot_light(light_)));
        }
        
        // --> read environment lights
        for (const Parser::SphericalDirectionalLight_ &light_ : parsed_scene.spherical_directional_lights) {
          scene.environment_lights_.push_back(
              std::make_unique<EnvironmentLight>(create_spherical_directional_light(light_)));
        }

        for (const Parser::Triangle_ &triangle_ : parsed_scene.triangles)
        {
          auto geometry = std::make_shared<Triangle>(
              create_triangle(triangle_, parsed_scene.vertex_data, parsed_scene.tex_coord_data));
          auto inst = Instance(geometry);
          inst.set_transform(create_transformation_matrix(triangle_.transformations));
          inst.set_material_id(triangle_.material_id);
          inst.set_texture_ids(triangle_.texture_ids);
          scene.add_shape(std::move(inst));
        }
        
        struct ObjectBase
        {
          std::shared_ptr<Hittable> geometry;
          glm::mat4 composite_transform;
        };
        std::unordered_map<int, ObjectBase> object_registry;

        for (const Parser::Mesh_ &mesh_ : parsed_scene.meshes)
        {
          std::vector<Triangle> mesh_faces;
          if (mesh_.smooth_shading)
          {
            std::vector<std::vector<std::pair<Vec3, float>>>
                per_vertex_triangles;
            per_vertex_triangles.resize(parsed_scene.vertex_data.size());
            for (const Triangle_ &triangle_ : mesh_.faces)
            {
              Vec3 v0 = create_vec3(parsed_scene.vertex_data[triangle_.v0_id]);
              Vec3 v1 = create_vec3(parsed_scene.vertex_data[triangle_.v1_id]);
              Vec3 v2 = create_vec3(parsed_scene.vertex_data[triangle_.v2_id]);
              float area = get_triangle_area(v0, v1, v2);
              Vec3 edge1 = v1 - v0;
              Vec3 edge2 = v2 - v0;
              Vec3 face_normal = glm::normalize(glm::cross(edge1, edge2));
              per_vertex_triangles[triangle_.v0_id].push_back(
                  std::make_pair(face_normal, area));
              per_vertex_triangles[triangle_.v1_id].push_back(
                  std::make_pair(face_normal, area));
              per_vertex_triangles[triangle_.v2_id].push_back(
                  std::make_pair(face_normal, area));
            }
            std::vector<Vec3> vertex_normals;
            for (const auto &v : per_vertex_triangles)
            {
              Vec3 normal(0.0, 0.0, 0.0);
              float total_area = 0.0;
              for (const auto &pair : v)
              {
                normal = normal + pair.first * pair.second;
                total_area += pair.second;
              }
              if (total_area > 0.0)
              {
                normal = normal / total_area;
              }
              vertex_normals.push_back(normal);
            }
            for (const Triangle_ &triangle_ : mesh_.faces)
            {
              // 1. Vertices
              Vec3 vertices[3] = {
                  create_vec3(parsed_scene.vertex_data[triangle_.v0_id]),
                  create_vec3(parsed_scene.vertex_data[triangle_.v1_id]),
                  create_vec3(parsed_scene.vertex_data[triangle_.v2_id])};

              // 2. Normals
              Vec3 per_vertex_normals[3] = {
                  vertex_normals[triangle_.v0_id],
                  vertex_normals[triangle_.v1_id],
                  vertex_normals[triangle_.v2_id]};

              // 3. Texture Coordinates
              Vec2 uvs[3];
              bool has_uv = !parsed_scene.tex_coord_data.empty();
              if (has_uv)
              {
                uvs[0] = create_vec2(parsed_scene.tex_coord_data[triangle_.v0_id]);
                uvs[1] = create_vec2(parsed_scene.tex_coord_data[triangle_.v1_id]);
                uvs[2] = create_vec2(parsed_scene.tex_coord_data[triangle_.v2_id]);
              }

              // 4. Tangent & Bitangent
              Vec3 tangents[2];
              if (has_uv) {
                Vec3 edge1 = vertices[1] - vertices[0];
                Vec3 edge2 = vertices[2] - vertices[0];
                auto a = uvs[1].x - uvs[0].x;
                auto b = uvs[1].y - uvs[0].y;
                auto c = uvs[2].x - uvs[0].x;
                auto d = uvs[2].y - uvs[0].y;
                float inv_det = 1.0f / (a * d - b * c);

                tangents[0] = inv_det * (d * edge1 - b * edge2);
                tangents[1] = inv_det * (-c * edge1 + a * edge2);
                tangents[0] = glm::normalize(tangents[0]);
                tangents[1] = glm::normalize(tangents[1]);
              }

              mesh_faces.push_back(Triangle(vertices, per_vertex_normals, has_uv ? uvs : nullptr, has_uv ? tangents : nullptr, true));
            }
          }
          else
          {
            for (const Parser::Triangle_ &triangle_ : mesh_.faces)
            {
                // 1. Vertices
                Vec3 vertices[3] = {
                  create_vec3(parsed_scene.vertex_data[triangle_.v0_id]),
                  create_vec3(parsed_scene.vertex_data[triangle_.v1_id]),
                  create_vec3(parsed_scene.vertex_data[triangle_.v2_id])};
                
                // 2. Texture Coordinates
                Vec2 uvs[3];
                bool has_uv = !parsed_scene.tex_coord_data.empty();
                if (has_uv)
                {
                  uvs[0] = create_vec2(parsed_scene.tex_coord_data[triangle_.v0_id]);
                  uvs[1] = create_vec2(parsed_scene.tex_coord_data[triangle_.v1_id]);
                  uvs[2] = create_vec2(parsed_scene.tex_coord_data[triangle_.v2_id]);
                }

                // 3. Tangent & Bitangent
                Vec3 tangents[2];
                if (has_uv) {
                  Vec3 edge1 = vertices[1] - vertices[0];
                  Vec3 edge2 = vertices[2] - vertices[0];
                  auto a = uvs[1].x - uvs[0].x;
                  auto b = uvs[1].y - uvs[0].y;
                  auto c = uvs[2].x - uvs[0].x;
                  auto d = uvs[2].y - uvs[0].y;
                  float inv_det = 1.0f / (a * d - b * c);

                  tangents[0] = inv_det * (d * edge1 - b * edge2);
                  tangents[1] = inv_det * (-c * edge1 + a * edge2);
                  tangents[0] = glm::normalize(tangents[0]);
                  tangents[1] = glm::normalize(tangents[1]);
                }
                mesh_faces.push_back(Triangle(
                    vertices, 
                    nullptr, 
                    has_uv ? uvs : nullptr, 
                    has_uv ? tangents : nullptr,
                    false
                ));
            }
          }

          auto mesh_geo = std::make_shared<Mesh>(mesh_faces);
          auto inst = Instance(mesh_geo);
          glm::mat4 m_base = create_transformation_matrix(mesh_.transformations);
          inst.set_transform(m_base);
          inst.set_motion_blur(create_vec3(mesh_.motion_blur));
          inst.set_material_id(mesh_.material_id);
          inst.set_texture_ids(mesh_.texture_ids);
          object_registry[mesh_.id] = {mesh_geo, m_base};
          scene.add_shape(std::move(inst));
        }

        for (const Parser::MeshInstance_ mi_ : parsed_scene.mesh_instances)
        {
          if (object_registry.find(mi_.base_mesh_id) == object_registry.end())
          {
            LOG_ERROR("Base object ID " << mi_.base_mesh_id << " not found!");
            continue;
          }
          const auto &base_info = object_registry[mi_.base_mesh_id];
          glm::mat4 m_new = create_transformation_matrix(mi_.transformations);
          glm::mat4 m_final;
          if (mi_.reset_transform)
          {
            m_final = m_new;
          }
          else
          {
            m_final = m_new * base_info.composite_transform;
          }
          auto mi_inst = Instance(base_info.geometry);
          mi_inst.set_transform(m_final);
          mi_inst.set_motion_blur(create_vec3(mi_.motion_blur));
          mi_inst.set_material_id(mi_.material_id);
          mi_inst.set_texture_ids(mi_.texture_ids);
          object_registry[mi_.id] = {base_info.geometry, m_final};
          scene.objects_.push_back(std::move(mi_inst));
        }

        for (const Parser::Plane_ &plane_ : parsed_scene.planes)
        {
          Vec3 point =
              create_vec3(parsed_scene.vertex_data[plane_.point_vertex_id]);
          Vec3 normal = create_vec3(plane_.normal);

          auto plane_geo = std::make_shared<Plane>(point, normal);
          auto plane_inst = Instance(plane_geo);
          plane_inst.set_material_id(plane_.material_id);
          plane_inst.set_texture_ids(plane_.texture_ids);

          glm::mat4 transform = create_transformation_matrix(plane_.transformations);
          plane_inst.set_transform(transform);

          scene.planes_.push_back(std::move(plane_inst));
        }

        scene.build_bvh();
        return scene;
      }

    } // namespace ParserAdapter
  } // namespace Parser
} // namespace hasmet