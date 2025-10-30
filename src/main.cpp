// src/main.cpp

#include <filesystem>
#include <memory>

#include "camera/pinhole.h"
#include "core/logging.h"
#include "film/film.h"
#include "geometry/sphere.h"
#include "geometry/triangle.h"
#include "integrator/whitted.h"
#include "material/material.h"
#include "material/material_manager.h"
#include "parser/parser_adapter.h"
#include "scene/scene.h"
//
//int main(int argc, char* argv[]) {
//  if (argc != 2) {
//    LOG_ERROR("Usage: whitted_renderer <input_json_file>");
//    return 1;
//  }
//
//  std::filesystem::path scene_path(argv[1]);
//  if (!std::filesystem::exists(scene_path)) {
//    LOG_ERROR("Input file does not exist: " << scene_path);
//    return 1;
//  }
//
//  try {
//    LOG_INFO("Reading scene...");
//    Scene scene = Parser::ParserAdapter::read_scene(scene_path.string());
//    WhittedIntegrator integrator(scene.render_config_.max_recursion_depth);
//
//    for (const std::unique_ptr<PinholeCamera>& camera : scene.cameras_) {
//      std::string output_name = camera->image_name_;
//      std::filesystem::path output_path(output_name);
//
//      if (!output_path.parent_path().empty()) {
//        std::filesystem::create_directories(output_path.parent_path());
//      }
//
//      Film image(camera->film_width_, camera->film_height_, output_name);
//      LOG_INFO("Rendering to: " << output_path.string());
//      integrator.render(scene, image, *camera);
//      image.write();
//    }
//  } catch (const std::exception& e) {
//    LOG_ERROR("An error occurred: " << e.what());
//    return 1;
//  }
//}

 int main() {

  const std::string filename = "cornellbox_recursive";
  const std::string input_folder = "C:/Users/akin/Desktop/hw1/inputs/";
  const std::string input_filename = input_folder + filename + ".json";
  const std::string output_folder = "C:/Users/akin/Desktop/whitted/";
  const std::string output_filename = output_folder + filename + ".png";

  LOG_INFO("Reading scene...");
  Scene scene = Parser::ParserAdapter::read_scene(input_filename);
  WhittedIntegrator integrator(scene.render_config_.max_recursion_depth);

  for (const std::unique_ptr<PinholeCamera>& camera : scene.cameras_) {
    Film image(camera->film_width_, camera->film_height_, output_filename);
    integrator.render(scene, image, *camera);
    image.write();
  }

  LOG_INFO("Program completed.");
  return 0;
}