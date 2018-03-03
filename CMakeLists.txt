cmake_minimum_required(VERSION 3.1)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
project(adarkar_wastes)

enable_language(C)
enable_language(CXX)

include("defaults.cmake")

find_boost("filesystem" "program_options" "system" "thread" "wave")
find_glew()
find_opengl()
find_png()
find_sdl2()

output_flags("DEBUG" on)

add_definitions(-DUSE_LD)

include_directories("${PROJECT_SOURCE_DIR}/src")

add_executable(adarkar_wastes
  "src/bsd_rand.c"
  "src/bsd_rand.h"
  "src/glsl_program.cpp"
  "src/glsl_program.hpp"
  "src/glsl_shader.cpp"
  "src/glsl_shader.hpp"
  "src/glsl_wave.cpp"
  "src/glsl_wave.hpp"
  "src/image_png.cpp"
  "src/image_png.hpp"
  "src/intro.cpp")
if(MSVC)
  target_link_libraries(adarkar_wastes "Imm32")
  target_link_libraries(adarkar_wastes "Version")
  target_link_libraries(adarkar_wastes "WinMM")
  target_link_libraries(adarkar_wastes "${OPENGL_gl_LIBRARY}")
  target_link_libraries(adarkar_wastes debug "${GLEW_LIBRARY_DEBUG}" optimized "${GLEW_LIBRARY}")
  target_link_libraries(adarkar_wastes debug "${PNG_LIBRARY_DEBUG}" optimized "${PNG_LIBRARY}")
  target_link_libraries(adarkar_wastes debug "${SDL2_LIBRARY_DEBUG}" optimized "${SDL2_LIBRARY}")
  target_link_libraries(adarkar_wastes debug "${SNDFILE_LIBRARY_DEBUG}" optimized "${SNDFILE_LIBRARY}")
  target_link_libraries(adarkar_wastes debug "${ZLIB_LIBRARY_DEBUG}" optimized "${ZLIB_LIBRARY}")
else()
  target_link_libraries(adarkar_wastes "${BOOST_FILESYSTEM_LIBRARY}")
  target_link_libraries(adarkar_wastes "${BOOST_PROGRAM_OPTIONS_LIBRARY}")
  target_link_libraries(adarkar_wastes "${BOOST_SYSTEM_LIBRARY}")
  target_link_libraries(adarkar_wastes "${BOOST_THREAD_LIBRARY}")
  target_link_libraries(adarkar_wastes "${BOOST_WAVE_LIBRARY}")
  target_link_libraries(adarkar_wastes "${GLEW_LIBRARY}")
  target_link_libraries(adarkar_wastes "${PNG_LIBRARY}")
  target_link_libraries(adarkar_wastes "${SDL2_LIBRARY}")
  target_link_libraries(adarkar_wastes "${SNDFILE_LIBRARY}")
endif()
