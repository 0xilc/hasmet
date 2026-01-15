# CMakeList.txt : CMake project for hasmet, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("hasmet")

# Default to Release build if no build type is specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)

    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)
add_executable (raytracer "src/main.cpp" "src/core/logging.h" "src/core/ray.h" "src/io/image_io.cpp" "src/film/film.h" "src/film/film.cpp" "src/camera/camera.h" "src/camera/pinhole.h" "src/camera/pinhole.cpp" "src/geometry/sphere.h" "src/geometry/sphere.cpp" "src/scene/scene.h" "src/scene/scene.cpp" "src/light/light.h" "src/light/ambient_light.h" "src/light/point_light.h" "src/integrator/integrator.h" "src/integrator/whitted.h" "src/integrator/whitted.cpp"  "src/geometry/triangle.h" "src/geometry/triangle.cpp" "src/material/material_manager.h" "src/material/material_manager.cpp"  "src/core/aabb.h" "src/core/interval.h" "src/accelerator/hittable.h" "src/core/hit_record.h" "src/accelerator/bvh.h" "src/parser/parser.h" "src/parser/parser.cpp" "src/parser/parser_adapter.cpp" "src/parser/parser_adapter.h"   "src/geometry/plane.h" "src/geometry/plane.cpp" "src/geometry/mesh.h" "src/geometry/mesh.cpp"   "src/camera/thinlens.cpp" "src/light/area_light.h" "src/core/sampling.h"  "src/accelerator/instance.h" "src/core/sampler.h" "src/texture/texture_manager.cpp" "src/image/image_manager.cpp" "src/image/image.cpp" "src/texture/texture.cpp" "src/core/perlin.h" "src/core/perlin.cpp" "external/miniz.c" "src/film/tonemap.cpp" "src/light/environment_light.cpp" "src/light/point_light.cpp" "src/light/spot_light.cpp" "src/light/directional_light.cpp" "src/light/area_light.cpp" "src/material/material.cpp" "src/core/frame.h" "src/material/bxdf.h" "src/material/bsdf.h" "src/material/bsdf.cpp")

target_include_directories(raytracer PUBLIC 
"${CMAKE_CURRENT_SOURCE_DIR}/src"
"${CMAKE_CURRENT_SOURCE_DIR}/external"
)

target_link_libraries(raytracer PUBLIC OpenMP::OpenMP_CXX)
