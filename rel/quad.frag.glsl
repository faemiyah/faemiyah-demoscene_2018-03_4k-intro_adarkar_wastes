layout(location=8) uniform sampler2D color_texture;
layout(location=9) uniform sampler2D depth_texture;

varying vec3 direction;
varying vec2 position;

float dmap(vec2 position)
{
  return (2 * I_PLANE_NEAR) / (I_PLANE_FAR + I_PLANE_NEAR - texture(depth_texture, position).r * (I_PLANE_FAR - I_PLANE_NEAR));
}

void main()
{
  float night_decrement = (1 - night_state * 0.8);

  float luminosity = dot(normalize(direction), light) * 0.5 + 0.5;
  vec3 sky = mix(vec3(0.6), vec3(1, 1, 0.9), pow(luminosity, 7) + sample_noise_3d(direction * 0.05) * 0.05) * night_decrement;

  float depth = dmap(position);
  float dof = abs(uniform_array[4].x / 255.0 - depth);

  vec4 color = vec4(0);
  for(float ii = 0; (ii < 6.28); ii += 0.314) // Loop from -PI to +PI in 20 steps.
  {
    vec2 tpos = position + vec2(cos(ii), sin(ii)) * dof * 0.01;
    vec4 add = texture(color_texture, tpos, dof * 3);
    add.rgb = mix(add.rgb, sky, depth) - mix(0, depth * 0.9 + 0.1, 1 - luminosity);
    add.a += 0.1;
    color += vec4(add.rgb * add.a, add.a);
  }
  color.rgb /= color.a;

  float fade = pow(abs(float(uniform_array[5].x) / uniform_array[5].y - 0.5) * 2, 8);

  gl_FragColor = vec4(mix(color.rgb, vec3(0.4 * night_decrement), mix(mix(0.0, fade, depth), fade, fade)), 1);
}
