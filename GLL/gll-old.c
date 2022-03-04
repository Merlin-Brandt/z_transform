#include "gll.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <GL/glew.h>
#include <math.h>
#include "kazmath/kazmath.h"

GLLformat *gllFormatCreate(GLLformat *format, uint8_t attrib_count)
{
	format->attrib_count = attrib_count;
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

GLuint gllShaderCreate(char const *vertex_shader, char const *fragment_shader,  
					   char const **attrib_names, unsigned attrib_count, 
					   char const **uniform_names, GLint *uniform_locs, unsigned uniform_count)
{
	char const *shdSrc[2] = {vertex_shader, fragment_shader};
	GLuint shdProgO;
	// shader objects
	GLuint shdO[2];
	// current shader index
	int shdI, attribI, uniI;

	// for both shaders (vertex, fragment)
	for (shdI = 0; shdI < 2; ++shdI)
	{
		// compile shader
		int compileStatus;
		int logLength;

		shdO[shdI] = glCreateShader(shdI ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);
		glShaderSource(shdO[shdI], 1, &shdSrc[shdI], 0);
		glCompileShader(shdO[shdI]);

		glGetShaderiv(shdO[shdI], GL_COMPILE_STATUS, &compileStatus);
		glGetShaderiv(shdO[shdI], GL_INFO_LOG_LENGTH, &logLength);
		if (!compileStatus && logLength)
		{
			char log[logLength];
			glGetShaderInfoLog(shdO[shdI], logLength, 0, log);
			fprintf(stderr, "Failed to compile %s shader: %s\n", shdI ?"fragment" :"vertex", log);
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
			fprintf(stderr, "Failed to link shaders: %s\n", log);
			exit(EXIT_FAILURE);
		}

		// delete shaders
		for (shdI = 0; shdI < 2; ++shdI)
		{
			glDetachShader(shdProgO, shdO[shdI]);
			glDeleteShader(shdO[shdI]);
		}
	} // link program
	
	return shdProgO;
}

void gllShaderDestroy(GLuint shader)
{
	glDeleteProgram(shdProgO);
}

static void init_shd_uni()
{
	glUseProgram(shdProgO);
	// get uniform locations
	shdProgMvpUniL = glGetUniformLocation(shdProgO, "mvp");
	shdProgModelUniL = glGetUniformLocation(shdProgO, "model");
	shdProgLightColUniL = glGetUniformLocation(shdProgO, "lightRGB");
	glUniform3f(shdProgLightColUniL, 1, 1, 1);
	shdProgLightPosUniL = glGetUniformLocation(shdProgO, "lightXYZ");
	shdProgAmbientColUniL = glGetUniformLocation(shdProgO, "ambientLightRGB");
	glUniform3f(shdProgAmbientColUniL, 0, 0, 0);
	shdProgEyePosUniL = glGetUniformLocation(shdProgO, "eyePosXYZ");
	shdProgMatDiffTexUniL = glGetUniformLocation(shdProgO, "materialDiffuseTex");
	glUniform1i(shdProgMatDiffTexUniL, 0);
	shdProgShininessUniL = glGetUniformLocation(shdProgO, "materialShininess");
	glUniform1f(shdProgShininessUniL, 4);
	glUseProgram(0);
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

		shdO[shdI] = glCreateShader(shdI ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);
		glShaderSource(shdO[shdI], 1, &shdSrc[shdI], 0);
		glCompileShader(shdO[shdI]);

		glGetShaderiv(shdO[shdI], GL_COMPILE_STATUS, &compileStatus);
		glGetShaderiv(shdO[shdI], GL_INFO_LOG_LENGTH, &logLength);
		if (!compileStatus && logLength)
		{
			char log[logLength];
			glGetShaderInfoLog(shdO[shdI], logLength, 0, log);
			fprintf(stderr, "Failed to compile %s shader: %s\n", shdI ?"fragment" :"vertex", log);
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
	} // link program

}

void gllInit()
{
	GLenum returnVal = glewInit();
	if (returnVal != GLEW_OK)
	{
		printf("Failed initialize GLEW: %s\n", glewGetErrorString(returnVal));
		exit(EXIT_FAILURE);
	}

	init_shd();
    init_shd_uni();

	kmMat4Identity((kmMat4 *) viewMat);
	kmMat4Identity((kmMat4 *) gllDefaultProjection);
	gllProjMatPtr = gllDefaultProjection;

	camPos[Z] = 3;
	camDir[X] = 0;
	camDir[Y] = -1;
	camDir[Z] = 0;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}

void gllTerminate()
{
	glDeleteProgram(shdProgO);
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

GLLobj *gllObjInit(GLLobj *obj)
{
	glGenBuffers(1, &obj->vertexBuf);
	glGenBuffers(1, &obj->indexBuf);
	glGenVertexArrays(1, &obj->vertexArr);

	glBindVertexArray(obj->vertexArr);
	glBindBuffer(GL_ARRAY_BUFFER, obj->vertexBuf);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	if (GLL_VERTEX_STRIDE != 8)
	{
		fprintf(stderr, "GLL: ERROR: GLL_VERTEX_STRIDE is not 8 but %i!\n", GLL_VERTEX_STRIDE);
		exit(EXIT_FAILURE);
	}
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, GLL_VERTEX_STRIDE * sizeof (GLfloat), (void*) (intptr_t) (0 * sizeof (GLfloat)));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, GLL_VERTEX_STRIDE * sizeof (GLfloat), (void*) (intptr_t) (3 * sizeof (GLfloat)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, GLL_VERTEX_STRIDE * sizeof (GLfloat), (void*) (intptr_t) (5 * sizeof (GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenTextures(1, &obj->texture);
	obj->flags = GLL_FLAG_USED;

	kmMat4Identity((kmMat4 *) obj->transform);
	return obj;
}

void gllObjDeinit(GLLobj *obj)
{
	glDeleteBuffers(1, &obj->vertexBuf);
	glDeleteBuffers(1, &obj->indexBuf);
	glDeleteVertexArrays(1, &obj->vertexArr);
	glDeleteTextures(1, &obj->texture);
	obj->flags &= ~GLL_FLAG_USED;
}

GLLobj *gllObjCopyVertices(GLLobj *obj, const GLfloat *vertexPtr, unsigned compCount, GLenum usage)
{
	glBindBuffer(GL_ARRAY_BUFFER, obj->vertexBuf);
	glBufferData(GL_ARRAY_BUFFER, compCount * sizeof(GLfloat), vertexPtr, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return obj;
}

GLLobj *gllObjSetIndicesEnabled(GLLobj *obj, int enabled)
{
	if (enabled)
		obj->flags |= GLL_FLAG_INDICES;
	else
		obj->flags &= ~GLL_FLAG_INDICES;
	return obj;
}

GLLobj *gllObjIndices(GLLobj *obj, const GLuint *indexPtr, unsigned indexCount, GLenum usage)
{
	gllObjSetIndicesEnabled(obj, 1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->indexBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLuint), indexPtr, usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return obj;
}

GLLobj *gllObjCopyTexture(GLLobj *obj, const GLubyte *pixelPtr, unsigned width, unsigned height)
{
	glBindTexture(GL_TEXTURE_2D, obj->texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelPtr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	return obj;
}

GLLobj *gllObjMove(GLLobj *obj, float x, float y, float z)
{
	kmMat4Translation((kmMat4 *) tmpMat, x, y, z);
	kmMat4Multiply((kmMat4 *) obj->transform, (kmMat4 *) obj->transform, (kmMat4 *) tmpMat);
	return obj;
}

GLLobj *gllObjRotate(GLLobj *obj, float pitch, float yaw, float roll)
{
	kmMat4RotationYawPitchRoll((kmMat4 *) tmpMat, pitch, yaw, roll);
	kmMat4Multiply((kmMat4 *) obj->transform, (kmMat4 *) obj->transform, (kmMat4 *) tmpMat);
	return obj;
}

GLLobj *gllObjScale(GLLobj *obj, float x, float y, float z)
{
	kmMat4Scaling((kmMat4 *) tmpMat, x, y, z);
	kmMat4Multiply((kmMat4 *) obj->transform, (kmMat4 *) obj->transform, (kmMat4 *) tmpMat);
	return obj;
}

GLLobj *gllObjReset(GLLobj *obj)
{
	kmMat4Identity((kmMat4 *) obj->transform);
	return obj;
}

void gllViewMoveForwards(float t)
{
	camPos[X]+= camDir[X] * t;
	camPos[Y]+= camDir[Y] * t;
	camPos[Z]+= camDir[Z] * t;
}

void gllViewMoveBackwards(float t)
{
	camPos[X]-= camDir[X] * t;
	camPos[Y]-= camDir[Y] * t;
	camPos[Z]-= camDir[Z] * t;
}

void gllViewMoveLeft(float t)
{
	camPos[X]-= camRight[X] * t;
	camPos[Y]-= camRight[Y] * t;
	camPos[Z]-= camRight[Z] * t;
}

void gllViewMoveRight(float t)
{
	camPos[X]+= camRight[X] * t;
	camPos[Y]+= camRight[Y] * t;
	camPos[Z]+= camRight[Z] * t;
}

void gllViewMoveUp(float t)
{
	camPos[X]+= camUp[X] * t;
	camPos[Y]+= camUp[Y] * t;
	camPos[Z]+= camUp[Z] * t;
}

void gllViewMoveDown(float t)
{
	camPos[X]-= camUp[X] * t;
	camPos[Y]-= camUp[Y] * t;
	camPos[Z]-= camUp[Z] * t;
}

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

void gllClear(GLfloat r, GLfloat g, GLfloat b)
{
	glClearColor(r, g, b, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gllRender(GLLobj *obj)
{
	static float mvp[16];
	int bufsize = 0;

	glUseProgram(shdProgO);
	glBindBuffer(GL_ARRAY_BUFFER, obj->vertexBuf);
	glBindVertexArray(obj->vertexArr);
	glBindTexture(GL_TEXTURE_2D, obj->texture);

	kmMat4Multiply((kmMat4 *) mvp, (kmMat4 *) gllProjMatPtr, (kmMat4 *) viewMat);
	kmMat4Multiply((kmMat4 *) mvp, (kmMat4 *) mvp, (kmMat4 *) obj->transform);
	glUniformMatrix4fv(shdProgMvpUniL, 1, GL_FALSE, mvp);
	glUniformMatrix4fv(shdProgModelUniL, 1, GL_FALSE, obj->transform);

	static float lightPos[3] = {0, -3, 1};
	int i;
	for (i = 0; i < 3; ++i)
		lightPos[i]+= (rand() % 101) / 5000.;
	glUniform3f(shdProgLightPosUniL, lightPos[0], lightPos[1], lightPos[2]);

	glUniform3f(shdProgEyePosUniL, camPos[X], camPos[Y], camPos[Z]);
	glUniform3f(shdProgLightPosUniL, camPos[X], camPos[Y], camPos[Z]);

	if (obj->flags & GLL_FLAG_INDICES)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->indexBuf);

		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufsize);
		glDrawElements(GL_TRIANGLES, bufsize / sizeof (GLuint), GL_UNSIGNED_INT, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	else
	{
		glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufsize);
		glDrawArrays(GL_TRIANGLES, 0, bufsize / sizeof (GLfloat) / GLL_VERTEX_STRIDE);
	}

	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
