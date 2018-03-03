layout(location=0) in vec4 vertex;

varying vec3 real;
varying vec2 tree_coord;
varying vec3 forward;
varying vec3 right;

const float I_TREE_HEIGHT = 6.0;
const float I_TREE_WIDTH = 0.5;
const vec3 I_TREE_SWAY = vec3(0.13, 0, 0);

void main()
{
  vec4 terrain = calculate_terrain(vertex.xz * 0.05);
  vec3 i_up = vec3(0, 1.0, 0);

  forward = calculate_position() - terrain.xyz;
  right = normalize(cross(forward, i_up));

  tree_coord = vec2((int(vertex.w) % 2) * 2 - 1, int(vertex.w) / 2);

  real = terrain.xyz + (tree_coord.x * right * I_TREE_WIDTH + tree_coord.y * i_up * I_TREE_HEIGHT) * (1 - smoothstep(18, 33, terrain.y)) * (1 - pow(terrain.w, 3)) * min(abs(terrain.y - I_WATER_LEVEL), 1) + I_TREE_SWAY * sin(vertex.y + ftime() * 0.000003) * tree_coord.y - vec3(0, 0.2, 0);

  gl_Position = calculate_perspective_modelview() * vec4(real, 1);
}
