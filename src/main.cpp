#include <filesystem>
#include <memory>
#include <chrono>

#include "camera/pinhole.h"
#include "camera/thinlens.h"
#include "core/logging.h"
#include "film/film.h"
#include "geometry/sphere.h"
#include "geometry/triangle.h"
#include "integrator/whitted.h"
#include "material/material.h"
#include "material/material_manager.h"
#include "parser/parser_adapter.h"
#include "scene/scene.h"
#include <chrono>
#include "parser/parser.h"

using namespace hasmet;

// int main(int argc, char* argv[]) {
//   if (argc != 2) {
//     LOG_ERROR("Usage: whitted_renderer <input_json_file>");
//     return 1;
//   }

//   std::filesystem::path scene_path(argv[1]);
//   if (!std::filesystem::exists(scene_path)) {
//     LOG_ERROR("Input file does not exist: " << scene_path);
//     return 1;
//   }

//   try {
//     LOG_INFO("Reading scene...");
//     Scene scene = Parser::ParserAdapter::read_scene(scene_path.string());
//     WhittedIntegrator integrator(scene.render_config_.max_recursion_depth);

//     for (const std::unique_ptr<Camera>& camera : scene.cameras_) {
//       std::string output_name = camera->image_name_;
//       std::filesystem::path output_path(output_name);

//       if (!output_path.parent_path().empty()) {
//         std::filesystem::create_directories(output_path.parent_path());
//       }
//       auto start = std::chrono::high_resolution_clock::now();
//       Film image(camera->film_width_, camera->film_height_, output_name);
//       LOG_INFO("Rendering to: " << output_path.string());
//       integrator.render(scene, image, *camera);
//       auto end = std::chrono::high_resolution_clock::now();
//       std::chrono::duration<double> elapsed = end - start;
//       LOG_INFO("Rendering: " << output_path << " : "<< elapsed.count() << " seconds.");
//       image.write();

//     }
//   } catch (const std::exception& e) {
//     LOG_ERROR("An error occurred: " << e.what());
//     return 1;
//   }
// }

int main() {
 const std::string filename = "dragon_metal";
 const std::string input_folder =
     "/home/ilc/Desktop/hw2/inputs/";
 const std::string input_filename = input_folder + filename + ".json";
 const std::string output_folder = "/home/ilc/Desktop/whitted/";
 const std::string output_filename = output_folder + filename + ".png";

 LOG_INFO("Reading the scene: " << filename);
 Parser::Scene_ parser_scene;
 Parser::parseScene(input_filename, parser_scene);
 Parser::printScene(parser_scene);
 
 auto start = std::chrono::high_resolution_clock::now();
 Scene scene = Parser::ParserAdapter::read_scene(input_filename);
 WhittedIntegrator integrator(scene.render_config_.max_recursion_depth);
 auto end = std::chrono::high_resolution_clock::now();
 std::chrono::duration<double> elapsed = end - start;
 LOG_INFO("Reading took:" << elapsed.count() << " seconds.");

 for (const std::unique_ptr<Camera>& camera : scene.cameras_) {
   Film image(camera->film_width_, camera->film_height_, output_filename);
   auto start = std::chrono::high_resolution_clock::now();
   LOG_INFO("Starting rendering.")
   integrator.render(scene, image, *camera);
   auto end = std::chrono::high_resolution_clock::now();
   std::chrono::duration<double> elapsed = end - start;
   LOG_INFO("Rendering took: " << elapsed.count() << " seconds.");
   image.write();
 }

 LOG_INFO("Program terminated.");
 return 0;
}