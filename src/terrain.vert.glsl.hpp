static const char *g_shader_vertex_terrain = ""
#if defined(USE_LD)
"terrain.vert.glsl"
#else
"layout(location=0)in vec2 r;"
"varying vec4 e;"
"void main()"
"{"
"float o=.75;"
"int c=gl_InstanceID/384,v=gl_InstanceID-c*384;"
"if(c>384)c-=384,o*=3;"
"vec2 t=vec2((v-191.5)*o,(c-191.5)*o),i=floor(a().rb/o)*o+t+r*o*.5;"
"e=m(i),gl_Position=u()*vec4(e.rgb,1);"
"}"
#endif
"";
