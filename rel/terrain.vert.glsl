layout(location=0) in vec2 vertex;

varying vec4 real;

void main()
{
  int i_plane_side = 384;
  float quad_side = 0.75;
  int row = gl_InstanceID / i_plane_side;
  int column = gl_InstanceID - (row * i_plane_side);
  if(row > i_plane_side)
  {
    row -= i_plane_side;
    quad_side *= 3;
  }
  vec2 center = vec2((column - (i_plane_side / 2) + 0.5) * quad_side, (row - (i_plane_side / 2) + 0.5) * quad_side);

  vec2 pos = floor(calculate_position().xz / quad_side) * quad_side + center + vertex * quad_side * 0.5;
  real = calculate_terrain(pos);

  gl_Position = calculate_perspective_modelview() * vec4(real.xyz, 1);
}
