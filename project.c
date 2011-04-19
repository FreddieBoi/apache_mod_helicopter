
#include <GL/glut.h>
#include <math.h>

#include "helpers.h"
#include "inverse.h"

#define crossProduct(a,b,c) \
	(a)[0] = (b)[1] * (c)[2] - (c)[1] * (b)[2]; \
	(a)[1] = (b)[2] * (c)[0] - (c)[2] * (b)[0]; \
	(a)[2] = (b)[0] * (c)[1] - (c)[0] * (b)[1];

int height, width, terrainWidth, terrainHeight, rows, cols;
unsigned char *imagedata;
GLfloat tileWidth, tileHeight, heightScale;
static GLfloat helicopterCameraPosX = 0;
static GLfloat helicopterCameraPosY = -5;
static GLfloat helicopterCameraPosZ = -10;

// Textures
GLuint textureTree,
	groundTexture,
	lakeTexture,
	textureNegativeZ,
	texturePositiveZ,
	textureNegativeY,
	texturePositiveY,
	textureNegativeX,
	texturePositiveX;

// Models
Model* modelTree;
Model* modelApache;
Model* modelRotor;
Model* modelBackRotor;
//Model* modelTeaPot;

// Array used to construct the ground plane.
GLfloat groundVertices[4*1][3] = {{-100, 0, 100}, {-100, 0, -100}, { 100, 0, -100}, { 100, 0, 100}};
GLfloat groundNormals[4*1][3] = {{ 0, 1, 0}, { 0, 1, 0}, { 0, 1, 0}, { 0, 1, 0}};
GLfloat groundTexCoords[4*1][2] = {{0, 0}, {0, 100}, {100, 100}, {100, 0}};
GLfloat groundColors[4*1][3] = {{1,1,1}, {1,1,1}, {1,1,1}, {1,1,1}};
int numGroundIndices = 6;
GLuint groundIndices[6] = { 2, 1, 0, 3, 2, 0 };

GLfloat skyBoxSize;

void norm(GLfloat* a) {
/* norm(vec3) - Normalize the specified vector. */
  GLfloat l = sqrt(pow(a[0],2)+pow(a[1],2)+pow(a[2],2));
  a[0] = a[0]/l;
  a[1] = a[1]/l;
  a[2] = a[2]/l;
}

GLfloat getHeight(GLfloat x, GLfloat z) {
/* getHeight(float, float) - Calculate the height (y-value) of the terrain surface at 
	the specified (x,z) coordinate. */
  // The height to return.
  GLfloat height;

  // Translate the input coordinates to the heightmap (terrain).
  GLfloat realX = x+terrainWidth/2;
  GLfloat realZ = z+terrainHeight/2;

  // Out of bounds (ret -1)
  if(fabs(x) > terrainWidth/2 || fabs(z) > terrainHeight/2) {
    return -1;
  }

  // Step 1: Find what quad the point falls into
  int x2col = realX/tileWidth;
  int z2row = realZ/tileHeight;

  // Step 2: Find what triangle to use
  // The TopLeft corner of the quad, representing the TopLeft triangle.
  GLfloat topleft[2] = {x2col*tileWidth, z2row*tileHeight};
  // The BottomRight corner of the quad, representing the BottomRight triangle.
  GLfloat bottomright[2] = {(x2col+1)*tileWidth, (z2row+1)*tileHeight};

  // Calculate the distances from the (x,z) coordinate to the TopLeft and to the BottomRight corners.
  GLfloat distTL = sqrt(pow(realX-topleft[0],2)+pow(realZ-topleft[1],2));
  GLfloat distBR = sqrt(pow(realX-bottomright[0],2)+pow(realZ-bottomright[1],2));

  // Get some useful imagedata (heights)
  GLfloat curHeight = ((int)imagedata[(z2row*width + x2col)*3])*heightScale;
  GLfloat nextRowHeight = ((int)imagedata[((z2row+1)*width + x2col)*3])*heightScale;
  GLfloat nextColHeight = ((int)imagedata[(z2row*width + (x2col+1))*3])*heightScale;
  GLfloat nextRowColHeight = ((int)imagedata[((z2row+1)*width + (x2col+1))*3])*heightScale;

  // Find the closest corner (and use the associated triangle).
  // The coordinate is closer to TopLeft triangle
  if (distTL < distBR) {
    // Get the height of the TopLeft-most point.
    GLfloat y = ((int)imagedata[(z2row*width + x2col)*3])*heightScale;
    
    // Get the points of the triangle.
    GLfloat p1[3] = {x2col*tileWidth, curHeight, z2row*tileHeight};
    GLfloat p2[3] = {x2col*tileWidth, nextRowHeight, (z2row+1)*tileHeight};
    GLfloat p3[3] = {(x2col+1)*tileWidth, nextColHeight, z2row*tileHeight};
    
    // Construct the normal of the Triangle
    GLfloat n[3] = {0};
    GLfloat v1[3] = {p3[0]-p1[0], p3[1]-p1[1], p3[2]-p1[2]};
    GLfloat v2[3] = {p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2]};
    crossProduct(n,v2,v1);
    norm(n);
    
    // Step 3: Get the height of the (x,z) coordinate by using the Plane equation.
    GLfloat d = n[0]*topleft[0] + n[1]*y + n[2]*topleft[1];
    height = (d-n[0]*realX-n[2]*realZ)/n[1];
  }
  // The coordinate is closer to BottomRight corner
  else {
    // Get the height of the BottomRight-most point.
    GLfloat y = ((int)imagedata[((z2row+1)*width + (x2col+1))*3])*heightScale;

    // Get100 the points of the triangle.
    GLfloat p4[3] = {x2col*tileWidth, nextRowHeight, (z2row+1)*tileHeight};
    GLfloat p5[3] = {(x2col+1)*tileWidth, nextRowColHeight, (z2row+1)*tileHeight};
    GLfloat p6[3] = {(x2col+1)*tileWidth, nextColHeight, z2row*tileHeight};

    // Construct the normal of the Triangle
    GLfloat n[3] = {0};
    GLfloat v3[3] = {p6[0]-p4[0], p6[1]-p4[1], p6[2]-p4[2]};
    GLfloat v4[3] = {p5[0]-p4[0], p5[1]-p4[1], p5[2]-p4[2]};
    crossProduct(n,v4,v3);
    norm(n);

    // Step 3: Get the height of the (x,z) coordinate by using the Plane equation.
    GLfloat d = n[0]*bottomright[0] + n[1]*y + n[2]*bottomright[1];
    height = (d-n[0]*realX-n[2]*realZ)/n[1];
  }
  return height;
}

void renderGround()
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glBegin(GL_POLYGON);
  glNormal3f(0, 1, 0);
  glTexCoord2f(0, 10);
  glVertex3f(-100, -0.1, 100);
  glTexCoord2f(0, 0);
  glVertex3f(100, -0.1, 100);
  glTexCoord2f(10, 0);
  glVertex3f(100, -0.1, -100);
  glTexCoord2f(10, 10);
  glVertex3f(-100, -0.1, -100);
  glEnd();

  glPopAttrib();
}

void renderTerrain()
{
  glPushMatrix();
  glTranslatef(-terrainWidth/2,0,-terrainHeight/2);
  glBindTexture(GL_TEXTURE_2D, groundTexture);
  int row, col;
  float curPosS = 0;
  float curPosT = 0;
  float SStep = 1.0/cols;
  float TStep = 1.0/rows;
  for (row = 0; row < rows; row++) {
    if (curPosT*TStep > 1) {
      curPosT= 0;
    } else {
      curPosT += TStep;
    }
    curPosS = 0;
    for (col = 0; col < cols; col++) {
      if (curPosS*SStep > 1) {
        curPosS = 0;
      } else {
        curPosS += SStep;
      }

      // Get useful heights from imagedata
      GLfloat curHeight = ((int)imagedata[(row*width + col)*3])*heightScale;
      GLfloat nextRowHeight = ((int)imagedata[((row+1)*width + col)*3])*heightScale;
      GLfloat nextColHeight = ((int)imagedata[(row*width + (col+1))*3])*heightScale;
      GLfloat nextRowColHeight = ((int)imagedata[((row+1)*width + (col+1))*3])*heightScale;

      // Compute normal of the triangle given its points.
      GLfloat p1[3] = {col*tileWidth, curHeight, row*tileHeight};
      GLfloat p2[3] = {col*tileWidth, nextRowHeight, (row+1)*tileHeight};
      GLfloat p3[3] = {(col+1)*tileWidth, nextColHeight, row*tileHeight};
      GLfloat n[3] = {0};
      GLfloat v1[3] = {p3[0]-p1[0], p3[1]-p1[1], p3[2]-p1[2]};
      GLfloat v2[3] = {p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2]};
      crossProduct(n,v2,v1);
      norm(n);

      // Shift texture if to draw "the lake", that is if the normal only has a Y-component and no height.
      GLboolean shouldDrawLake = (n[0] == 0 && n[2] == 0 && getHeight(col*tileWidth-terrainWidth/2, row*tileHeight-terrainHeight/2) < 0.01);
      if (shouldDrawLake) {
        glBindTexture(GL_TEXTURE_2D, lakeTexture);
      }

      glBegin(GL_POLYGON);
      glNormal3f(n[0], n[1], n[2]);
      glTexCoord2f(curPosS, curPosT);
      glVertex3f(col*tileWidth, curHeight, row*tileHeight);
      glTexCoord2f(curPosS, curPosT+TStep);
      glVertex3f(col*tileWidth, nextRowHeight, (row+1)*tileHeight);
      glTexCoord2f(curPosS+SStep, curPosT);
      glVertex3f((col+1)*tileWidth, nextColHeight, row*tileHeight);
      glEnd();

      // Shift texture back if needed
      if (shouldDrawLake) {
        glBindTexture(GL_TEXTURE_2D, groundTexture);
      }

      // Compute normal of the triangle given its points.
      GLfloat p4[3] = {col*tileWidth, nextRowHeight, (row+1)*tileHeight};
      GLfloat p5[3] = {(col+1)*tileWidth, nextRowColHeight, (row+1)*tileHeight};
      GLfloat p6[3] = {(col+1)*tileWidth, nextColHeight, row*tileHeight};
      GLfloat v3[3] = {p6[0]-p4[0], p6[1]-p4[1], p6[2]-p4[2]};
      GLfloat v4[3] = {p5[0]-p4[0], p5[1]-p4[1], p5[2]-p4[2]};
      crossProduct(n,v4,v3);
      norm(n);

      // Shift texture if vertex is part of "the lake"
      shouldDrawLake = (n[0] == 0 && n[2] == 0 && getHeight(col*tileWidth-terrainWidth/2, (row+1)*tileHeight-terrainHeight/2) < 0.1);
      if (shouldDrawLake) {
        glBindTexture(GL_TEXTURE_2D, lakeTexture);
      }

      glBegin(GL_POLYGON);
      glNormal3f(n[0], n[1], n[2]);
      glTexCoord2f(curPosS, curPosT+TStep);
      glVertex3f(col*tileWidth, nextRowHeight, (row+1)*tileHeight);
      glTexCoord2f(curPosS+SStep, curPosT+TStep);
      glVertex3f((col+1)*tileWidth, nextRowColHeight, (row+1)*tileHeight);
      glTexCoord2f(curPosS+SStep, curPosT);
      glVertex3f((col+1)*tileWidth, nextColHeight, row*tileHeight);
      glEnd();

      // Shift texture back if needed
      if (shouldDrawLake) {
        glBindTexture(GL_TEXTURE_2D, groundTexture);
      }
    }
  }
  glPopMatrix();
}

void renderSkyBox() {
  GLdouble skyBoxMatrix[16];
  GLdouble* cameraMatrix = getCameraMatrix();
  int i;
  for(i = 0; i < 16; i++) {
    skyBoxMatrix[i] = cameraMatrix[i];
  }
  skyBoxMatrix[12] = 0;
  skyBoxMatrix[13] = 0;
  skyBoxMatrix[14] = 0;
  glLoadMatrixd(skyBoxMatrix);

  // Enable textures.
  glEnable(GL_TEXTURE_2D);

  // Draw the skybox polygons
  glBindTexture(GL_TEXTURE_2D, texturePositiveZ);
  glBegin(GL_POLYGON);
  glColor3f(1, 1, 1);
  glNormal3f(0, 0, 1);
  glTexCoord2f(0, 0);
  glVertex3f(-skyBoxSize, skyBoxSize, -skyBoxSize);
  glTexCoord2f(0, 1);
  glVertex3f(-skyBoxSize, -skyBoxSize, -skyBoxSize);
  glTexCoord2f(1, 1);
  glVertex3f(skyBoxSize, -skyBoxSize, -skyBoxSize);
  glTexCoord2f(1, 0);
  glVertex3f(skyBoxSize, skyBoxSize, -skyBoxSize);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, textureNegativeX);
  glBegin(GL_POLYGON);
  glColor3f(1, 1, 1);
  glNormal3f(1, 0, 0);
  glTexCoord2f(0, 0);
  glVertex3f(-skyBoxSize, skyBoxSize, skyBoxSize);
  glTexCoord2f(0, 1);
  glVertex3f(-skyBoxSize, -skyBoxSize, skyBoxSize);
  glTexCoord2f(1, 1);
  glVertex3f(-skyBoxSize, -skyBoxSize, -skyBoxSize);
  glTexCoord2f(1, 0);
  glVertex3f(-skyBoxSize, skyBoxSize, -skyBoxSize);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, texturePositiveX);
  glBegin(GL_POLYGON);
  glColor3f(1, 1, 1);
  glNormal3f(-1, 0, 0);
  glTexCoord2f(0, 0);
  glVertex3f(skyBoxSize, skyBoxSize, -skyBoxSize);
  glTexCoord2f(0, 1);
  glVertex3f(skyBoxSize, -skyBoxSize, -skyBoxSize);
  glTexCoord2f(1, 1);
  glVertex3f(skyBoxSize, -skyBoxSize, skyBoxSize);
  glTexCoord2f(1, 0);
  glVertex3f(skyBoxSize, skyBoxSize, skyBoxSize);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, textureNegativeZ);
  glBegin(GL_POLYGON);
  glColor3f(1, 1, 1);
  glNormal3f(0, 0, -1);
  glTexCoord2f(0, 0);
  glVertex3f(skyBoxSize, skyBoxSize, skyBoxSize);
  glTexCoord2f(0, 1);
  glVertex3f(skyBoxSize, -skyBoxSize, skyBoxSize);
  glTexCoord2f(1, 1);
  glVertex3f(-skyBoxSize, -skyBoxSize, skyBoxSize);
  glTexCoord2f(1, 0);
  glVertex3f(-skyBoxSize, skyBoxSize, skyBoxSize);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, texturePositiveY);
  glBegin(GL_POLYGON);
  glColor3f(1, 1, 1);
  glNormal3f(0, -1, 0);
  glTexCoord2f(0, 0);
  glVertex3f(-skyBoxSize, skyBoxSize, skyBoxSize);
  glTexCoord2f(0, 1 );
  glVertex3f(-skyBoxSize, skyBoxSize, -skyBoxSize);
  glTexCoord2f(1, 1);
  glVertex3f(skyBoxSize, skyBoxSize,-skyBoxSize);
  glTexCoord2f(1,0);
  glVertex3f(skyBoxSize, skyBoxSize, skyBoxSize);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, textureNegativeY);
  glBegin(GL_POLYGON);
  glColor3f(1, 1, 1);
  glNormal3f(0, 1, 0);
  glTexCoord2f(0, 0);
  glVertex3f(-skyBoxSize, -skyBoxSize, -skyBoxSize);
  glTexCoord2f(0, 1);
  glVertex3f(-skyBoxSize, -skyBoxSize, skyBoxSize);
  glTexCoord2f(1, 1);
  glVertex3f(skyBoxSize, -skyBoxSize, skyBoxSize);
  glTexCoord2f(1, 0);
  glVertex3f(skyBoxSize, -skyBoxSize, -skyBoxSize);
  glEnd();

  glLoadMatrixd(cameraMatrix);
}

void renderProps() {
  // Draw a large Tree
  glPushMatrix();
  glRotatef(90, 0, 0, 1);
  glTranslatef(0, 50, 0);
  glBindTexture(GL_TEXTURE_2D, textureTree);
  glVertexPointer(3, GL_FLOAT, 0, modelTree->vertexArray);
  glNormalPointer(GL_FLOAT, 0, modelTree->normalArray);
  glTexCoordPointer(2, GL_FLOAT, 0, modelTree->texCoordArray);
  glDrawElements(GL_TRIANGLES, modelTree->numIndices, GL_UNSIGNED_INT, 
		 modelTree->indexArray);
  glPopMatrix();

  // Draw a smaller Tree
  glPushMatrix();
  glRotatef(90,0,0,1);
  glTranslatef(0,40,-40);
  glScalef(0.5,0.5,0.5);
  glBindTexture(GL_TEXTURE_2D, textureTree);
  glVertexPointer(3, GL_FLOAT, 0, modelTree->vertexArray);
  glNormalPointer(GL_FLOAT, 0, modelTree->normalArray);
  glTexCoordPointer(2, GL_FLOAT, 0, modelTree->texCoordArray);
  glDrawElements(GL_TRIANGLES, modelTree->numIndices, GL_UNSIGNED_INT, 
		 modelTree->indexArray);
  glPopMatrix();
}

void renderHelicopter() {
  // Draw the helicopter

  // Copy the cameraMatrix.
  GLdouble helicopterMatrix[16];
  GLdouble* cameraMatrix = getCameraMatrix();
  int i;
  for(i = 0; i < 16; i++) {
    helicopterMatrix[i] = cameraMatrix[i];
  }

  // Fix position (relative the camera)
  helicopterMatrix[12] = 0;
  helicopterMatrix[13] = 0;
  helicopterMatrix[14] = 0;

  // Fix rotation (relative the camera)
  helicopterMatrix[0] = 1;
  helicopterMatrix[1] = 0;
  helicopterMatrix[2] = 0;
  helicopterMatrix[4] = 0;
  helicopterMatrix[5] = 1;
  helicopterMatrix[6] = 0;
  helicopterMatrix[8] = 0;
  helicopterMatrix[9] = 0;
  helicopterMatrix[10] = 1;

  // Load the helicopterMatrix instead of cameraMatrix.
  glLoadMatrixd(helicopterMatrix);
  //glPushMatrix();

  // The model is off 90 deg, rotate it.
  glTranslatef(helicopterCameraPosX,helicopterCameraPosY,helicopterCameraPosZ);
  glRotatef(-90, 0,1,0);
  glScalef(0.35f,0.35f,0.35f);

  /*GLdouble* cameraMatrix = getCameraMatrix();
  GLdouble inverseCameraMatrix[16];
  inverse(getCameraMatrix(), inverseCameraMatrix);
  glLoadMatrixd(inverseCameraMatrix);
  printf( "\nTHE INVERSE OF THE MATRIX:\n" );
  int i, j, r;
  r = 4;
  for ( i = 0;i < r;i++ )
  {
  	for ( j = 0;j < r;j++ )
  	{
  		printf( "\t%f", inverseCameraMatrix[i*r+j] );
  	}

  	printf( "\n" );
  }*/
  //glScalef(10,10,10);
  glVertexPointer(3, GL_FLOAT, 0, modelApache->vertexArray);
  glNormalPointer(GL_FLOAT, 0, modelApache->normalArray);
  glTexCoordPointer(2, GL_FLOAT, 0, modelApache->texCoordArray);
  glDrawElements(GL_TRIANGLES, modelApache->numIndices, GL_UNSIGNED_INT, modelApache->indexArray);
  //glPopMatrix();

  // Draw the main rotor
  glPushMatrix();
  glTranslatef(0.1,4.9,-0.10);
  glRotatef(1*360*fmod(getElapsedTime(),60),0,1,0);
  glVertexPointer(3, GL_FLOAT, 0, modelRotor->vertexArray);
  glNormalPointer(GL_FLOAT, 0, modelRotor->normalArray);
  glTexCoordPointer(2, GL_FLOAT, 0, modelRotor->texCoordArray);
  glDrawElements(GL_TRIANGLES, modelRotor->numIndices, GL_UNSIGNED_INT, modelRotor->indexArray);
  glPopMatrix();
  
  // Draw the back rotor
  glPushMatrix();
  glTranslatef(11.7,3.8,1);
  glRotatef(4*360*fmod(getElapsedTime(),60),0,0,1);
  glVertexPointer(3, GL_FLOAT, 0, modelBackRotor->vertexArray);
  glNormalPointer(GL_FLOAT, 0, modelBackRotor->normalArray);
  glTexCoordPointer(2, GL_FLOAT, 0, modelBackRotor->texCoordArray);
  glDrawElements(GL_TRIANGLES, modelBackRotor->numIndices, GL_UNSIGNED_INT, modelBackRotor->indexArray);
  glPopMatrix();
  
  // Load back the cameraMatrix again.
  glLoadMatrixd(cameraMatrix);
}

// Initialize!
void init()
{
  // Place one-time initialization code here

  // Load models...
  modelTree = loadModel("models/tree.obj");
  modelApache = loadModel("models/apache/apache_wo_rotors.obj");
  modelRotor = loadModel("models/apache/main_rotor.obj");
  modelBackRotor = loadModel("models/apache/back_rotor.obj");
  //modelTeaPot = loadModel("../models/various/teapot.obj");
 
  // Load textures..
  // An ordinary texture for the ground plane
  groundTexture = loadTexture("textures/skybox/negativeY.jpg");//"../textures/TropicalFoliage0025_1_S.jpg");

  // Texture for the lake
  lakeTexture = loadTexture("textures/water.jpg");

  // Tree texture
  textureTree = loadTexture("textures/tree.jpg");

  // SkyBox textures
  /*
  textureNegativeZ = loadTexture("blue-sky.jpeg");
  texturePositiveZ = textureNegativeZ;
  textureNegativeY = textureNegativeZ;
  texturePositiveY = textureNegativeZ;
  textureNegativeX = textureNegativeZ;
  texturePositiveX = textureNegativeZ;
  */

  textureNegativeZ = loadTexture("textures/skybox/negativeZ.jpg");
  texturePositiveZ = loadTexture("textures/skybox/positiveZ.jpg");
  textureNegativeY = loadTexture("textures/skybox/negativeY.jpg");
  texturePositiveY = loadTexture("textures/skybox/positiveY.jpg");
  textureNegativeX = loadTexture("textures/skybox/negativeX.jpg");
  texturePositiveX = loadTexture("textures/skybox/positiveX.jpg");

  /*
  textureNegativeZ = loadTexture("../cubemaps/cubemap_berkeley/berkeley_negative_z.jpg");
  texturePositiveZ = loadTexture("../cubemaps/cubemap_berkeley/berkeley_positive_z.jpg");
  textureNegativeY = loadTexture("../cubemaps/cubemap_berkeley/berkeley_negative_y.jpg");
  texturePositiveY = loadTexture("../cubemaps/cubemap_berkeley/berkeley_positive_y.jpg");
  textureNegativeX = loadTexture("../cubemaps/cubemap_berkeley/berkeley_negative_x.jpg");
  texturePositiveX = loadTexture("../cubemaps/cubemap_berkeley/berkeley_positive_x.jpg");
  */

  // A heightmap image
  //read_JPEG_file("asdf.jpeg", &imagedata, &height, &width);
  imagedata = readppm("textures/heightmap/terrain.ppm", &height, &width);

  // terrain and imagedata vars.
  terrainWidth = terrainHeight = 750;
  rows = height-1;
  cols = width-1;
  tileWidth = (float)terrainWidth/cols;
  tileHeight = (float)terrainHeight/rows;
  heightScale = 0.3;

  // The size of the SkyBox
  skyBoxSize = 200;
  //glutGameModeString("1024×768:32@60");
  //glutEnterGameMode();
}

void display()
{
  // This function is called whenever it is time to render
  // a new frame; due to the idle()-function below, this
  // function will get called several times per second

  // Clear framebuffer & zbuffer
  glClearColor(0.3, 0.4, 0.5, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Setup projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(90, 1, 0.01, 500);

  // Setup object matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Fix the SkyBox to the camera.
  renderSkyBox();

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  // Position light 0
  GLfloat light_position[] = { 0.0, 1.0, 0.0, 0.0 }; // Directional from above
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  // Set default material properties
  GLfloat mat_shininess[] = { 50.0 };
  GLfloat mat_diffuseColor[] = { 1.0, 1.0, 1.0, 0.5 };
  GLfloat mat_specularColor[] = { 1.0, 1.0, 1.0, 0.5 };

  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuseColor);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specularColor);

  /*GLfloat ambientColor [] = { 0.0, 0.0, 0.0, 0.0 };
  GLfloat diffuseColor[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat specularColor[] = { 1.0, 1.0, 1.0, 1.0 };
  glLightfv(GL_LIGHT1, GL_AMBIENT, ambientColor);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseColor);
  glLightfv(GL_LIGHT1, GL_SPECULAR, specularColor);*/

  // Enable lighting and light 0
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  //glEnable(GL_LIGHT1);
  glEnable(GL_NORMALIZE);

  // Enable Z-buffering
  glEnable(GL_DEPTH_TEST);

  // Enable Gouraud shading
  glShadeModel(GL_SMOOTH);

  // Enable backface culling
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glPushMatrix();

  // Add a lightsource
  //glMatrixMode(GL_MODELVIEW);
  //GLfloat light_position2[] = { 20*sin(getElapsedTime()), 20, 20*cos(getElapsedTime()), 1.0 };
  //GLfloat light_position2[] = { 0, 0, 0, 1.0 };

  // Enable texturing
  glEnable(GL_TEXTURE_2D);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  // Render the terrain
  renderTerrain();

  // Render some random props
  renderProps();

  // Disable textures
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  // TODO: REMOVE THIS! OR DO SOMETHING COOL.
  GLdouble invCameraMatrix[16];
  inverse(getCameraMatrix(), invCameraMatrix);
  GLdouble heliCameraVec[4] = { helicopterCameraPosX, helicopterCameraPosY, helicopterCameraPosZ, 1 };
  //GLdouble heliFwdCameraVec[4] = { helicopterCameraPosX, helicopterCameraPosY, helicopterCameraPosZ-10, 1 };
  GLdouble heliWorldVec[4];
  //GLdouble heliFwdWorldVec[4];
  int index, multindex;
  for(index = 0; index < 4; index++) {
    heliWorldVec[index] = 0;
  }
  for(index = 0; index < 4; index++) {
    for(multindex = 0; multindex < 4; multindex++) {
      heliWorldVec[index] += invCameraMatrix[(4*multindex)+index] * heliCameraVec[multindex];
      //heliFwdWorldVec[index] += invCameraMatrix[(4*multindex)+index] * heliFwdCameraVec[multindex];
    }
    //printf("%f,", heliWorldVec[index]);
  }
  //printf("\n");
  /*GLdouble rotation[3] = {0};
  GLdouble diffWorld[4];
  GLdouble diffCamera[4];
  int ind;
  for(ind = 0; ind < 4; ind++)
  diffWorld[ind] = heliFwdWorldVec[ind]-heliWorldVec[ind];
  diffCamera[ind] = heliFwdCameraVec[ind]-heliCameraVec[ind];
  crossProduct(rotation, diffWorld, diffCamera);
  printf("(%f,%f,%f)\n", rotation[0], rotation[1], rotation[2]);*/

  GLfloat chopperH = getHeight(heliWorldVec[0], heliWorldVec[2]);
  //printf("Height at chopper: %f\n", chopperH);
  // Check for collision with ground
  if(heliWorldVec[1] <= chopperH || chopperH == -1) {
    printf("THE HELICOPTER CRASHED! THE APP TOO LOL :O\n");
    exitGame();
  }

  // TODO: REMOVE THIS! TEAPOT HITBOX STUFF
  /*glPushMatrix();
  glTranslatef(heliWorldVec[0], heliWorldVec[1], heliWorldVec[2]);
  //glRotatef(90, 0, 0, 1);
  //glBindTexture(GL_TEXTURE_2D, textureTree);
  glVertexPointer(3, GL_FLOAT, 0, modelTeaPot->vertexArray);
  glNormalPointer(GL_FLOAT, 0, modelTeaPot->normalArray);
  glTexCoordPointer(2, GL_FLOAT, 0, modelTeaPot->texCoordArray);
  glDrawElements(GL_TRIANGLES, modelTeaPot->numIndices, GL_UNSIGNED_INT, modelTeaPot->indexArray);
  glPopMatrix();*/

  //glPopMatrix();

  // Update the camera (and helicopter) position
  updatePosition();

  // Render the helicopter
  renderHelicopter();

  //glLoadMatrixd(getCameraMatrix());

  glPopAttrib();

  // Swap front- and backbuffers
  glutSwapBuffers();
}

void idle()
{
  // This function is called whenever the computer is idle

  // As soon as the machine is idle, ask GLUT to trigger rendering of a new
  // frame
  glutPostRedisplay();
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);

  // Configure GLUT:
  //  - framebuffer with RGB + Alpha values per pixel
  //  - Z-buffer
  //  - two sets of above mentioned buffers, so that
  //    doublebuffering is possible
  //
  // Initial window size 800x800
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

  if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) {
    glutGameModeString("1024x768:32@60");
    // start fullscreen game mode
    glutEnterGameMode();
  } else {
    glutInitWindowSize(1024, 768);
    glutCreateWindow("Apache mod_helicopter");
  }

  // Hide the mouse cursor
  glutSetCursor(GLUT_CURSOR_NONE);

  initHelperLibrary();
  init();

  // Register our display- and idle-functions with GLUT
  glutDisplayFunc(display);
  glutIdleFunc(idle);

  // Enter GLUT's main loop; this function will never return
  glutMainLoop();

  return 0;
}
