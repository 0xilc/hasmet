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

# Add source to this project's executable.
add_executable (raytracer "src/main.cpp" "src/core/vec.h" "src/core/logging.h")

target_include_directories(raytracer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")