static const char *g_shader_vertex_tree = ""
#if defined(USE_LD)
"tree.vert.glsl"
#else
"layout(location=0)in vec4 r;"
"varying vec3 e;"
"varying vec2 c;"
"varying vec3 g;"
"varying vec3 s;"
"void main()"
"{"
"vec4 o=m(r.rb*.05);"
"g=a()-o.rgb,s=normalize(cross(g,vec3(0,1.,0))),c=vec2(int(r.a)%2*2-1,int(r.a)/2),e=o.rgb+(c.r*s*.5+c.g*vec3(0,1.,0)*6.)*(1-smoothstep(18,33,o.g))*(1-pow(o.a,3))*min(abs(o.g-18.),1)+vec3(.13,0,0)*sin(r.g+v()*.000003)*c.g-vec3(0,.2,0),gl_Position=u()*vec4(e,1);"
"}"
#endif
"";
