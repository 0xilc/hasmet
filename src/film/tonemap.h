#pragma once

#include "film.h"
#include "logging.h"
#include "camera/camera.h"

namespace hasmet {

Film do_tonemapping(const Tonemap& tonemap, const Film& film);

} // namespace hasmet