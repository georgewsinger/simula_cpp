#ifndef MOTORCAR_SHADER_SOFTKINETICDEPTHCAM_VERT__
#define MOTORCAR_SHADER_SOFTKINETICDEPTHCAM_VERT__

std::string shader_softkineticdepthcam_vert = 

"uniform mat4 uMVPMatrix;\n"
"attribute vec3 aPosition;\n"
"attribute float aConfidence;\n"
"attribute vec2 aTexCoord;\n"
"\n"
"varying vec2 vTexCoord;\n"
"varying float vIsValid;\n"
"\n"
"void main(void)\n"
"{\n"
"    if(aConfidence < 500.0 || aPosition.z < 0.01){\n"
"\n"
"        vIsValid = 0.0;\n"
"    }else{\n"
"        vIsValid = 1.0;\n"
"    }\n"
"    gl_Position =   uMVPMatrix * vec4(aPosition, 1.);\n"
"    vTexCoord = aTexCoord;\n"
"\n"
"}\n";

#endif
