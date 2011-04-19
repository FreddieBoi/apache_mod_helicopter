
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "helpers.h"
#include "readppm.h"
#include "zpr.h"
#include "readjpeg.h"
#include "loadobj.h"



static double startTime = 0;

void resetElapsedTime()
{
  struct timeval timeVal;
  gettimeofday(&timeVal, 0);
  startTime = (double) timeVal.tv_sec + (double) timeVal.tv_usec * 0.000001;
}

void initHelperLibrary()
{
  resetElapsedTime();

  zprInit();
}


float getElapsedTime()
{
  struct timeval timeVal;
  gettimeofday(&timeVal, 0);
  double currentTime = (double) timeVal.tv_sec
    + (double) timeVal.tv_usec * 0.000001;

  return currentTime - startTime;
}


GLuint loadTexture(char* name)
{
  GLuint texNum;
  int width = 0, height = 0;
  char* pixelData = 0;
  int nameLen = strlen(name);

  printf("Loading texture %s\n", name);

  if ((nameLen >= 4 && (!strcmp(name + nameLen - 4, ".jpg")
			|| !strcmp(name + nameLen - 4, ".JPG")))
      || (nameLen >= 5 && (!strcmp(name + nameLen - 5, ".jpeg")
			   || !strcmp(name + nameLen - 5, ".JPEG"))))
    {
      read_JPEG_file(name, &pixelData, &width, &height);
    }
  else if (nameLen >= 4 && (!strcmp(name + nameLen - 4, ".ppm")
			    || !strcmp(name + nameLen - 4, ".PPM")))
    {
      pixelData = readppm(name, &width, &height);
    }

  if (!pixelData)
    exit(0);

  glGenTextures(1, &texNum);
  glBindTexture(GL_TEXTURE_2D, texNum);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0,
	       GL_RGB, GL_UNSIGNED_BYTE, pixelData);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  free(pixelData);

  return texNum;
}

GLdouble* getObjectMatrix()
{
  return zprGetObjectMatrix();
}

GLdouble* getCameraMatrix()
{
  return zprGetCameraMatrix();
}

void updatePosition() {
  zprUpdate();
}

Model* loadModel(char* name)
{
  Model* model = 0;
  int nameLen = strlen(name);

  printf("Loading model %s\n", name);

  if (nameLen >= 4 && (!strcmp(name + nameLen - 4, ".obj")
		       || !strcmp(name + nameLen - 4, ".OBJ")))
    {
      model = loadOBJModel(name);
    }
  else
    {
      fprintf(stderr, "Unknown file extension for file %s\n", name);
      fflush(stderr);
    }

  if (!model)
    exit(0);

  printf("Model has vertex colors: %s\n",
	 model->colorArray ? "Yes" : "No");
  printf("Model has vertex normals: %s\n", model->normalArray ? "Yes" : "No");
  printf("Model has texture coordinates: %s\n",
	 model->texCoordArray ? "Yes" : "No");

  return model;
}

void freeModel(Model* model)
{
  if (model)
    {
      if (model->vertexArray)
	free(model->vertexArray);
      if (model->normalArray)
	free(model->normalArray);
      if (model->texCoordArray)
	free(model->texCoordArray);
      if (model->colorArray)
	free(model->colorArray);
      if (model->indexArray)
	free(model->indexArray);
      free(model);
    }
}

void exitGame()
{
  zprExit();
}
