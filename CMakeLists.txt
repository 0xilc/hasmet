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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source to this project's executable.
add_executable (raytracer "src/main.cpp" "src/core/logging.h" "src/core/ray.h" "src/core/spectrum.h" "src/io/image_io.cpp" "src/film/film.h" "src/film/film.cpp" "src/camera/camera.h" "src/camera/pinhole.h" "src/camera/pinhole.cpp" "src/geometry/shape.h" "src/geometry/sphere.h" "src/geometry/sphere.h" "src/geometry/sphere.cpp" "src/scene/scene.h" "src/scene/scene.cpp" "src/light/light.h" "src/light/ambient_light.h" "src/light/point_light.h" "src/integrator/integrator.h" "src/integrator/whitted.h" "src/integrator/whitted.cpp")

target_include_directories(raytracer PUBLIC 
"${CMAKE_CURRENT_SOURCE_DIR}/src"
"${CMAKE_CURRENT_SOURCE_DIR}/external"
)