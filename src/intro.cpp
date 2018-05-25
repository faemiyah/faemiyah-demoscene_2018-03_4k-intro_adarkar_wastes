/// \file
/// Very small intro stub.

//######################################
// Include #############################
//######################################

#include "dnload.h"
#if !defined(WIN32)
#include "synth.h"
#endif

#if defined(USE_LD)
#include "glsl_program.hpp"
#include "image_png.hpp"
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/scoped_array.hpp>
#include <boost/tuple/tuple.hpp>
namespace fs = boost::filesystem;
namespace po = boost::program_options;
#endif

//######################################
// Define ##############################
//######################################

#if !defined(DISPLAY_MODE)
/// Screen mode.
///
/// Negative values windowed.
/// Positive values fullscreen.
#define DISPLAY_MODE -720
#endif

/// \cond
#if (0 > (DISPLAY_MODE))
#define SCREEN_F 0
#define SCREEN_H (-(DISPLAY_MODE))
#elif (0 < (DISPLAY_MODE))
#define SCREEN_F 1
#define SCREEN_H (DISPLAY_MODE)
#else
#error "invalid display mode (pre)"
#endif
#if ((800 == SCREEN_H) || (1200 == SCREEN_H))
#define SCREEN_W ((SCREEN_H / 10) * 16)
#else
#define SCREEN_W (((SCREEN_H * 16) / 9) - (((SCREEN_H * 16) / 9) % 4))
#endif
/// \endcond

/// Size of one sample in bytes.
#define AUDIO_SAMPLE_SIZE 4

/// \cond
#if (4 == AUDIO_SAMPLE_SIZE)
#define AUDIO_SAMPLE_TYPE_SDL AUDIO_F32SYS
typedef float sample_t;
#elif (2 == AUDIO_SAMPLE_SIZE)
#define AUDIO_SAMPLE_TYPE_SDL AUDIO_S16SYS
typedef int16_t sample_t;
#elif (1 == AUDIO_SAMPLE_SIZE)
#define AUDIO_SAMPLE_TYPE_SDL AUDIO_U8
typedef uint8_t sample_t;
#else
#error "invalid audio sample size"
#endif
#define AUDIO_POSITION_SHIFT (9 - (4 / sizeof(sample_t)))
/// \endcond

/// Audio channels.
#define AUDIO_CHANNELS 2

/// Audio samplerate.
#define AUDIO_SAMPLERATE 44100

/// Audio byterate.
#define AUDIO_BYTERATE (AUDIO_CHANNELS * AUDIO_SAMPLERATE * AUDIO_SAMPLE_SIZE)

/// Intro length (in bytes of audio).
#define INTRO_LENGTH (87 * AUDIO_BYTERATE)

/// Intro start position (in seconds).
#define INTRO_START (0 * AUDIO_BYTERATE)

/// Noise axis (2D).
#define NOISE_AXIS_2D 512
/// Noise axis (3D).
#define NOISE_AXIS_3D 128
/// Noise size.
/// Must be the larger of 2D and 3D noise size.
#define NOISE_SIZE (sizeof(uint8_t) * NOISE_AXIS_3D * NOISE_AXIS_3D * NOISE_AXIS_3D)
/// Snow complexity.
#define SNOW_COUNT 131072
/// Snow draw call complexity.
#define SNOW_VERTEX_COUNT (SNOW_COUNT * 4)
/// Tree complexity.
#define TREE_COUNT 262144
/// Tree draw call complexity.
#define TREE_VERTEX_COUNT (TREE_COUNT * 4)
/// Random fill size (8 megabytes).
#define RAND_FILL_SIZE (TREE_COUNT * 4 * 4 * 2)

/// \cond
#define STARTING_POS_X 0.0f
#define STARTING_POS_Y 0.0f
#define STARTING_POS_Z 0.0f
#define STARTING_FW_X 0.0f
#define STARTING_FW_Y 0.0f
#define STARTING_FW_Z 1.0f
#define STARTING_UP_X 0.0f
#define STARTING_UP_Y 1.0f
#define STARTING_UP_Z 0.0f
#define STARTING_DOF 0.0f
/// \endcond

/// Precalculated file to load if synth cannot be ran.
#define SYNTH_TEST_INPUT_FILE "adarkar_wastes.wav"
#if 0 // Change to 1 to enable writing synth output.
/// Output file to write for synth test.
#define SYNTH_TEST_OUTPUT_FILE SYNTH_TEST_INPUT_FILE
#endif

//######################################
// Global data #########################
//######################################

/// Audio buffer for output.
static uint8_t g_audio_buffer[INTRO_LENGTH * 9 / 8];

/// Current audio position.
static int g_audio_position = INTRO_START;

/// Window.
SDL_Window *g_sdl_window;

#if defined(USE_LD)

/// \cond
static float g_pos_x = STARTING_POS_X;
static float g_pos_y = STARTING_POS_Y;
static float g_pos_z = STARTING_POS_Z;
static float g_fw_x = STARTING_FW_X;
static float g_fw_y = STARTING_FW_Y;
static float g_fw_z = STARTING_FW_Z;
static float g_up_x = STARTING_UP_X;
static float g_up_y = STARTING_UP_Y;
static float g_up_z = STARTING_UP_Z;
static float g_dof = STARTING_DOF;
/// \endcond

/// Developer mode global toggle.
static bool g_flag_developer = false;

/// Follow direction global toggle.
static bool g_direction_lock = true;

/// Usage string.
static const char *usage = ""
"Usage: adarkar_wastes <options>\n"
"For ??? ??? compo.\n"
"Release version does not pertain to any size limitations.\n";

#else

/// Developer mode disabled.
#define g_flag_developer 0

#endif

//######################################
// Utility #############################
//######################################

#if defined(USE_LD)

/// \brief Random float value.
///
/// \param op Given maximum value.
/// \return Random value between 0 and given value.
static float frand(float op)
{
  return static_cast<float>(dnload_rand() & 0xFFFF) * ((1.0f / 65535.0f) * op);
}

/// Round to integer.
///
/// \param op Floating-point value.
/// \return Rounded integer value.
static GLint iround(float op)
{
  return static_cast<GLint>(std::lround(op));
}

/// Liner mix of two values.
///
/// \param pa Param A.
/// \param pb Param B.
/// \param mixer Mixer value.
float mix(int pa, int pb, float mixer)
{
  return static_cast<float>(pb - pa) * mixer + static_cast<float>(pa);
}

#endif

/// Random int16_t value.
///
/// \return Random signed 16-bit integer value.
static int16_t i16rand()
{
  return static_cast<int16_t>(dnload_rand() & 0xFFFF);
}

/// Random seed value (8-bit).
///
/// \param Random seed value.
static void srand8(int8_t op)
{
  void (*srand_func)(int) = reinterpret_cast<void(*)(int)>(dnload_srand);
  srand_func(op);
}

//######################################
// Music ###############################
//######################################

/// \brief Update audio stream.
///
/// \param userdata Not used.
/// \param stream Target stream.
/// \param len Number of bytes to write.
static void audio_callback(void *userdata, Uint8 *stream, int len)
{
  (void)userdata;

  for(int ii = 0; (ii < len); ++ii)
  {
    stream[ii] = g_audio_buffer[g_audio_position + ii];
  }
  g_audio_position += len;
}

/// SDL audio specification struct.
static SDL_AudioSpec g_audio_spec =
{
  AUDIO_SAMPLERATE,
  AUDIO_SAMPLE_TYPE_SDL,
  AUDIO_CHANNELS,
  0,
#if defined(USE_LD)
  4096,
#else
  256, // ~172.3Hz, lower values seem to cause underruns
#endif
  0,
  0,
  audio_callback,
  NULL
};

/// Synth wrapper.
/// \param outbuf Output buffer.
/// \param samples Number of bytes to write.
static void synth(uint8_t *outbuf, unsigned bytes)
{
#if defined(USE_LD)
#if defined(SYNTH_TEST_INPUT_FILE)
  const fs::path REL_PATH("rel");
  const fs::path SYNTH_TEST_INPUT_FILE_PATH(SYNTH_TEST_INPUT_FILE);
  SDL_AudioSpec wav_spec;
  SDL_AudioSpec* wav_found;
  Uint32 wav_length;
  Uint8 *wav_buffer;

  wav_found = SDL_LoadWAV(SYNTH_TEST_INPUT_FILE_PATH.string().c_str(), &wav_spec, &wav_buffer, &wav_length);
  if(!wav_found)
  {
    fs::path fname = REL_PATH / SYNTH_TEST_INPUT_FILE_PATH;
    wav_found = SDL_LoadWAV(fname.string().c_str(), &wav_spec, &wav_buffer, &wav_length);
  }
  if(!wav_found)
  {
    fs::path fname = fs::path("..") / REL_PATH / SYNTH_TEST_INPUT_FILE_PATH;
    wav_found = SDL_LoadWAV(fname.string().c_str(), &wav_spec, &wav_buffer, &wav_length);
  }

  // If successful, read floating-point data from file.
  if(wav_found)
  {
    memcpy(g_audio_buffer, wav_buffer, std::min(static_cast<size_t>(wav_length), sizeof(g_audio_buffer)));
    SDL_FreeWAV(wav_buffer);
  }
  // No no audio data available, fill data with 0.
  else
#endif
  {
    std::cerr << "WARNING: synth not available, filling audio with 0" << std::endl;
    while(bytes > 0)
    {
      outbuf[bytes - 1] = 0;
      --bytes;
    }
  }

#else
  synth(reinterpret_cast<float*>(outbuf), bytes / sizeof(sample_t));

  // Write raw .wav file if necessary.
#if defined(SYNTH_TEST_OUTPUT_FILE)
  {
    SF_INFO info;
    info.samplerate = AUDIO_SAMPLERATE;
    info.channels = AUDIO_CHANNELS;
    info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

    SNDFILE *outfile = dnload_sf_open(SYNTH_TEST_OUTPUT_FILE, SFM_WRITE, &info);
    sf_count_t write_count = INTRO_LENGTH / AUDIO_CHANNELS / AUDIO_SAMPLE_SIZE;
    dnload_sf_writef_float(outfile, reinterpret_cast<float*>(g_audio_buffer), write_count);
    dnload_sf_close(outfile);
  }
#endif
#endif
}

//######################################
// Shaders #############################
//######################################

#include "header.glsl.hpp" // g_shader_header
#include "snow.vert.glsl.hpp" // g_shader_vertex_snow
#include "snow.frag.glsl.hpp" // g_shader_fragment_snow
#include "tree.vert.glsl.hpp" // g_shader_vertex_tree
#include "tree.frag.glsl.hpp" // g_shader_fragment_tree
#include "terrain.vert.glsl.hpp" // g_shader_vertex_terrain
#include "terrain.frag.glsl.hpp" // g_shader_fragment_terrain
#include "quad.vert.glsl.hpp" // g_shader_vertex_quad
#include "quad.frag.glsl.hpp" // g_shader_fragment_quad

/// Fixed uniform location.
static const GLint g_uniform_u = 0;
/// Fixed uniform location.
static const GLint g_uniform_noise_2d = 6;
/// Fixed uniform location.
static const GLint g_uniform_noise_3d = 7;
/// Fixed uniform location.
static const GLint g_uniform_color = 8;
/// Fixed uniform location.
static const GLint g_uniform_depth = 9;
#if defined(USE_LD)
/// Fixed uniform location.
static const GLint g_uniform_mat = 10;
/// Fixed uniform location.
static const GLint g_uniform_screen_size = 11;
#endif

/// Programs.
GLuint g_programs[4];

/// Shader program.
#define g_program_snow g_programs[0]
/// Shader program.
#define g_program_tree g_programs[1]
/// Shader program.
#define g_program_terrain g_programs[2]
/// Shader program.
#define g_program_quad g_programs[3]

#if !defined(USE_LD)

/// Create program.
///
/// \param vertex Vertex shader.
/// \param geometry Geometry shader, may be NULL.
/// \param fragment Fragment shader.
/// \return Program ID.
static GLuint program_create(const char *vertex, const char *fragment)
{
  GLuint ret = dnload_glCreateProgram();

  const GLchar* g_shader_source_array[] = { g_shader_header, NULL };

  GLuint shader_vert = dnload_glCreateShader(GL_VERTEX_SHADER);
  g_shader_source_array[1] = vertex;
  dnload_glShaderSource(shader_vert, 2, g_shader_source_array, NULL);
  dnload_glCompileShader(shader_vert);
  dnload_glAttachShader(ret, shader_vert);

  GLuint shader_frag = dnload_glCreateShader(GL_FRAGMENT_SHADER);
  g_shader_source_array[1] = fragment;
  dnload_glShaderSource(shader_frag, 2, g_shader_source_array, NULL);
  dnload_glCompileShader(shader_frag);
  dnload_glAttachShader(ret, shader_frag);

  dnload_glLinkProgram(ret);

  return ret;
}

#endif

//######################################
// Drawing #############################
//######################################

#if !defined(SYNTH_WORK_SIZE)
// Work size at least enough for other elements.
#define SYNTH_WORK_SIZE (48 * 1024 * 1024)
#endif

/// Uniform buffer.
/// 0-2: Position 1.
/// 3-5: Position 2.
/// 6-8: Forward.
/// 9-11: Up.
/// 12: Depth of Field.
/// 13: Time of Day.
/// 14: -
/// 15: Scene time.
/// 16: Scene length.
/// 17: Global time.static GLint g_uniform_array[18];
static GLint g_uniform_array[18];

#if defined(USE_LD)

/// Matrix uniform array.
///
/// One 3x3 matrix.
static float g_uniform_matrix[9] =
{
  -0.99f, -0.16f, 0.02f,
  0.14f, -0.77f, 0.63f,
  -0.08f, 0.62f, 0.78f
};

#endif

/// Framebuffer.
static GLuint g_fbo;

/// All textures.
static GLuint g_textures[4];

/// Texture index.
#define TEXTURE_NOISE_2D_INDEX 0
/// Texture index.
#define TEXTURE_NOISE_3D_INDEX 1
/// Texture index.
#define TEXTURE_COLOR_INDEX 2
/// Texture index.
#define TEXTURE_DEPTH_INDEX 3

/// Direction.
/// 0-2: Start position.
/// 3-5: End position.
/// 6: Random seed for up.
/// 7: Random seed for forward.
/// 8: DoF value.
/// 9: Scene length.
int8_t g_direction[] =
{
  4, 3, -15, 5, 3, -15, -21, -121, 92, 44,
  -4, 4, 14, -4, 4, 15, -95, -121, -75, 58,
  14, 4, 42, 18, 4, 54, 95, -54, -116, 42,
  23, 3, 53, 24, 3, 54, 37, 30, 37, 60,
  89, 13, 106, 89, 15, 105, 83, 35, -12, 44,
  60, 5, -4, 59, 5, -3, -21, -54, 111, 44,
  42, 9, 8, 40, 8, 11, 83, -54, 92, 58,
  -1, 5, 7, -2, 6, 7, 117, 100, -116, 44,
  -10, 7, -10, -8, 13, -8, 35, 73, 111, 76,
};

/// Common buffer for data.
/// This buffer needs to be large enough for any of the following at one time:
/// - Noise textures.
/// - Uniform buffer and persistent vertex buffers.
/// - Synth scratchpad (determines buffer size).
static uint8_t g_buffer[SYNTH_WORK_SIZE];

/// Snow/tree array.
///
/// Arranged as:
/// 0: Position X.
/// 1: Position Y.
/// 2: Position Z.
/// 3: Corner index, [0, 3].
///
/// One flake repeats this 4 times, with position remaining constant and corner index running.
static int16_t* g_snow_array = reinterpret_cast<int16_t*>(g_buffer);

/// Terrain data.
static int8_t g_terrain[] =
{
  -1, -1,
  1, -1,
  1, 1,
  -1, 1
};

/// Bind a program and issue generic uniforms.
///
static void bind_program(GLuint program)
{
  dnload_glUseProgram(program);
  dnload_glUniform3iv(g_uniform_u, 6, g_uniform_array);
  dnload_glUniform1i(g_uniform_noise_2d, TEXTURE_NOISE_2D_INDEX);
  dnload_glUniform1i(g_uniform_noise_3d, TEXTURE_NOISE_3D_INDEX);
#if defined(USE_LD)
  glUniformMatrix3fv(g_uniform_mat, 1, GL_FALSE, g_uniform_matrix);
#endif
}

/// Draw the world.
///
/// \param ticks Tick count.
/// \param aspec Screen aspect.
static void draw(GLsizei screen_w, GLsizei screen_h, int ticks)
{
#if defined(USE_LD)
  if(g_flag_developer && !g_direction_lock)
  {
    g_uniform_array[0] = iround(g_pos_x),
    g_uniform_array[1] = iround(g_pos_y) - 1,
    g_uniform_array[2] = iround(g_pos_z),
    g_uniform_array[3] = iround(g_pos_x),
    g_uniform_array[4] = iround(g_pos_y) + 1,
    g_uniform_array[5] = iround(g_pos_z),
    g_uniform_array[6] = iround(g_fw_x * 1000.0f),
    g_uniform_array[7] = iround(g_fw_y * 1000.0f),
    g_uniform_array[8] = iround(g_fw_z * 1000.0f),
    g_uniform_array[9] = iround(g_up_x * 1000.0f),
    g_uniform_array[10] = iround(g_up_y * 1000.0f),
    g_uniform_array[11] = iround(g_up_z * 1000.0f),
    g_uniform_array[12] = iround(g_dof),
    g_uniform_array[13] = 0;
    g_uniform_array[14] = 0;
    g_uniform_array[15] = 1;
    g_uniform_array[16] = 2;
    g_uniform_array[17] = ticks;
  }
  else
#endif
  {
    int tick_iter = ticks;
    int8_t* direction = g_direction;

    for(;;)
    {
      int scene_length = static_cast<int>(reinterpret_cast<uint8_t*>(direction)[9]) << 16;

      if(scene_length > tick_iter)
      {
        int32_t* ptr = reinterpret_cast<int32_t*>(g_uniform_array);
        ptr[0] = static_cast<int>(direction[0]) << 3;
        ptr[1] = static_cast<int>(direction[1]) << 3;
        ptr[2] = static_cast<int>(direction[2]) << 3;
        ptr[3] = static_cast<int>(direction[3]) << 3;
        ptr[4] = static_cast<int>(direction[4]) << 3;
        ptr[5] = static_cast<int>(direction[5]) << 3;
        srand8(direction[6]);
        ptr[6] = i16rand();
        ptr[7] = i16rand();
        ptr[8] = i16rand();
        srand8(direction[7]);
        ptr[9] = i16rand();
        ptr[10] = i16rand();
        ptr[11] = i16rand();
        ptr[12] = reinterpret_cast<uint8_t*>(direction)[8];
        ptr[13] = 0;
        ptr[14] = 0;
        ptr[15] = tick_iter;
        ptr[16] = scene_length;
        ptr[17] = ticks;
        break;
      }

      tick_iter -= scene_length;
      direction += 10;
    }

#if defined(USE_LD)
    if(g_flag_developer)
    {
      g_fw_x = static_cast<float>(g_uniform_array[6]);
      g_fw_y = static_cast<float>(g_uniform_array[7]);
      g_fw_z = static_cast<float>(g_uniform_array[8]);
      float fwlen = sqrtf((g_fw_x * g_fw_x) + (g_fw_y * g_fw_y) + (g_fw_z * g_fw_z));
      g_fw_x /= fwlen;
      g_fw_y /= fwlen;
      g_fw_z /= fwlen;

      g_up_x = static_cast<float>(g_uniform_array[9]);
      g_up_y = static_cast<float>(g_uniform_array[10]);
      g_up_z = static_cast<float>(g_uniform_array[11]);
      float uplen = sqrtf((g_up_x * g_up_x) + (g_up_y * g_up_y) + (g_up_z * g_up_z));
      g_up_x /= uplen;
      g_up_y /= uplen;
      g_up_z /= uplen;

      g_uniform_array[0] += iround(g_pos_x);
      g_uniform_array[1] += iround(g_pos_y);
      g_uniform_array[2] += iround(g_pos_z);
      g_uniform_array[3] += iround(g_pos_x);
      g_uniform_array[4] += iround(g_pos_y);
      g_uniform_array[5] += iround(g_pos_z);

      g_uniform_array[12] += static_cast<int>(g_dof);
    }
#endif
  }

  // Offscreen.
  {
    dnload_glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
    dnload_glViewport(0, 0, screen_w << 1, screen_h << 1);
    dnload_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw snow.
    {
      bind_program(g_program_snow);
#if defined(USE_LD)
      glUniform2i(g_uniform_screen_size, screen_w, screen_h);
#endif

      dnload_glVertexAttribPointer(0, 4, GL_SHORT, GL_FALSE, sizeof(int16_t) * 4, g_snow_array);
      dnload_glDrawArrays(GL_QUADS, 0, SNOW_VERTEX_COUNT);
    }

    // Draw trees.
    {
      bind_program(g_program_tree);
#if defined(USE_LD)
      glUniform2i(g_uniform_screen_size, screen_w, screen_h);
#endif

#if defined(USE_LD)
      // Same as snow.
      dnload_glVertexAttribPointer(0, 4, GL_SHORT, GL_FALSE, sizeof(int16_t) * 4, g_snow_array);
#endif
      dnload_glDrawArrays(GL_QUADS, 0, TREE_VERTEX_COUNT);
    }

    // Draw terrain.
    {
      bind_program(g_program_terrain);
#if defined(USE_LD)
      glUniform2i(g_uniform_screen_size, screen_w, screen_h);
#endif

      dnload_glVertexAttribPointer(0, 2, GL_BYTE, GL_FALSE, sizeof(int8_t) * 2, g_terrain);
      dnload_glDrawArraysInstanced(GL_QUADS, 0, 4, 294912);
    }
  }

  // On-screen.
  {
    dnload_glBindFramebuffer(GL_FRAMEBUFFER, 0);
    dnload_glViewport(0, 0, screen_w, screen_h);
    dnload_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if defined(USE_LD)
    glActiveTexture(GL_TEXTURE0 + TEXTURE_COLOR_INDEX);
    glBindTexture(GL_TEXTURE_2D, g_textures[TEXTURE_COLOR_INDEX]);
    glGenerateMipmap(GL_TEXTURE_2D);
#else
    dnload_glGenerateTextureMipmap(g_textures[TEXTURE_COLOR_INDEX]);
#endif

    // Draw postproc quad.
    {
      bind_program(g_program_quad);
      dnload_glUniform1i(g_uniform_color, TEXTURE_COLOR_INDEX);
      dnload_glUniform1i(g_uniform_depth, TEXTURE_DEPTH_INDEX);
#if defined(USE_LD)
      glUniform2i(g_uniform_screen_size, screen_w, screen_h);
#endif

#if defined(USE_LD)
      // Same as terrain.
      dnload_glVertexAttribPointer(0, 2, GL_BYTE, GL_FALSE, sizeof(int8_t) * 2, g_terrain);
#endif
      dnload_glDrawArrays(GL_QUADS, 0, 4);
    }
  }
}

//######################################
// Utility #############################
//######################################

#if defined(USE_LD)

/// Find random seed most closely resembling given direction.
///
/// \param px X component.
/// \param py Y component.
/// \param pz Z component.
/// \return Random number most closely representing given number.
int8_t find_rand_dir(float px, float py, float pz)
{
  float dlen = sqrtf((px * px) + (py * py) + (pz * pz));
  px /= dlen;
  py /= dlen;
  pz /= dlen;

  float best_difference = std::numeric_limits<float>::max();
  int8_t ret = 0;

  for(int ii = -128; (ii < 128); ++ii)
  {
    int8_t seed = static_cast<int8_t>(ii);
    srand8(seed);

    float cx = static_cast<float>(i16rand());
    float cy = static_cast<float>(i16rand());
    float cz = static_cast<float>(i16rand());

    float clen = sqrtf((cx * cx) + (cy * cy) + (cz * cz));
    cx /= clen;
    cy /= clen;
    cz /= clen;

    float xdiff = cx - px;
    float ydiff = cy - py;
    float zdiff = cz - pz;
    float sqrdiff = (xdiff * xdiff) + (ydiff * ydiff) + (zdiff * zdiff);

    if(sqrdiff < best_difference)
    {
      best_difference = sqrdiff;
      ret = seed;
    }
  }

  return ret;
}

/// Generate a ZXY rotation matrix.
///
/// \param rx Rotation x.
/// \param ry Rotation y.
/// \param rz Rotation z.
/// \param out_matrix Matrix to write into.
static void generate_rotation_matrix_zxy(float rx, float ry, float rz, float *out_matrix)
{
  float sx = sinf(rx);
  float sy = sinf(ry);
  float sz = sinf(rz);
  float cx = cosf(rx);
  float cy = cosf(ry);
  float cz = cosf(rz);

  out_matrix[0] = sx * sy * sz + cy * cz;
  out_matrix[1] = sz * cx;
  out_matrix[2] = sx * sz * cy - sy * cz;
  out_matrix[3] = sx * sy * cz - sz * cy;
  out_matrix[4] = cx * cz;
  out_matrix[5] = sx * cy * cz + sy * sz;
  out_matrix[6] = sy * cx;
  out_matrix[7] = -sx;
  out_matrix[8] = cx * cy;
}

/// Parse resolution from string input.
///
/// \param op Resolution string.
/// \return Tuple of width and height.
boost::tuple<unsigned, unsigned> parse_resolution(const std::string &op)
{
  size_t cx = op.find("x");
  
  if(std::string::npos == cx)
  {
    cx = op.rfind("p");

    if((std::string::npos == cx) || (0 >= cx))
    {
      std::ostringstream sstr;
      sstr << "invalid resolution string '" << op << '\'';
      BOOST_THROW_EXCEPTION(std::runtime_error(sstr.str()));
    }

    std::string sh = op.substr(0, cx);

    unsigned rh = boost::lexical_cast<unsigned>(sh);
    unsigned rw = (rh * 16) / 9;
    unsigned rem4 = rw % 4;

    return boost::make_tuple(rw - rem4, rh);
  }

  std::string sw = op.substr(0, cx);
  std::string sh = op.substr(cx + 1);

  return boost::make_tuple(boost::lexical_cast<int>(sw), boost::lexical_cast<int>(sh));
}

/// \brief Audio writing callback.
///
/// \param data Raw audio data.
/// \param size Audio data size (in samples).
void write_audio(void *data, size_t size)
{
  FILE *fd = fopen("intro.raw", "wb");

  if(fd != NULL)
  {
    fwrite(data, size, 1, fd);
  }

  fclose(fd);
  return;
}

/// \brief Image writing callback.
///
/// \param screen_w Screen width.
/// \param screen_h Screen height.
/// \param idx Frame index to write.
void write_frame(unsigned screen_w, unsigned screen_h, unsigned idx)
{
  boost::scoped_array<uint8_t> image(new uint8_t[screen_w * screen_h * 3]);
  std::ostringstream sstr;

  glReadPixels(0, 0, static_cast<GLsizei>(screen_w), static_cast<GLsizei>(screen_h), GL_RGB, GL_UNSIGNED_BYTE,
      image.get());

  sstr << "adarkar_wastes_" << std::setfill('0') << std::setw(4) << idx << ".png";

  gfx::image_png_save(sstr.str(), screen_w, screen_h, 24, image.get());
  return;
}

#endif

//######################################
// intro / _start ######################
//######################################

#if defined(USE_LD)
/// \brief Intro body function.
///
/// \param screen_w Screen width.
/// \param screen_h Screen height.
/// \param flag_fullscreen Fullscreen toggle.
/// \param flag_record Record toggle.
void intro(unsigned screen_w, unsigned screen_h, bool flag_fullscreen, bool flag_record)
#else
#define screen_w static_cast<unsigned>(SCREEN_W)
#define screen_h static_cast<unsigned>(SCREEN_H)
#define flag_fullscreen static_cast<bool>(SCREEN_F)
void _start()
#endif
{
  dnload();
  dnload_SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  g_sdl_window = dnload_SDL_CreateWindow(NULL, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      static_cast<int>(screen_w), static_cast<int>(screen_h),
      SDL_WINDOW_OPENGL | (flag_fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
  dnload_SDL_GL_CreateContext(g_sdl_window);
  dnload_SDL_ShowCursor(g_flag_developer);

#if defined(USE_LD)
  {
    GLenum err = glewInit();
    if(GLEW_OK != err)
    {
      std::cerr << "glewInit(): " << glewGetErrorString(err) << std::endl;
      exit(1);
    }
  }
#endif

  // Generate music.
  synth(g_audio_buffer, INTRO_LENGTH);

  // Generate textures.
  dnload_glGenTextures(4, g_textures);

  // Noise texture preparation.
  {
    dnload_srand(0);

    for(unsigned ii = 0; (ii < RAND_FILL_SIZE); ++ii)
    {
      g_buffer[ii] = static_cast<uint8_t>(dnload_rand());
    }

    dnload_glActiveTexture(GL_TEXTURE0 + TEXTURE_NOISE_2D_INDEX);
    dnload_glBindTexture(GL_TEXTURE_2D, g_textures[TEXTURE_NOISE_2D_INDEX]);
    dnload_glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, NOISE_AXIS_2D, NOISE_AXIS_2D, 0, GL_RED,
        GL_UNSIGNED_BYTE, g_buffer);
    dnload_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#if defined(USE_LD)
    dnload_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    dnload_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    dnload_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif

    dnload_glActiveTexture(GL_TEXTURE0 + TEXTURE_NOISE_3D_INDEX);
    dnload_glBindTexture(GL_TEXTURE_3D, g_textures[TEXTURE_NOISE_3D_INDEX]);
    dnload_glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, NOISE_AXIS_3D, NOISE_AXIS_3D, NOISE_AXIS_3D, 0, GL_RED,
        GL_UNSIGNED_BYTE, g_buffer);
    dnload_glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#if defined(USE_LD)
    dnload_glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    dnload_glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    dnload_glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    dnload_glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
  }

  // Color texture preparation.
  dnload_glActiveTexture(GL_TEXTURE0 + TEXTURE_COLOR_INDEX);
  dnload_glBindTexture(GL_TEXTURE_2D, g_textures[TEXTURE_COLOR_INDEX]);
  dnload_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(screen_w << 1),
      static_cast<GLsizei>(screen_h << 1), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  dnload_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  dnload_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  dnload_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
#if defined(USE_LD)
  dnload_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif

  // Depth texture preparation.
  dnload_glActiveTexture(GL_TEXTURE0 + TEXTURE_DEPTH_INDEX);
  dnload_glBindTexture(GL_TEXTURE_2D, g_textures[TEXTURE_DEPTH_INDEX]);
  dnload_glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, static_cast<GLsizei>(screen_w << 1),
      static_cast<GLsizei>(screen_h << 1), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  dnload_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  dnload_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  dnload_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#if defined(USE_LD)
  dnload_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif

#if defined(USE_LD)
  glGenFramebuffers(1, &g_fbo);
  {
    glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, g_textures[TEXTURE_COLOR_INDEX], 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, g_textures[TEXTURE_DEPTH_INDEX], 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
      std::ostringstream sstr;
      sstr << "framebuffer " << g_fbo << " not complete: " << status;
      BOOST_THROW_EXCEPTION(std::runtime_error(sstr.str()));
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
#else
  dnload_glCreateFramebuffers(1, &g_fbo);
  dnload_glNamedFramebufferTexture(g_fbo, GL_COLOR_ATTACHMENT0, g_textures[TEXTURE_COLOR_INDEX], 0);
  dnload_glNamedFramebufferTexture(g_fbo, GL_DEPTH_ATTACHMENT, g_textures[TEXTURE_DEPTH_INDEX], 0);
#endif

#if defined(USE_LD)
  GlslProgram program_terrain;
  program_terrain.addShader(GL_VERTEX_SHADER, g_shader_header, g_shader_vertex_terrain);
  program_terrain.addShader(GL_FRAGMENT_SHADER, g_shader_header, g_shader_fragment_terrain);
  if(!program_terrain.link())
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("program creation failure (terrain)"));
  }
  g_program_terrain = program_terrain.getId();
  GlslProgram program_tree;
  program_tree.addShader(GL_VERTEX_SHADER, g_shader_header, g_shader_vertex_tree);
  program_tree.addShader(GL_FRAGMENT_SHADER, g_shader_header, g_shader_fragment_tree);
  if(!program_tree.link())
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("program creation failure (tree)"));
  }
  g_program_tree = program_tree.getId();
  GlslProgram program_snow;
  program_snow.addShader(GL_VERTEX_SHADER, g_shader_header, g_shader_vertex_snow);
  program_snow.addShader(GL_FRAGMENT_SHADER, g_shader_header, g_shader_fragment_snow);
  if(!program_snow.link())
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("program creation failure (snow)"));
  }
  g_program_snow = program_snow.getId();
  GlslProgram program_quad;
  program_quad.addShader(GL_VERTEX_SHADER, g_shader_header, g_shader_vertex_quad);
  program_quad.addShader(GL_FRAGMENT_SHADER, g_shader_header, g_shader_fragment_quad);
  if(!program_quad.link())
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("program creation failure (quad)"));
  }
  g_program_quad = program_quad.getId();
#else
  g_program_snow = program_create(g_shader_vertex_snow, g_shader_fragment_snow);
  g_program_tree = program_create(g_shader_vertex_tree, g_shader_fragment_tree);
  g_program_terrain = program_create(g_shader_vertex_terrain, g_shader_fragment_terrain);
  g_program_quad = program_create(g_shader_vertex_quad, g_shader_fragment_quad);
#endif

  // Non-changing GL state.
  dnload_glEnable(GL_CULL_FACE);
  dnload_glEnable(GL_DEPTH_TEST);
  dnload_glEnableVertexAttribArray(0);

  // Data is already random post-texture. Set snow parameters.
  for(unsigned ii = 0; (ii < (TREE_VERTEX_COUNT * 4 * 2)); ii += 4 * 4 * 2)
  {
    uint8_t* u8arr = reinterpret_cast<uint8_t*>(g_snow_array) + ii;
    uint32_t* u32arr = reinterpret_cast<uint32_t*>(u8arr);
    uint32_t r1 = u32arr[0];
    uint32_t r2 = u32arr[1];
    u32arr[2] = r1;
    u32arr[3] = r2;
    u32arr[4] = r1;
    u32arr[5] = r2;
    u32arr[6] = r1;
    u32arr[7] = r2;
    uint16_t* u16arr = reinterpret_cast<uint16_t*>(u8arr);
    u16arr[3] = 0;
    u16arr[7] = 1;
    u16arr[11] = 3;
    u16arr[15] = 2;
  }

#if defined(USE_LD)
  if(flag_record)
  {
    SDL_Event event;
    unsigned frame_idx = 0;

    // audio
    SDL_PauseAudio(1);

    // video
    for(;;)
    {
      int ticks = static_cast<int>(static_cast<float>(frame_idx) / 60.0f * static_cast<float>(AUDIO_BYTERATE));

      if(ticks > INTRO_LENGTH)
      {
        break;
      }

      if(SDL_PollEvent(&event) && (event.type == SDL_KEYDOWN) && (event.key.keysym.sym == SDLK_ESCAPE))
      {
        break;
      }

      draw(static_cast<GLsizei>(screen_w), static_cast<GLsizei>(screen_h), ticks);
      write_frame(screen_w, screen_h, frame_idx);
      SDL_GL_SwapWindow(g_sdl_window);
      ++frame_idx;
    }

    SDL_Quit();
    return;
  }

  if(!g_flag_developer)
  {
    SDL_OpenAudio(&g_audio_spec, NULL);
    SDL_PauseAudio(0);
  }
#else
  dnload_SDL_OpenAudio(&g_audio_spec, NULL);
  dnload_SDL_PauseAudio(0);
#endif

#if defined(USE_LD)
  int start_ticks = static_cast<int>(SDL_GetTicks());
  const float MOVE_SPEED_SLOW = 8.0f / 60.0f;
  const float MOVE_SPEED_FAST = 8.0f / 6.0f;
  const int TIME_SPEED_FAST = 10;
  float current_time = 0.0f;
  float stored_orientation[9] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  float move_speed = MOVE_SPEED_SLOW;
  int time_speed = 1;
  int8_t move_backward = 0;
  int8_t move_down = 0;
  int8_t move_forward = 0;
  int8_t move_left = 0;
  int8_t move_right = 0;
  int8_t move_up = 0;
  int8_t time_delta = 0;
  int8_t dof_delta = 0;
  bool mouse_look = false;
  bool stored_orientation_active = false;
#endif

  for(;;)
  {
#if defined(USE_LD)
    int mouse_look_x = 0;
    int mouse_look_y = 0;
    int time_skip = 0;
    bool quit = false;
#endif
    SDL_Event event;
    int curr_ticks;

#if defined(USE_LD)
    while(SDL_PollEvent(&event))
    {
      if(SDL_QUIT == event.type)
      {
        quit = true;
      }
      else if(SDL_KEYDOWN == event.type)
      {
        switch(event.key.keysym.sym)
        {
          case SDLK_a:
            move_left = 1;
            break;

          case SDLK_d:
            move_right = 1;
            break;

          case SDLK_e:
            move_up = 1;
            break;

          case SDLK_q:
            move_down = 1;
            break;

          case SDLK_s:
            move_backward = 1;
            break;

          case SDLK_w:
            move_forward = 1;
            break;

          case SDLK_z:
            dof_delta = -1;
            break;

          case SDLK_x:
            dof_delta = 1;
            break;

          case SDLK_LEFT:
            time_skip = 5000;
            break;

          case SDLK_DOWN:
            time_skip = 30000;
            break;

          case SDLK_RIGHT:
            time_skip = -5000;
            break;

          case SDLK_UP:
            time_skip = -30000;
            break;

          case SDLK_LSHIFT:
          case SDLK_RSHIFT:
            move_speed = MOVE_SPEED_FAST;
            time_speed = TIME_SPEED_FAST;
            break;            

          case SDLK_LALT:
            time_delta = -1;
            break;

          case SDLK_MODE:
          case SDLK_RALT:
            time_delta = 1;
            break;

          case SDLK_INSERT:
            {
              float rx = frand(static_cast<float>(M_PI * 2.0));
              float ry = frand(static_cast<float>(M_PI * 2.0));
              float rz = frand(static_cast<float>(M_PI * 2.0));

              generate_rotation_matrix_zxy(rx, ry, rz, g_uniform_matrix);

              printf("[ %1.2f ; %1.2f ; %1.2f ] =>\n[ %+1.2f ; %+1.2f ; %+1.2f\n"
                  "  %+1.2f ; %+1.2f ; %+1.2f\n  %+1.2f ; %+1.2f ; %+1.2f ]\n",
                  rx, ry, rz,
                  g_uniform_matrix[0], g_uniform_matrix[3], g_uniform_matrix[6],
                  g_uniform_matrix[1], g_uniform_matrix[4], g_uniform_matrix[7],
                  g_uniform_matrix[2], g_uniform_matrix[5], g_uniform_matrix[8]);
            }
            break;

          case SDLK_SPACE:
            if(!stored_orientation_active)
            {
              if(g_direction_lock)
              {
                float mixer = static_cast<float>(g_uniform_array[15]) / static_cast<float>(g_uniform_array[16]);
                float mixed_x = mix(g_uniform_array[0], g_uniform_array[3], mixer);
                float mixed_y = mix(g_uniform_array[1], g_uniform_array[4], mixer);
                float mixed_z = mix(g_uniform_array[2], g_uniform_array[5], mixer);
                float ftime = static_cast<float>(g_uniform_array[17]) / static_cast<float>(AUDIO_BYTERATE);
                int seconds = static_cast<int>(ftime);
                int milliseconds = static_cast<int>((ftime - static_cast<float>(seconds)) * 100.0f);

                std::cout << "[ " << iround(mixed_x) << ", " << iround(mixed_y) << ", " <<
                  iround(mixed_z) << " ] " << " ; " << g_uniform_array[17] << " (" << seconds << "." <<
                  milliseconds << ")" << std::endl;
              }
              else
              {
                // Store current.
                stored_orientation[0] = g_pos_x;
                stored_orientation[1] = g_pos_y;
                stored_orientation[2] = g_pos_z;
                stored_orientation[3] = g_fw_x;
                stored_orientation[4] = g_fw_y;
                stored_orientation[5] = g_fw_z;
                stored_orientation[6] = g_up_x;
                stored_orientation[7] = g_up_y;
                stored_orientation[8] = g_up_z;
                // Find closest.
                int8_t fw_rand = find_rand_dir(g_fw_x, g_fw_y, g_fw_z);
                int8_t up_rand = find_rand_dir(g_up_x, g_up_y, g_up_z);
                // Round position.
                g_pos_x = static_cast<float>(iround(g_pos_x / 8.0f)) * 8.0f;
                g_pos_y = static_cast<float>(iround(g_pos_y / 8.0f)) * 8.0f;
                g_pos_z = static_cast<float>(iround(g_pos_z / 8.0f)) * 8.0f;
                // Replace with randomized.
                srand8(fw_rand);
                g_fw_x = static_cast<float>(i16rand());
                g_fw_y = static_cast<float>(i16rand());
                g_fw_z = static_cast<float>(i16rand());
                srand8(up_rand);
                g_up_x = static_cast<float>(i16rand());
                g_up_y = static_cast<float>(i16rand());
                g_up_z = static_cast<float>(i16rand());
                int print_dof = iround(g_dof) + ((g_dof >= 128.0f) ? -128 : 0);
                // Inform.
                std::cout << "[ " << iround(g_pos_x / 8.0f) << ", " << iround(g_pos_y / 8.0f) << ", " <<
                  iround(g_pos_z / 8.0f) << " ] ; " << static_cast<int>(fw_rand) << " ; " <<
                  static_cast<int>(up_rand) << " ; " << print_dof << std::endl;
                // Mark as active.
                stored_orientation_active = true;
              }
            }
            break;

          case SDLK_RETURN:
            g_pos_x = 0.0f;
            g_pos_y = 0.0f;
            g_pos_z = 0.0f;
            g_dof = 0;
            g_direction_lock = true;
            break;

          case SDLK_F5:
            if(!program_terrain.link())
            {
              BOOST_THROW_EXCEPTION(std::runtime_error("program recreation failure (terrain)"));
            }
            g_program_terrain = program_terrain.getId();
            if(!program_tree.link())
            {
              BOOST_THROW_EXCEPTION(std::runtime_error("program recreation failure (tree)"));
            }
            g_program_tree = program_tree.getId();
            if(!program_snow.link())
            {
              BOOST_THROW_EXCEPTION(std::runtime_error("program recreation failure (snow)"));
            }
            g_program_snow = program_snow.getId();
            if(!program_quad.link())
            {
              BOOST_THROW_EXCEPTION(std::runtime_error("program recreation failure (quad)"));
            }
            g_program_quad = program_quad.getId();
            break;

          case SDLK_ESCAPE:
            quit = true;
            break;

          default:
            break;
        }
      }
      else if(SDL_KEYUP == event.type)
      {
        switch(event.key.keysym.sym)
        {
          case SDLK_a:
            move_left = 0;
            break;

          case SDLK_d:
            move_right = 0;
            break;

          case SDLK_e:
            move_up = 0;
            break;

          case SDLK_q:
            move_down = 0;
            break;

          case SDLK_s:
            move_backward = 0;
            break;

          case SDLK_w:
            move_forward = 0;
            break;

          case SDLK_z:
            dof_delta = 0;
            break;

          case SDLK_x:
            dof_delta = 0;
            break;

          case SDLK_SPACE:
            if(stored_orientation_active)
            {
              g_pos_x = stored_orientation[0];
              g_pos_y = stored_orientation[1];
              g_pos_z = stored_orientation[2];
              g_fw_x = stored_orientation[3];
              g_fw_y = stored_orientation[4];
              g_fw_z = stored_orientation[5];
              g_up_x = stored_orientation[6];
              g_up_y = stored_orientation[7];
              g_up_z = stored_orientation[8];
              stored_orientation_active = false;
            }
            break;

          case SDLK_LSHIFT:
          case SDLK_RSHIFT:
            move_speed = MOVE_SPEED_SLOW;
            time_speed = 1;
            break;            

          case SDLK_MODE:
          case SDLK_LALT:
          case SDLK_RALT:
            time_delta = 0;
            break;

          default:
            break;
        }
      }
      else if(SDL_MOUSEBUTTONDOWN == event.type)
      {
        if(g_direction_lock)
        {
          g_pos_x += static_cast<float>(g_uniform_array[0]);
          g_pos_y += static_cast<float>(g_uniform_array[1]);
          g_pos_z += static_cast<float>(g_uniform_array[2]);
          g_dof = static_cast<float>(g_uniform_array[12]);
          g_direction_lock = false;
        }
        
        if(1 == event.button.button)
        {
          mouse_look = true;
        }
      }
      else if(SDL_MOUSEBUTTONUP == event.type)
      {
        if(1 == event.button.button)
        {
          mouse_look = false;
        }
      }
      else if(SDL_MOUSEMOTION == event.type)
      {
        if(mouse_look)
        {
          mouse_look_x += event.motion.xrel;
          mouse_look_y += event.motion.yrel;
        }
      }
    }

    if(g_flag_developer)
    {
      float uplen = sqrtf(g_up_x * g_up_x + g_up_y * g_up_y + g_up_z * g_up_z);
      float fwlen = sqrtf(g_fw_x * g_fw_x + g_fw_y * g_fw_y + g_fw_z * g_fw_z);
      float rt_x;
      float rt_y;
      float rt_z;
      float movement_rt = static_cast<float>(move_right - move_left) * move_speed;
      float movement_up = static_cast<float>(move_up - move_down) * move_speed;
      float movement_fw = static_cast<float>(move_forward - move_backward) * move_speed;

      g_up_x /= uplen;
      g_up_y /= uplen;
      g_up_z /= uplen;

      g_fw_x /= fwlen;
      g_fw_y /= fwlen;
      g_fw_z /= fwlen;

      rt_x = g_fw_y * g_up_z - g_fw_z * g_up_y;
      rt_y = g_fw_z * g_up_x - g_fw_x * g_up_z;
      rt_z = g_fw_x * g_up_y - g_fw_y * g_up_x;

      if(0 != mouse_look_x)
      {
        float angle = static_cast<float>(mouse_look_x) / static_cast<float>(screen_h / 4) * 0.25f;
        float ca = cosf(angle);
        float sa = sinf(angle);
        float new_rt_x = ca * rt_x + sa * g_fw_x;
        float new_rt_y = ca * rt_y + sa * g_fw_y;
        float new_rt_z = ca * rt_z + sa * g_fw_z;
        float new_fw_x = ca * g_fw_x - sa * rt_x;
        float new_fw_y = ca * g_fw_y - sa * rt_y;
        float new_fw_z = ca * g_fw_z - sa * rt_z;

        rt_x = new_rt_x;          
        rt_y = new_rt_y;
        rt_z = new_rt_z;
        g_fw_x = new_fw_x;
        g_fw_y = new_fw_y;
        g_fw_z = new_fw_z;
      }
      if(0 != mouse_look_y)
      {
        float angle = static_cast<float>(mouse_look_y) / static_cast<float>(screen_h / 4) * 0.25f;
        float ca = cosf(angle);
        float sa = sinf(angle);
        float new_fw_x = ca * g_fw_x + sa * g_up_x;
        float new_fw_y = ca * g_fw_y + sa * g_up_y;
        float new_fw_z = ca * g_fw_z + sa * g_up_z;
        float new_up_x = ca * g_up_x - sa * g_fw_x;
        float new_up_y = ca * g_up_y - sa * g_fw_y;
        float new_up_z = ca * g_up_z - sa * g_fw_z;

        g_fw_x = new_fw_x;
        g_fw_y = new_fw_y;
        g_fw_z = new_fw_z;
        g_up_x = new_up_x;
        g_up_y = new_up_y;
        g_up_z = new_up_z;
      }

      g_pos_x += movement_rt * rt_x + movement_up * g_up_x - movement_fw * g_fw_x;
      g_pos_y += movement_rt * rt_y + movement_up * g_up_y - movement_fw * g_fw_y;
      g_pos_z += movement_rt * rt_z + movement_up * g_up_z - movement_fw * g_fw_z;
    }

    if(g_flag_developer)
    {
      current_time += static_cast<float>(AUDIO_BYTERATE) / 60.0f * static_cast<float>(time_delta * time_speed);
      if(current_time >= static_cast<float>(INTRO_LENGTH))
      {
        current_time = static_cast<float>(INTRO_LENGTH);
      }
      curr_ticks = static_cast<int>(current_time);

      float dof = std::max(std::min(g_dof + dof_delta, 255.0f), -255.0f);
      if(dof != g_dof)
      {
        int print_dof = iround(dof) + (g_direction_lock ? g_uniform_array[12] : 0);
        std::cout << "dof: " << print_dof << std::endl;
        g_dof = dof;
      }
    }
    else
    {
      int actual_ticks = static_cast<int>(SDL_GetTicks());
      
      // Adjust start time for skip.
      if(time_skip)
      {
        start_ticks += time_skip;

        if(start_ticks > actual_ticks)
        {
          start_ticks = actual_ticks;
        }
      }

      float ftime = static_cast<float>(static_cast<int>(SDL_GetTicks()) - start_ticks) / 1000.0f;

      if(time_skip)
      {
        int seconds = static_cast<int>(ftime);
        int milliseconds = static_cast<int>((ftime - static_cast<float>(seconds)) * 100.0f);
        std::cout << seconds << "." << milliseconds << std::endl;
      }

      curr_ticks = static_cast<int>(ftime * static_cast<float>(AUDIO_BYTERATE)) + INTRO_START;

      // Adjust music copy location in a similar fashion.
      if(time_skip)
      {
        g_audio_position = curr_ticks - (curr_ticks % (AUDIO_SAMPLE_SIZE * AUDIO_CHANNELS));
      }
    }

    if((curr_ticks > static_cast<int>(INTRO_LENGTH)) || quit)
    {
      break;
    }
#else
    curr_ticks = g_audio_position;

    dnload_SDL_PollEvent(&event);

    if((event.type == SDL_KEYDOWN) || (curr_ticks > INTRO_LENGTH))
    {
      break;
    }
#endif

    draw(static_cast<GLsizei>(screen_w), static_cast<GLsizei>(screen_h), curr_ticks);
    dnload_SDL_GL_SwapWindow(g_sdl_window);

#if defined(USE_LD) && 0
    {
      static prev_ticks = curr_ticks;
      if(!g_flag_developer)
      {
        const int FRAME_TIME = AUDIO_BYTERATE / 60;
        int tick_diff = curr_ticks - prev_ticks;

        if((tick_diff > (FRAME_TIME + (AUDIO_BYTERATE / 1000 * 3 / 2))) && !time_skip)
        {
          std::cout << "frameskip: " << (tick_diff - FRAME_TIME) << std::endl;
        }
      }
      prev_ticks = curr_ticks;
    }
#endif
}

  dnload_SDL_Quit();
#if !defined(USE_LD)
  asm_exit();
#endif
}

//######################################
// Main ################################
//######################################

#if defined(USE_LD)
/// Main function.
///
/// \param argc Argument count.
/// \param argv Arguments.
/// \return Program return code.
int DNLOAD_MAIN(int argc, char **argv)
{
  unsigned screen_w = SCREEN_W;
  unsigned screen_h = SCREEN_H;
  bool fullscreen = true;
  bool record = false;

  try
  {
    if(argc > 0)
    {
      po::options_description desc("Options");
      desc.add_options()
        ("developer,d", "Developer mode.")
        ("help,h", "Print help text.")
        ("record,R", "Do not play intro normally, instead save audio as .wav and frames as .png -files.")
        ("resolution,r", po::value<std::string>(), "Resolution to use, specify as 'WIDTHxHEIGHT' or 'HEIGHTp'.")
        ("window,w", "Start in window instead of full-screen.");

      po::variables_map vmap;
      po::store(po::command_line_parser(argc, argv).options(desc).run(), vmap);
      po::notify(vmap);

      if(vmap.count("developer"))
      {
        g_flag_developer = true;
      }
      if(vmap.count("help"))
      {
        std::cout << usage << desc << std::endl;
        return 0;
      }
      if(vmap.count("record"))
      {
        record = true;
      }
      if(vmap.count("resolution"))
      {
        boost::tie(screen_w, screen_h) = parse_resolution(vmap["resolution"].as<std::string>());
      }
      if(vmap.count("window"))
      {
        fullscreen = false;
      }
    }

    intro(screen_w, screen_h, fullscreen, record);
  }
  catch(const boost::exception &err)
  {
    std::cerr << boost::diagnostic_information(err);
    return 1;
  }
  catch(const std::exception &err)
  {
    std::cerr << err.what() << std::endl;
    return 1;
  }
  catch(...)
  {
    std::cerr << __FILE__ << ": unknown exception caught\n";
    return -1;
  }
  return 0;
}
#endif

//######################################
// End #################################
//######################################

