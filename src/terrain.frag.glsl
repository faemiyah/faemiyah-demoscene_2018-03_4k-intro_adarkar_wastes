varying vec4 real;

void main()
{
  float leps = 3.3;
  vec4 px = calculate_terrain(vec2(real.x + leps, real.z));
  vec4 pz = calculate_terrain(vec2(real.x, real.z + leps));
  vec3 nor = normalize(cross(pz.xyz - real.xyz, px.xyz - real.xyz));

  vec3 terrain_color = mix(vec3(1), vec3(0.8, 0.7, 0.6), sample_noise_3d(real.xyz * 0.0005));
  vec3 water_color = mix(vec3(0.8, 0.9, 1), vec3(0.6, 0.6, 0.7), sample_noise_3d(real.xyz * 0.0005));

  float dp = dot(nor, light);
  float ref_pos = clamp(dot(reflect(normalize(real.xyz - calculate_position()), nor), light), 0, 1);
  float ref_neg = dot(reflect(normalize(calculate_position() - real.xyz), nor), light);

  float water_mix = clamp((real.w - 0.99) * 100, 0, 1);
  float mul = dp * 0.8 + mix(pow(ref_neg, 3) * 0.3, pow(ref_pos, 18) * 0.8, real.w);

  vec3 color = mix(terrain_color, water_color, water_mix) * mul;

  gl_FragColor = vec4(color, 1);
}
