static const char *g_shader_fragment_tree = ""
#if defined(USE_LD)
"tree.frag.glsl"
#else
"varying vec3 e;"
"varying vec2 c;"
"varying vec3 g;"
"varying vec3 s;"
"void main()"
"{"
"float a=1-abs(c.r),o=(c.g-.75)*4;"
"if(o<0)o/=-3.9;"
"float r=a*pow(1-o,2),v=r*(l(e*.022)+.6);"
"if(v<.05)discard;"
"vec3 t=normalize(s*c.r+.03*g),c=mix(vec3(.5,.55,.5),vec3(1,.9,.7),v*(l(e*.022)+l(e*.033))),e=vec3(3,3,2)*pow(r,2.)*b;"
"gl_FragColor=vec4(c*(dot(n,t)*.5+.5)+e,1);"
"}"
#endif
"";
