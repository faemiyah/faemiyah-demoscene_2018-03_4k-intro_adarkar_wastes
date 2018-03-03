#version 430 compatibility

layout(location=0) uniform ivec3 uniform_array[6];
layout(location=6) uniform sampler2D noise_texture_2d;
layout(location=7) uniform sampler3D noise_texture_3d;
#if defined(USE_LD)
layout(location=10) uniform mat3 noise_matrix;
layout(location=11) uniform ivec2 screen_size;
#endif

const float I_WATER_LEVEL = 18.0;
const float I_ICE_LEVEL_HIGH = 13.5;
const float I_ICE_LEVEL_LOW = 10.5;
const float I_ICE_EDGE = 3.0;
const float I_PLANE_NEAR = 0.1;
const float I_PLANE_FAR = 333.0;
const float I_TERRAIN_HEIGHT_SCALE = 66.0;
const float I_TERRAIN_PLANAR_SCALE = 0.00002;
const float I_SNOW_START = 10000000;
const float I_SNOW_END = 12800000;
const float I_NIGHT_START = 17000000;
const float I_NIGHT_END = 18200000;
const float I_TIMELAPSE = 2000000;

float ftime()
{
  float ret = uniform_array[5].z;
  return ret += smoothstep(I_NIGHT_START, I_NIGHT_END, ret) * I_TIMELAPSE;
}

float snow_state = smoothstep(I_SNOW_START, I_SNOW_END, ftime());
float night_state = smoothstep(I_NIGHT_START, I_NIGHT_END + I_TIMELAPSE, ftime());
const vec3 light = normalize(vec3(1, 0.5, 1));

float sample_noise_2d(vec2 pos)
{
  float i_spread = 2.7;
  vec2 hh = pos * i_spread;
  vec2 ii = hh * i_spread;
  vec2 jj = ii * i_spread;
  vec2 kk = jj * i_spread;
  vec2 i_ll = kk * i_spread;
  float i_aa = texture(noise_texture_2d, pos).x;
  float i_bb = texture(noise_texture_2d, hh).x;
  float i_cc = texture(noise_texture_2d, ii).x;
  float i_dd = texture(noise_texture_2d, jj).x;
  float i_ee = texture(noise_texture_2d, kk).x;
  float i_ff = texture(noise_texture_2d, i_ll).x;
  float decay = 0.38;
  return sin(i_aa) - sin(i_bb) * decay + sin(i_cc) * pow(decay, 2) - sin(i_dd) * pow(decay, 3) + sin(i_ee) * pow(decay, 4) - sin(i_ff) * pow(decay, 5);
}

float sample_noise_3d(vec3 pos)
{
#if !defined(USE_LD)
  mat3 noise_matrix = mat3(-0.99, -0.16, 0.02, 0.14, -0.77, 0.63, -0.08, 0.62, 0.78);
#endif
  float i_spread = 1.3;
  vec3 hh = noise_matrix * pos;
  vec3 ii = noise_matrix * hh * i_spread;
  vec3 jj = noise_matrix * ii * i_spread;
  vec3 kk = noise_matrix * jj * i_spread;
  vec3 ll = noise_matrix * kk * i_spread;
  vec3 i_mm = noise_matrix * ll * i_spread;
  float i_aa = texture(noise_texture_3d, hh).x;
  float i_bb = texture(noise_texture_3d, ii).x;
  float i_cc = texture(noise_texture_3d, jj).x;
  float i_dd = texture(noise_texture_3d, kk).x;
  float i_ee = texture(noise_texture_3d, ll).x;
  float i_ff = texture(noise_texture_3d, i_mm).x;
  return i_aa - i_bb * 0.8 + i_cc * 0.6 - i_dd * 0.4 + i_ee * 0.2 - i_ff * 0.1;
}

vec3 calculate_position()
{
  return mix(vec3(uniform_array[0]), vec3(uniform_array[1]), float(uniform_array[5].x) / float(uniform_array[5].y));
}

mat4 calculate_perspective_modelview()
{
  // Most of perspective is 0.
  mat4 perspective = mat4(0);

  float i_fov = 1.73;
  //float i_fov = 1.0 / tan(60.0 / 180.0 * PI * 0.5);
#if defined(USE_LD)
  perspective[0][0] = i_fov / (float(screen_size.x) / float(screen_size.y));
#elif (DISPLAY_MODE == -800) || (DISPLAY_MODE == 800) || (DISPLAY_MODE == -1200) || (DISPLAY_MODE == 1200)
  perspective[0][0] = i_fov / 1.6;
#else // Assuming 16/9.
  perspective[0][0] = i_fov / 1.78;
#endif
  perspective[1][1] = i_fov;
  perspective[2][2] = (I_PLANE_NEAR + I_PLANE_FAR) / (I_PLANE_NEAR - I_PLANE_FAR);
  perspective[2][3] = -1;
  perspective[3][2] = (2 * I_PLANE_NEAR * I_PLANE_FAR) / (I_PLANE_NEAR - I_PLANE_FAR);

  // Lookat components.
  vec3 forward = normalize(vec3(uniform_array[2]));
  vec3 right = normalize(cross(forward, normalize(vec3(uniform_array[3]))));
  vec3 i_up = normalize(cross(right, forward));

  // Lookat matrix.
  mat3 lookat = transpose(mat3(right, i_up, forward));
  mat4 modelview = mat4(lookat);
  modelview[3] = vec4(lookat * -calculate_position(), 1);

  return perspective * modelview;
}

vec4 calculate_terrain(vec2 pos)
{
  float ht = sample_noise_2d(pos * I_TERRAIN_PLANAR_SCALE) * I_TERRAIN_HEIGHT_SCALE;
  float water = 0.0;

  if(ht < I_WATER_LEVEL)
  {
    if(ht < I_ICE_LEVEL_HIGH)
    {
      water = smoothstep(-I_ICE_LEVEL_HIGH, -I_ICE_LEVEL_LOW, -ht);
      ht = I_WATER_LEVEL - water * I_ICE_EDGE;
    }
    else
    {
      ht = I_WATER_LEVEL;
    }
  }

  ht += pow(water, 4.0) * (sample_noise_2d(sin(pos * 0.0035 + ftime() * 0.000000004)) * 0.35 - sample_noise_2d(cos(pos * 0.0065 - ftime() * 0.000000004)) * 0.21);

  return vec4(pos.x, ht, pos.y, water);
}
