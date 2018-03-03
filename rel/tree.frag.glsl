varying vec3 real;
varying vec2 tree_coord;
varying vec3 forward;
varying vec3 right;

void main()
{
  float strength_x = 1 - abs(tree_coord.x);
  float strength_y = (tree_coord.y - 0.75) * 4;
  if(strength_y < 0)
  {
    strength_y /= -3.9;
  }
  float luminance = strength_x * pow(1 - strength_y, 2);
  float strength = luminance * (sample_noise_3d(real * 0.022) + 0.6);

  if(strength < 0.05)
  {
    discard;
  }

  vec3 normal = normalize(right * tree_coord.x + 0.03 * forward);

  vec3 color = mix(vec3(0.5, 0.55, 0.5), vec3(1, 0.9, 0.7), strength * (sample_noise_3d(real * 0.022) + sample_noise_3d(real * 0.033)));

  vec3 illumination = vec3(3, 3, 2) * pow(luminance, 2.0) * night_state;

  gl_FragColor = vec4(color * (dot(light, normal) * 0.5 + 0.5) + illumination, 1);
}
