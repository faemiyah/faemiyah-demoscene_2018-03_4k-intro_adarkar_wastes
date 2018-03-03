varying vec3 real;
varying vec2 snow_coord;

void main()
{
  float len = length(snow_coord);
  if(len > 1)
  {
    discard;
  }

  float ref = mix(0.8, 1.0, 1 - abs(dot(normalize(real - calculate_position()), light)));

  float depth = (2 * I_PLANE_NEAR) / (I_PLANE_FAR + I_PLANE_NEAR - gl_FragCoord.z * (I_PLANE_FAR - I_PLANE_NEAR));
  
  len = sin(len * 2);
  gl_FragColor = vec4(mix(mix(vec3(0.9), vec3(1), pow(len, 8)) * ref, vec3(0.4), depth), 1);
}
