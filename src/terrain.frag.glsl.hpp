static const char *g_shader_fragment_terrain = ""
#if defined(USE_LD)
"terrain.frag.glsl"
#else
"varying vec4 e;"
"void main()"
"{"
"float r=3.3;"
"vec4 c=m(vec2(e.r+r,e.b)),v=m(vec2(e.r,e.b+r));"
"vec3 o=normalize(cross(v.rgb-e.rgb,c.rgb-e.rgb)),t=mix(vec3(1),vec3(.8,.7,.6),l(e.rgb*.0005)),i=mix(vec3(.8,.9,1),vec3(.6,.6,.7),l(e.rgb*.0005));"
"float l=dot(o,n),m=clamp(dot(reflect(normalize(e.rgb-a()),o),n),0,1),g=dot(reflect(normalize(a()-e.rgb),o),n),a=clamp((e.a-.99)*100,0,1),n=l*.8+mix(pow(g,3)*.3,pow(m,18)*.8,e.a);"
"vec3 e=mix(t,i,a)*n;"
"gl_FragColor=vec4(e,1);"
"}"
#endif
"";
