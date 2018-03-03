static const char *g_shader_fragment_snow = ""
#if defined(USE_LD)
"snow.frag.glsl"
#else
"varying vec3 e;"
"varying vec2 c;"
"void main()"
"{"
"float o=length(c);"
"if(o>1)discard;"
"float r=mix(.8,1.,1-abs(dot(normalize(e-a()),n))),e=.2/(333.1-gl_FragCoord.b*332.9);"
"o=sin(o*2),gl_FragColor=vec4(mix(mix(vec3(.9),vec3(1),pow(o,8))*r,vec3(.4),e),1);"
"}"
#endif
"";
