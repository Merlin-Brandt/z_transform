#include <GL/glew.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cells.h"

int const VERTEX_SHD = 0, FRAG_SHD = 1;

static GLuint shdProgO;
// the uniform location of "texSampler" in shdProg
static GLint shdProgTexSmplUniL;
static GLuint cellsVao, cellsVbo;
static GLuint cellsTexO;
static GLubyte *cellsTex;

#define QUAD_RENDER

static char const *shdSrc[2] =
{
    // vertex shader
    "#version 130\n"
    "in vec2 pos2;"
    "smooth out vec2 fragPos2;"

    "void main()"
    "{"
        "gl_Position = vec4(pos2, 0, 1);"
        "fragPos2 = pos2;"
    "}"

    ,
    // fragment shader
    "#version 130\n"
    "smooth in vec2 fragPos2;"
    "uniform sampler2D texSampler;"
    "out vec4 fragColor4;"

    "void main()"
    "{"
        "vec2 fragUv2 =  vec2(.5, .5) + fragPos2 / 2.;"
        "float val = texture(texSampler, fragUv2).r;"
        "fragColor4 = vec4(val, val, val, 1);"
    "}"
};

#if 1
void checkGlErr(const char *msg)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        fprintf(stderr, "OpenGL error (%s): %s\n", msg, gluErrorString(err));
        exit(EXIT_FAILURE);
    }
}
#else
#define checkGlErr(X)
#warning DOEN NOT MAK SENS! perFORMANC NOt SO BIG!!!!!!!!!!!!!!!!!!!!!!
#error DO YOU REALLY WANNA DO IT? THEN REMOVE thIS!!!!
#endif

static void init_glew()
{
    GLenum glewStatus;

    glewStatus = glewInit();
    if (glewStatus != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize glew: %s", glewGetErrorString(glewStatus));
        exit(EXIT_FAILURE);
    }
}

static void init_shd()
{
    // shader objects
    GLuint shdO[2];
    // current shader index
    int shdI;

    // for both shaders (vertex, fragment)
    for (shdI = 0; shdI < 2; ++shdI)
    {
        // compile shader
        int compileStatus;
        int logLength;

        shdO[shdI] = glCreateShader(shdI == VERTEX_SHD ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
        glShaderSource(shdO[shdI], 1, &shdSrc[shdI], 0);
        glCompileShader(shdO[shdI]);

        glGetShaderiv(shdO[shdI], GL_COMPILE_STATUS, &compileStatus);
        glGetShaderiv(shdO[shdI], GL_INFO_LOG_LENGTH, &logLength);
        if (!compileStatus && logLength)
        {
            char log[logLength];
            glGetShaderInfoLog(shdO[shdI], logLength, 0, log);
            fprintf(stderr, "Failed to compile %s shader: %s\n", shdI == VERTEX_SHD ?"vertex" :"fragment", log);
            exit(EXIT_FAILURE);
        }
    } // for both shaders

    // link program
    {
        int shdI;
        int linkStatus;
        int logLength;

        shdProgO = glCreateProgram();
        for (shdI = 0; shdI < 2; ++shdI)
            glAttachShader(shdProgO, shdO[shdI]);
        glLinkProgram(shdProgO);

        glGetProgramiv(shdProgO, GL_LINK_STATUS, &linkStatus);
        glGetProgramiv(shdProgO, GL_INFO_LOG_LENGTH, &logLength);
        if (!linkStatus && logLength)
        {
            char log[logLength];
            glGetProgramInfoLog(shdProgO, logLength, 0, log);
            fprintf(stderr, "Failed to link shaders: %s\n", log);
            exit(EXIT_FAILURE);
        }

        // delete shaders
        for (shdI = 0; shdI < 2; ++shdI)
        {
            glDetachShader(shdProgO, shdO[shdI]);
            glDeleteShader(shdO[shdI]);
        }

        glUseProgram(shdProgO);
    } // link program

    // get uniform locations
    shdProgTexSmplUniL = glGetUniformLocation(shdProgO, "texSampler");
    glUniform1i(shdProgTexSmplUniL, 0);
}

// init cells objects (vertex array object, vertex buffer object, texture buffer object)
static void init_cellsO()
{
    // vbo
    GLfloat vertBuf[] = {
#ifdef QUAD_RENDER
        -1, -1,
        -1, 1,
        1, 1,
        1, -1
#else
        -1, -1,
        1, 1,
        1, -1,

        -1, -1,
        -1, 1,
        1, 1,
#endif
    };

    glGenBuffers(1, &cellsVbo);
    glBindBuffer(GL_ARRAY_BUFFER, cellsVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertBuf), vertBuf, GL_STATIC_DRAW);

    glGenVertexArrays(1, &cellsVao);
    glBindVertexArray(cellsVao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    cellsTex = malloc(cellCount);
    glGenTextures(1, &cellsTexO);
    glBindTexture(GL_TEXTURE_2D, cellsTexO);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void cells_render_init(GLint renderX, GLint renderY, GLsizei renderW, GLsizei renderH)
{
    glViewport(renderX, renderY, renderW, renderH);
    init_glew();
    init_shd();
    init_cellsO();
    checkGlErr("after init");
}

void cells_render_deinit()
{
    glDeleteTextures(1, &cellsTexO);
    free(cellsTex);
    glDeleteProgram(shdProgO);
}

// updates buffers according to the cell's value
void cells_render_update()
{
    CellIndex cellI;
    for (cellI = 0; cellI < cellCount; ++cellI)
	{
        cellsTex[cellI] = fmin(255 * cellLife[cellI], 255);
	}

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, cellGridW, cellGridH, 0, GL_RED, GL_UNSIGNED_BYTE, cellsTex);

    checkGlErr("after render update");
}

void cells_render()
{
	glClearColor(0, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
#ifdef QUAD_RENDER
    glDrawArrays(GL_QUADS, 0, 4);
#else
    glDrawArrays(GL_TRIANGLES, 0, 6);
#endif
    checkGlErr("after rendering");
}
