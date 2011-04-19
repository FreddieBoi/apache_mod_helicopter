
#ifndef helpers_h
#define helpers_h


#include <GL/gl.h>

// One-time initialization function.
// Call this after OpenGL has been initialized.
void initHelperLibrary();

// Returns how much wall-clock time has elapsed since the helper library
// was initialized.
float getElapsedTime();

// Loads a texture from disk. PPM and JPG formats are supported.
// Exits program if there is any error.
GLuint loadTexture(char* name);

// Retrieves the object-rotation/translation matrix that is managed
//  by the helper library.
GLdouble* getObjectMatrix();

// Retrieves the free-flight camera matrix that is managed
//  by the helper library.
GLdouble* getCameraMatrix();

void exitGame();

// Special
void updatePosition();

typedef struct
{
  GLfloat* vertexArray;
  GLfloat* normalArray;
  GLfloat* texCoordArray;
  GLfloat* colorArray;
  GLuint* indexArray;
  int numVertices;
  int numIndices;
} Model;


// Loads a model from disk. OBJ format is supported.
// Exits program if there is any error.
Model* loadModel(char* name);

// Deallocates data structures associated with a model.
void freeModel(Model* model);


#endif
