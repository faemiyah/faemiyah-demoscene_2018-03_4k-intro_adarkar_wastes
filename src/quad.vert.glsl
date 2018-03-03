layout(location=0) in vec2 vertex;

varying vec3 direction;
varying vec2 position;

void main()
{
  vec3 forward = normalize(vec3(uniform_array[2]));
  vec3 right = normalize(cross(forward, normalize(vec3(uniform_array[3]))));
  vec3 i_up = normalize(cross(right, forward));
#if defined(USE_LD)
  direction = (right * vertex.x * float(screen_size.x) / float(screen_size.y) + i_up * vertex.y) * 0.5 - forward;
#elif (DISPLAY_MODE == -800) || (DISPLAY_MODE == 800) || (DISPLAY_MODE == -1200) || (DISPLAY_MODE == 1200)
  direction = (right * vertex.x * 1.6 + i_up * vertex.y) * 0.5 - forward;
#else // Assuming 16/9.
  direction = (right * vertex.x * 1.78 + i_up * vertex.y) * 0.5 - forward;
#endif

  position = (vertex + 1) * 0.5;

  gl_Position=vec4(vertex, 0, 1);
}
