#ifndef OPENGLIB_H_INCLUDED
#define OPENGLIB_H_INCLUDED

#include <stdint.h>
#include <GL/glew.h>

#define GLL_DB(X) X
#define GLL_FLAG_USED 0x01
#define GLL_FLAG_INDICES 0x02
#define GLL_VERTEX_STRIDE 8
#define GLL_NO_SHADER 0

typedef struct
{
	GLuint vertexBuf, indexBuf, vertexArr;
	unsigned vertexCount, indexCount;
	GLuint *textures;
	unsigned texcount;
	uint8_t flags;
} GLLobj;

typedef struct
{
	unsigned attrib_count;
	unsigned stride;
	unsigned *attrib_offsets, *attrib_sizes;
	GLenum *attrib_types;
	int *attrib_normalized;
} GLLformat;

typedef GLuint GLLshader;
typedef GLuint GLLtex;
typedef GLint GLLuniform;

void gllInit();

void gllExitOnErr(char const *msg);
int gllPrintOnErr(char const *msg);
int gllCheckErr(char *errstr, unsigned size);

GLLshader gllShaderCreate(char const *vertex_shader, char const *fragment_shader, int isfile,
					   char const **attrib_names, unsigned attrib_count, 
					   char const **uniform_names, GLLuniform *uniform_locs, unsigned uniform_count);
void gllShaderDestroy(GLLshader shader);
#define gllUseShader glUseProgram 

GLLformat *gllFormatCreate(GLLformat *format, unsigned attrib_count, unsigned stride);
void gllFormatDestroy(GLLformat *format);
GLLformat *gllFormatAttribRange(GLLformat *format, unsigned attrib, unsigned offset, unsigned size);
GLLformat *gllFormatAttribType(GLLformat *format, unsigned attrib, GLenum type, int normalized);

GLLtex gllTexCreate();
void gllTexDestroy(GLLtex tex);
GLLtex gllTexPixels2D(GLLtex tex, GLubyte const *pixelPtr, unsigned width, unsigned height);

GLLobj *gllObjCreate(GLLobj *obj, GLLformat *format, unsigned texcount);
void gllObjDestroy(GLLobj *obj);
GLLobj *gllObjFormat(GLLobj *obj, GLLformat *format);
GLLobj *gllObjVertices(GLLobj *obj, const void *vertexPtr, unsigned size, unsigned vertexCount, GLenum usage);
GLLobj *gllObjIndicesEnabled(GLLobj *obj, int enabled);
GLLobj *gllObjIndices(GLLobj *obj, const GLuint *indexPtr, unsigned indexCount, GLenum usage);
GLLobj *gllObjTexture(GLLobj *obj, unsigned texindex, GLLtex tex);

void gllClear(GLfloat r, GLfloat g, GLfloat b);
void gllRender(GLLobj *obj, GLenum mode);

#endif /* OPENGLIB_H_INCLUDED */