static const char *g_shader_vertex_quad = ""
#if defined(USE_LD)
"quad.vert.glsl"
#else
"layout(location=0)in vec2 r;"
"varying vec3 c;"
"varying vec2 g;"
"void main()"
"{"
"vec3 e=normalize(vec3(o[2])),v=normalize(cross(e,normalize(vec3(o[3]))));"
"c=(v*r.r*1.78+normalize(cross(v,e))*r.g)*.5-e,g=(r+1)*.5,gl_Position=vec4(r,0,1);"
"}"
#endif
"";
