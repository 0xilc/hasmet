#define _USE_MATH_DEFINES
#include <cmath>
#include <glm/gtc/constants.hpp>

#include "core/ray.h"
#include "thinlens.h"

ThinLensCamera::ThinLensCamera(const glm::vec3& position,
                               const glm::vec3& look_at, 
                               const glm::vec3& up,
                               float vertical_fov_degrees, 
                               int film_width,
                               int film_height,
                               std::string image_name,
                               const glm::mat4& transform = glm::mat4(1.0f))
    : film_width_(film_width),
      film_height_(film_height),
      image_name_(image_name),
{

}

Ray ThinLensCamera::generateRay(float px, float py) const { return Ray(); }