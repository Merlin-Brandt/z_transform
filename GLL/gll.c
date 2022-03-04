#include "gll.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <GL/glew.h>
#include <math.h>

void gllInit()
{
	GLenum returnVal = glewInit();
	if (returnVal != GLEW_OK)
	{
		printf("Failed initialize GLEW: %s\n", glewGetErrorString(returnVal));
		exit(EXIT_FAILURE);
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
}

void gllExitOnErr(char const *msg)
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		fprintf(stderr, "GLL: OPENGL ERROR: %s: %s\n", msg, gluErrorString(err));
		exit(EXIT_FAILURE);
	}
}

int gllPrintOnErr(char const *msg)
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		fprintf(stderr, "GLL: OPENGL ERROR: %s: %s\n", msg, gluErrorString(err));
		return 1;
	}
	return 0;
}

int gllCheckErr(char *errstr, unsigned size)
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		strncpy(errstr, (char *) gluErrorString(err), size);
		return 1;
	}
	return 0;
}

GLuint gllShaderCreate(char const *vertex_shader, char const *fragment_shader, int isfile,
					   char const **attrib_names, unsigned attrib_count, 
					   char const **uniform_names, GLint *uniform_locs, unsigned uniform_count)
{
	// yes they loose const, but will not be altered.
	char *shdSrc[2] = {(char *) vertex_shader, (char *) fragment_shader};
	GLuint shdProgO;
	// shader objects
	GLuint shdO[2];
	// current shader index
	int shdI, attribI, uniI;
	FILE *shdfile[2];
	
	if (isfile)
	{
		for (shdI = 0; shdI < 2; ++shdI)
		{
			shdfile[shdI] = fopen(shdSrc[shdI], "r");
			
			if (!shdfile[shdI])
			{
				fprintf(stderr, "GLL: ERROR: Cannot open %s!\n", shdSrc[shdI]);
				exit(EXIT_FAILURE);
			}
			
			// get file size
			int filesize;
			fseek(shdfile[shdI], 0, SEEK_END);
			filesize = ftell(shdfile[shdI]);
			shdSrc[shdI] = malloc(filesize + 1);
			rewind(shdfile[shdI]);
			
			// load file
			fread(shdSrc[shdI], 1, filesize, shdfile[shdI]);
			shdSrc[shdI][filesize] = '\0';
			
			fclose(shdfile[shdI]);
		}
	}

	// for both shaders (vertex, fragment)
	for (shdI = 0; shdI < 2; ++shdI)
	{
		// compile shader
		int compileStatus;
		int logLength;

		shdO[shdI] = glCreateShader(shdI ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);
		glShaderSource(shdO[shdI], 1, (GLchar const **) &shdSrc[shdI], 0);
		glCompileShader(shdO[shdI]);
		
		// if isfile, shdSrc was malloc'd
		if (isfile)
			free(shdSrc[shdI]);

		glGetShaderiv(shdO[shdI], GL_COMPILE_STATUS, &compileStatus);
		glGetShaderiv(shdO[shdI], GL_INFO_LOG_LENGTH, &logLength);
		if (!compileStatus && logLength)
		{
			char log[logLength];
			glGetShaderInfoLog(shdO[shdI], logLength, 0, log);
			fprintf(stderr, "GLL: ERROR: Failed to compile %s shader: %s\n", shdI ?"fragment" :"vertex", log);
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
		
		// bind attributes
		for (attribI = 0; attribI < attrib_count; ++attribI)
			glBindAttribLocation(shdProgO, attribI, attrib_names[attribI]);
		
		glLinkProgram(shdProgO);
		
		// get uniform locations
		for (uniI = 0; uniI < uniform_count; ++uniI)
			uniform_locs[uniI] = glGetUniformLocation(shdProgO, uniform_names[uniI]); 

		// check link status
		glGetProgramiv(shdProgO, GL_LINK_STATUS, &linkStatus);
		glGetProgramiv(shdProgO, GL_INFO_LOG_LENGTH, &logLength);
		if (!linkStatus && logLength)
		{
			char log[logLength];
			glGetProgramInfoLog(shdProgO, logLength, 0, log);
			fprintf(stderr, "GLL: ERROR: Failed to link shaders: %s\n", log);
			exit(EXIT_FAILURE);
		}

		// delete shaders
		for (shdI = 0; shdI < 2; ++shdI)
		{
			glDetachShader(shdProgO, shdO[shdI]);
			glDeleteShader(shdO[shdI]);
		}
	} // link program
	
	glUseProgram(shdProgO);
	return shdProgO;
}

void gllShaderDestroy(GLuint shader)
{
	glDeleteProgram(shader);
}

GLLformat *gllFormatCreate(GLLformat *format, unsigned attrib_count, unsigned stride)
{
	format->attrib_count = attrib_count;
	format->stride = stride;
	format->attrib_offsets = malloc(attrib_count * sizeof (*format->attrib_offsets));
	format->attrib_sizes = malloc(attrib_count * sizeof (*format->attrib_sizes));
	format->attrib_types = malloc(attrib_count * sizeof (*format->attrib_types));
	format->attrib_normalized = malloc(attrib_count * sizeof (*format->attrib_normalized));	
	return format;
}

void gllFormatDestroy(GLLformat *format)
{
	format->attrib_count = 0;
	free(format->attrib_offsets);
	free(format->attrib_sizes);
	free(format->attrib_types);
	free(format->attrib_normalized);
}

GLLformat *gllFormatAttribRange(GLLformat *format, unsigned attrib, unsigned offset, unsigned size)
{
	format->attrib_offsets[attrib] = offset;
	format->attrib_sizes[attrib] = size;
	return format;
}

GLLformat *gllFormatAttribType(GLLformat *format, unsigned attrib, GLenum type, int normalized)
{
	format->attrib_types[attrib] = type;
	format->attrib_normalized[attrib] = normalized;
	return format;
}

GLuint gllTexCreate()
{
	GLuint tex;
	glGenTextures(1, &tex);
	return tex;
}

void gllTexDestroy(GLuint tex)
{
	glDeleteTextures(1, &tex);
}

GLuint gllTexPixels2D(GLuint tex, const GLubyte *pixelPtr, unsigned width, unsigned height)
{
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelPtr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	return tex;
}

GLLobj *gllObjCreate(GLLobj *obj, GLLformat *format, unsigned texcount)
{
	glGenBuffers(1, &obj->vertexBuf);
	glGenBuffers(1, &obj->indexBuf);
	glGenVertexArrays(1, &obj->vertexArr);
	gllObjFormat(obj, format);

	obj->texcount = texcount;
	if (texcount == 0)
		obj->textures = 0;
	else
	{
		obj->textures = malloc(texcount * sizeof (*obj->textures));
		memset(obj->textures, 0, texcount * sizeof (*obj->textures));
	}
	
	obj->flags = GLL_FLAG_USED;
	return obj;
}

void gllObjDestroy(GLLobj *obj)
{
	glDeleteBuffers(1, &obj->vertexBuf);
	glDeleteBuffers(1, &obj->indexBuf);
	glDeleteVertexArrays(1, &obj->vertexArr);
	free(obj->textures);
	obj->flags &= ~GLL_FLAG_USED;
}

GLLobj *gllObjFormat(GLLobj *obj, GLLformat *format)
{
	unsigned attribI;
	
	glBindVertexArray(obj->vertexArr);
	glBindBuffer(GL_ARRAY_BUFFER, obj->vertexBuf);
	for (attribI = 0; attribI < format->attrib_count; ++attribI)
	{
		glEnableVertexAttribArray(attribI);
		glVertexAttribPointer(attribI, format->attrib_sizes[attribI], format->attrib_types[attribI], 
							  format->attrib_normalized[attribI], format->stride, (void *) (format->attrib_offsets[attribI]));
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return obj;
}

GLLobj *gllObjVertices(GLLobj *obj, const void *vertexPtr, unsigned size, unsigned vertexCount, GLenum usage)
{
	obj->vertexCount = vertexCount;
	glBindBuffer(GL_ARRAY_BUFFER, obj->vertexBuf);
	glBufferData(GL_ARRAY_BUFFER, size, vertexPtr, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return obj;
}

GLLobj *gllObjIndicesEnabled(GLLobj *obj, int enabled)
{
	if (enabled)
		obj->flags |= GLL_FLAG_INDICES;
	else
		obj->flags &= ~GLL_FLAG_INDICES;
	return obj;
}

GLLobj *gllObjIndices(GLLobj *obj, const GLuint *indexPtr, unsigned indexCount, GLenum usage)
{
	obj->indexCount = indexCount;
	gllObjIndicesEnabled(obj, 1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->indexBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLuint), indexPtr, usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return obj;
}

GLLobj *gllObjTexture(GLLobj *obj, unsigned texindex, GLuint tex)
{
	obj->textures[texindex] = tex;
	return obj;
}

void gllClear(GLfloat r, GLfloat g, GLfloat b)
{
	glClearColor(r, g, b, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gllRender(GLLobj *obj, GLenum mode)
{
	unsigned texI;

	// bind
	glBindBuffer(GL_ARRAY_BUFFER, obj->vertexBuf);
	glBindVertexArray(obj->vertexArr);
	
	for (texI = 0; texI < obj->texcount; ++texI)
	{
		glActiveTexture(GL_TEXTURE0 + texI);
		glBindTexture(GL_TEXTURE_2D, obj->textures[texI]);
	}

	// render
	if (obj->flags & GLL_FLAG_INDICES)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->indexBuf);
		glDrawElements(mode, obj->indexCount, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	else
	{
		glDrawArrays(mode, 0, obj->vertexCount);
	}
	
	// unbind
	for (texI = 0; texI < obj->texcount; ++texI)
	{
		glActiveTexture(GL_TEXTURE0 + texI);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

/* R.I.P. beautiful code, excluded from the library :'(
void gllViewRotateByMouse(float t, float mx, float my)
{
	// cartesian to spherical coordinates
	static float camVertiAngle = 0;
	static float camHoriAngle = 0;

	camHoriAngle+= t * mx;
	camVertiAngle+= t * my;
	if (camVertiAngle < -1.5) camVertiAngle = -1.5;
	if (camVertiAngle > 1.5) camVertiAngle = 1.5;

	camDir[X] = cosf(camVertiAngle) * sinf(camHoriAngle);
	camDir[Y] = sinf(camVertiAngle);
	camDir[Z] = cosf(camVertiAngle) * cosf(camHoriAngle);
	kmVec3Normalize((kmVec3 *) camDir, (kmVec3 *) camDir);


	// horiAngle - 3.14 / 2 means horiAngle - 90°, which is the right
	camRight[X] = sin(camHoriAngle - 3.14f/2.0f);
	camRight[Y] = 0;
	camRight[Z] = cos(camHoriAngle - 3.14f/2.0f);
	kmVec3Normalize((kmVec3 *) camRight, (kmVec3 *) camRight);

	kmVec3Cross((kmVec3 *) camUp, (kmVec3 *) camRight, (kmVec3 *) camDir);
	kmVec3Normalize((kmVec3 *) camUp, (kmVec3 *) camUp);

	float camCenter[3];
	kmVec3Add((kmVec3 *) camCenter, (kmVec3 *) camPos, (kmVec3 *) camDir);
	kmMat4LookAt((kmMat4 *) viewMat, (kmVec3 *) camPos, (kmVec3 *) camCenter, (kmVec3 *) camUp);
}
*/
