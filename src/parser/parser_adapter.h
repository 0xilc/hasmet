#pragma once

#include <iostream>

#include "camera/pinhole.h"
#include "parser/parser.h"
#include "scene/scene.h"
#include "material/material_manager.h"
#include "core/types.h"

namespace hasmet {
namespace Parser {
namespace ParserAdapter {

Color create_color(const Parser::Vec3f_ v_);
PointLight create_point_light(const Parser::PointLight_ light_);
Material create_material(const Parser::Material_& material_);
PinholeCamera create_pinhole_camera(const Parser::Camera_& camera_);
Scene read_scene(std::string filename);
Vec3 create_vec3(const Parser::Vec3f_& v_);

}  // namespace ParserAdapter
}  // namespace Parser
}  // namespace hasmet