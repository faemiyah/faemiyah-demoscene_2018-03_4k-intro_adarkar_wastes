layout(location=0) in vec4 vertex;

const float I_SNOW_SIZE = 0.15;
const vec3 I_SWAY = vec3(0, 0, 1);
const vec3 I_WIND = vec3(-111, 0, 0);

varying vec3 real;
varying vec2 snow_coord;

void main()
{
  vec3 pos = vec3(vertex.x, 32768, vertex.z);
  float phase = fract((vertex.y / 32768) + ftime() * 0.0000002);
  vec3 wind = phase * I_WIND + sin(vertex.x + vertex.z + ftime() * 0.00004) * I_SWAY;

  real = vec3(uniform_array[0]) + (pos - phase * vec3(0.0, 65536, 0.0)) * 0.006 + wind;

  vec3 forward = calculate_position() - real;
  vec3 right = normalize(cross(forward, vec3(0, 1, 0)));
  vec3 up = normalize(cross(right, forward));

  snow_coord = vec2(int(vertex.w) % 2, int(vertex.w) / 2) * 2 - 1;

  real += (right * snow_coord.x + up * snow_coord.y) * I_SNOW_SIZE * snow_state;

  gl_Position = calculate_perspective_modelview() * vec4(real, 1);
}
