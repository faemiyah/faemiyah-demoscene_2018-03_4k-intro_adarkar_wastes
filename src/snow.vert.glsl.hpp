static const char *g_shader_vertex_snow = ""
#if defined(USE_LD)
"snow.vert.glsl"
#else
"layout(location=0)in vec4 r;"
"varying vec3 e;"
"varying vec2 c;"
"void main()"
"{"
"vec3 i=vec3(r.r,32768,r.b);"
"float t=fract(r.g/32768+v()*.0000002);"
"vec3 n=t*vec3(-111,0,0)+sin(r.r+r.b+v()*.00004)*vec3(0,0,1);"
"e=vec3(o[0])+(i-t*vec3(.0,65536,.0))*.006+n;"
"vec3 o=a()-e,v=normalize(cross(o,vec3(0,1,0))),a=normalize(cross(v,o));"
"c=vec2(int(r.a)%2,int(r.a)/2)*2-1,e+=(v*c.r+a*c.g)*.15*x,gl_Position=u()*vec4(e,1);"
"}"
#endif
"";
