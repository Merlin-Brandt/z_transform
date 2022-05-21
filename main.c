//
#include <emscripten.h>
#include <emscripten/html5.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <GLES3/gl3.h>

typedef struct {
    GLuint vbo, vao;
} gobject_t;

typedef struct {
    int _unused;
    gobject_t m5obj;
} data_t;

typedef struct { 
    float x,y,z,r,g,b;
} xyzrgb;

typedef struct {
    float x,y;
} xy;


bool background_is_black = true;
double starttime;


GLuint compile(char const *vertexSource, char const *fragmentSource);
void main_loop(void *data);

int main()
{
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("canvas", &attr);
    emscripten_webgl_make_context_current(ctx);
    //emscripten_set_canvas_element_size("canvas", 640, 640);
    {
        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLubyte* version = glGetString(GL_VERSION);
        printf("Renderer: %s\n", renderer);
        printf("OpenGL version: %s\n", version);    
    }

	GLuint shaderProgram = compile(
        "#version 300 es\n\
         layout (location = 0) in vec3 aPos;\n\
         layout (location = 1) in vec3 aColor;\n\
         //layout (location = 2) in vec2 aTexCoord;\n\
         \n\
         out vec3 PassColor;\n\
         //out vec2 TexCoord;\n\
         \n\
         void main()\n\
         {\n\
            gl_Position = vec4(aPos, 1.0);\n\
            PassColor = aColor;\n\
            //TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n\
         }",

        "#version 300 es\n\
         precision mediump float;\n\
         out vec4 FragColor;\n\
         \n\
         in vec3 PassColor;\n\
         //in vec2 TexCoord;\n\
         \n\
         uniform sampler2D texture1;\n\
         \n\
         void main()\n\
         {\n\
            FragColor = vec4(PassColor, 1); //texture(texture1, TexCoord);\n\
         }"
    );

    starttime = emscripten_get_now();

    gobject_t m5obj;
    glGenBuffers(1, &m5obj.vbo);
    glGenVertexArrays(1, &m5obj.vao);
    

    GLuint texture;
    int w, h;
    char *img_data = emscripten_get_preloaded_image_data("texture.png", &w, &h);
    glGenTextures(1, &texture);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    free(img_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    data_t data;
    data.m5obj = m5obj;
    
    emscripten_set_main_loop_arg(main_loop, &data, 0, true);

    return EXIT_SUCCESS;
}

float seconds()
{
    return (emscripten_get_now() - starttime) / 1000.;
}

float ZPHASE = 0;
int   ZPHASE_DENOM = 1;

__attribute__((used)) 
float getZPHASE()
{
	return ZPHASE / ZPHASE_DENOM;
}

void main_loop(void *rawdata) 
{
    data_t *data = (data_t *) rawdata;
    if( background_is_black )
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    else
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const int fs = sizeof(GLfloat);

    GLfloat *vertices = (GLfloat *)malloc(0);
    int const vertexcomponentcount = 6;
    int meshcomponentcount = 0;
    int i = 0;
    {
        #define M_PI 3.14159265359
        #define M_2PI (2.0*M_PI)
        float radius = 0.8f;

        {
            int objvertexcount = 1024;
            float colorr = 0.0f;
            float colorg = 0.0f;
            float colorb = 1.0f;
            meshcomponentcount+= objvertexcount * vertexcomponentcount;
            vertices = (GLfloat *)realloc(vertices, meshcomponentcount*fs);
            int verticesperpoint = 2; int vpp = verticesperpoint;
            int objpointcount = objvertexcount / vpp;
            for (int ibegin = i; i < ibegin + objvertexcount; i+=vpp)
            for (int l = 0; l < vpp; ++l)
            {
                int obji = i-ibegin;
                int point = obji/vpp;
                //int objpointcount = objvertexcount;
                //int point = obji;

                // each point corresponds to one sample
                // each sample at splitpoint is advanced by objpointcount

                int SAMPLERATE = 64;
                int splitpoint = ((int)(seconds() * SAMPLERATE)) % objpointcount;

                int localsampleoffset = point;
                int temporaljumps = (int)(seconds() * SAMPLERATE / objpointcount) + ((splitpoint >= point) ? 1 : 0);
                int temporalsampleoffset = temporaljumps * objpointcount;
                int samplei = 0 + localsampleoffset + temporalsampleoffset;

                float inputfrequency     = M_2PI;
                int inputfrequency_denom = 4;
                float INPUTSIGNAL =    0.333*
                                             sin(inputfrequency      *samplei/inputfrequency_denom) 
                                     + 0.333*sin(inputfrequency*0.5  *samplei/inputfrequency_denom)
                                     + 0.333*sin(inputfrequency*0.333*samplei/inputfrequency_denom)
                //float INPUTSIGNAL = (samplei % 3) / 3.f
                ;

                xy z; ZPHASE       = (M_2PI) * (seconds()); 
                      ZPHASE_DENOM = objpointcount*128*5;
                z.x = cos(ZPHASE*samplei/ZPHASE_DENOM)*radius;
                z.y = sin(ZPHASE*samplei/ZPHASE_DENOM)*radius;
                
                if (l == 0)
                {
                    xyzrgb v; *(xyzrgb *)
                    (vertices + (i+l) * vertexcomponentcount) = 
                       (v.x = z.x * INPUTSIGNAL,
                        v.y = z.y * INPUTSIGNAL,
                        v.z = 1.0f,
                        v.r = /*abs(splitpoint - point) < 2 ? 0.0 :*/ colorr,
                        v.g = /*abs(splitpoint - point) < 2 ? 0.0 :*/ colorg,
                        v.b = /*abs(splitpoint - point) < 2 ? 0.0 :*/ colorb,
                        v);
                }
                if (l == 1)
                {
                    xyzrgb v; *(xyzrgb *)
                    (vertices + (i+l) * vertexcomponentcount) = 
                       (v.x = z.x,
                        v.y = z.y,
                        v.z = 1.0f,
                        v.r = /*abs(splitpoint - point) < 2 ? 0.0 :*/ 0.0f,
                        v.g = /*abs(splitpoint - point) < 2 ? 0.0 :*/ 1.0f,
                        v.b = /*abs(splitpoint - point) < 2 ? 0.0 :*/ 0.0f,
                        v);
                }
                
            }
        }
        glBindBuffer(GL_ARRAY_BUFFER, data->m5obj.vbo);
        glBufferData(GL_ARRAY_BUFFER, meshcomponentcount*fs, vertices, GL_DYNAMIC_DRAW);
    }

    glBindVertexArray(data->m5obj.vao);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    //glEnableVertexAttribArray(2);
    
    glDrawArrays(GL_POINTS, 0, meshcomponentcount/vertexcomponentcount);

    
}

__attribute__((used)) 
void toggle_background_color() { background_is_black = !background_is_black; }

GLuint compile(char const *vertexSource, char const *fragmentSource)
{
    int status;
	int logLength;

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, 0);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);
    if (!status && logLength)
    {
        char log[logLength];
        glGetShaderInfoLog(vertexShader, logLength, 0, log);
        fprintf(stderr, "GLL: ERROR: Failed to compile vertex shader: %s\n", log);
        exit(EXIT_FAILURE);
    }

    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, 0);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);
    if (!status && logLength)
    {
        char log[logLength];
        glGetShaderInfoLog(fragmentShader, logLength, 0, log);
        fprintf(stderr, "GLL: ERROR: Failed to compile fragment shader: %s\n", log);
        exit(EXIT_FAILURE);
    }

    
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
    if (!status && logLength)
    {
        char log[logLength];
        glGetProgramInfoLog(shaderProgram, logLength, 0, log);
        fprintf(stderr, "GLL: ERROR: Failed to link shaders: %s\n", log);
        exit(EXIT_FAILURE);
    }

    glUseProgram(shaderProgram);
    return shaderProgram;
}