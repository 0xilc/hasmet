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
#include "parser/parser.h"
#include "core/timer.h"

using namespace hasmet;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    LOG_ERROR("Usage: raytracer <input_json_file>");
    return 1;
  }

  std::filesystem::path scene_path(argv[1]);
  if (!std::filesystem::exists(scene_path)) {
    LOG_ERROR("Input file does not exist: " << scene_path);
    return 1;
  }

  try {
    LOG_INFO("Reading scene...");
    Scene scene = Parser::ParserAdapter::read_scene(scene_path.string());
    WhittedIntegrator integrator;

    for (const std::unique_ptr<Camera>& camera : scene.cameras_) {
      std::string output_name = camera->image_name_;
      std::filesystem::path output_path(output_name);

      if (!output_path.parent_path().empty()) {
        std::filesystem::create_directories(output_path.parent_path());
      }
      
      Film film(camera->film_width_, camera->film_height_, output_name);
      
      TIMER_START(Rendering);
      integrator.render(scene, film, *camera);
      TIMER_STOP(Rendering);
      film.write();
    }
  } catch (const std::exception& e) {
    LOG_ERROR("An error occurred: " << e.what());
    return 1;
  }
}
