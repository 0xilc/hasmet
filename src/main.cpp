// src/main.cpp

#include <memory>

#include "camera/pinhole.h"
#include "core/logging.h"
#include "film/film.h"
#include "geometry/sphere.h"
#include "geometry/triangle.h"
#include "integrator/whitted.h"
#include "material/material.h"
#include "material/material_manager.h"
#include "scene/scene.h"
#include "parser/parser_adapter.h"


int main() {
  LOG_INFO("Hasmet Renderer");

  const std::string filename = "deniz_sayin/lobster";
  const std::string input_folder = "C:/Users/akin/Desktop/hw1/inputs/";
  const std::string input_filename = input_folder + filename + ".json"; 
  const std::string output_folder = "C:/Users/akin/Desktop/whitted/";
  const std::string output_filename = output_folder + filename + ".png";

  Scene scene = Parser::ParserAdapter::read_scene(input_filename);
  WhittedIntegrator integrator(scene.render_config_.max_recursion_depth);

  for (const std::unique_ptr<PinholeCamera>& camera : scene.cameras_) {
    Film image(camera->film_width_, camera->film_height_, output_filename);
    integrator.render(scene, image, *camera);
    image.write();
  }

  LOG_INFO("Render complete.");
  return 0;
}