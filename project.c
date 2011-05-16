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

// Random pre-generated number for placement/rotation of trees.
GLfloat treePositions[30] = { 149, 159, 159, 142, 129, 139, 140, 142, 127, 118, 113, 155, 115, 108, 146, 145, 128, 140, 126, 104, 119, 110, 108, 151, 154, 140, 108, 151, 156, 144 };
GLfloat treeRotations[30] = { 330, 309, 299, 192, 267, 65, 60, 321, 70, 160, 142, 19, 358, 335, 58 };

// Textures
GLuint textureTree,
      groundTexture,
      lakeTexture,
      helicopterTexture,
      textureRotor,
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

// Array used to construct the ground plane.
GLfloat groundVertices[4*1][3] = {{-100, 0, 100}, {-100, 0, -100}, { 100, 0, -100}, { 100, 0, 100}};
GLfloat groundNormals[4*1][3] = {{ 0, 1, 0}, { 0, 1, 0}, { 0, 1, 0}, { 0, 1, 0}};
GLfloat groundTexCoords[4*1][2] = {{0, 0}, {0, 100}, {100, 100}, {100, 0}};
GLfloat groundColors[4*1][3] = {{1,1,1}, {1,1,1}, {1,1,1}, {1,1,1}};
int numGroundIndices = 6;
GLuint groundIndices[6] = { 2, 1, 0, 3, 2, 0 };

GLfloat skyBoxSize;

void norm(GLfloat* a)
{
  // Normalize vector.
  // 	-- Normalize the specified vector.

  GLfloat l = sqrt(pow(a[0],2)+pow(a[1],2)+pow(a[2],2));
  a[0] = a[0]/l;
  a[1] = a[1]/l;
  a[2] = a[2]/l;
}

GLfloat getHeight(GLfloat x, GLfloat z)
{
  // Get the height of the terrain at the specified x and z world coordinates
  // 	-- Calculates the height (y-value) of the terrain surface at the specified (x,z) world coordinate.

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

void renderTerrain()
{
  // Render the terrain
  // 	-- Render the terrain using a heightmap

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

void renderSkyBox()
{
  // Render the SkyBox.
  // 	-- Renders the SkyBox sides and fixes them to the camera position.

  // Make sure the SkyBox is fixed to the camera
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

void renderProps()
{
  // Render props.
  // 	-- Render some trees and stuff to make the world look more alive */

  // Draw a Tree
  int treeX, treeZ;
  treeX = treeZ = 100;
  glPushMatrix();
  glTranslatef(treeX, getHeight(treeX, treeZ), treeZ);
  glRotatef(90, 0, 0, 1);
  glScalef(0.5,0.5,0.5);
  glBindTexture(GL_TEXTURE_2D, textureTree);
  glVertexPointer(3, GL_FLOAT, 0, modelTree->vertexArray);
  glNormalPointer(GL_FLOAT, 0, modelTree->normalArray);
  glTexCoordPointer(2, GL_FLOAT, 0, modelTree->texCoordArray);
  glDrawElements(GL_TRIANGLES, modelTree->numIndices, GL_UNSIGNED_INT, 
		 modelTree->indexArray);
  glPopMatrix();

  // Draw forest
  int i;
  for(i = 0; i < 15; i++) {
    glPushMatrix();
    treeX = treePositions[i];
    treeZ = -treePositions[15+i];
    glTranslatef(treeX, getHeight(treeX, treeZ), treeZ);
    glRotatef(90,0,0,1);
    glRotatef(treeRotations[i],1,0,0);
    glScalef(0.3,0.3,0.3);
    glBindTexture(GL_TEXTURE_2D, textureTree);
    glVertexPointer(3, GL_FLOAT, 0, modelTree->vertexArray);
    glNormalPointer(GL_FLOAT, 0, modelTree->normalArray);
    glTexCoordPointer(2, GL_FLOAT, 0, modelTree->texCoordArray);
    glDrawElements(GL_TRIANGLES, modelTree->numIndices, GL_UNSIGNED_INT, 
                  modelTree->indexArray);
    glPopMatrix();
  }

}

void renderHelicopter()
{
  // Render the helicopter.
  // 	-- Render the helicopter a fix position relative to the camera.
  // 	-- Rotate the blades of the main and back rotors.

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

  // Move it to its position (relative to the camera) to achieve third person view.
  glTranslatef(helicopterCameraPosX,helicopterCameraPosY,helicopterCameraPosZ);
  // The model is off 90 deg, rotate it.
  glRotatef(-90, 0,1,0);
  // Make it rather small
  glScalef(0.35f,0.35f,0.35f);

  // Render the helicopter
  glBindTexture(GL_TEXTURE_2D, helicopterTexture);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

  GLfloat planeS[] = {1, 0.0, 0.0, 1};
  GLfloat planeT[] = {0.0, 1, 0.0, 1};
  glTexGenfv(GL_S, GL_OBJECT_PLANE, planeS);
  glTexGenfv(GL_T, GL_OBJECT_PLANE, planeT);

  glVertexPointer(3, GL_FLOAT, 0, modelApache->vertexArray);
  glNormalPointer(GL_FLOAT, 0, modelApache->normalArray);
  //glTexCoordPointer(2, GL_FLOAT, 0, modelApache->texCoordArray);
  glDrawElements(GL_TRIANGLES, modelApache->numIndices, GL_UNSIGNED_INT, modelApache->indexArray);

  glBindTexture(GL_TEXTURE_2D, textureRotor);

  // Draw the main rotor
  glPushMatrix();
  glTranslatef(0.1,4.9,-0.10);
  glRotatef(1*360*fmod(getElapsedTime(),60),0,1,0);
  //glColorPointer(3, GL_FLOAT, 0, modelRotor->colorArray);
  glVertexPointer(3, GL_FLOAT, 0, modelRotor->vertexArray);
  glNormalPointer(GL_FLOAT, 0, modelRotor->normalArray);
  //glTexCoordPointer(2, GL_FLOAT, 0, modelRotor->texCoordArray);
  glDrawElements(GL_TRIANGLES, modelRotor->numIndices, GL_UNSIGNED_INT, modelRotor->indexArray);
  glPopMatrix();

  // Draw the back rotor
  glPushMatrix();
  glTranslatef(11.7,3.8,1);
  glRotatef(4*360*fmod(getElapsedTime(),60),0,0,1);
  glVertexPointer(3, GL_FLOAT, 0, modelBackRotor->vertexArray);
  glNormalPointer(GL_FLOAT, 0, modelBackRotor->normalArray);
 //glTexCoordPointer(2, GL_FLOAT, 0, modelBackRotor->texCoordArray);
  glDrawElements(GL_TRIANGLES, modelBackRotor->numIndices, GL_UNSIGNED_INT, modelBackRotor->indexArray);
  glPopMatrix();

  // Load back the cameraMatrix again.
  glLoadMatrixd(cameraMatrix);
}

void handleCollisions()
{
  // Handle collisions.
  // 	-- Check if the helicopter collides with the ground terrain

  // Calculate the inverse camera matrix
  GLdouble invCameraMatrix[16];
  inverse(getCameraMatrix(), invCameraMatrix);

  // Get the helicopter position (in camera coordinates)
  GLdouble heliCameraVec[4] = { helicopterCameraPosX, helicopterCameraPosY, helicopterCameraPosZ, 1 };

  // The vector to hold the world position of the helicopter
  GLdouble heliWorldVec[4];
  int index, multindex;

  // TODO: REMOVE SINCE NOT NEEDED?
  // Clear the previous position
  for(index = 0; index < 4; index++) {
    heliWorldVec[index] = 0;
  }

  // Get the new helicopter position in the world using the inverse camera matrix
  // and the helicopter position (in camera coordinates)
  for(index = 0; index < 4; index++) {
    for(multindex = 0; multindex < 4; multindex++) {
      heliWorldVec[index] += invCameraMatrix[(4*multindex)+index] * heliCameraVec[multindex];
    }
  }

  GLfloat chopperH = getHeight(heliWorldVec[0], heliWorldVec[2]);

  // Check for collision with ground
  if(heliWorldVec[1] <= chopperH || chopperH == -1) {
    printf("THE HELICOPTER CRASHED! THE APP TOO LOL :O\n");
    exitGame();
  }
}

void init()
{
  // Initialize!
  // 	-- Place one-time initialization code here

  // Load models...
  modelTree = loadModel("models/tree.obj");
  modelApache = loadModel("models/apache/apache_wo_rotors.obj");
  modelRotor = loadModel("models/apache/main_rotor.obj");
  modelBackRotor = loadModel("models/apache/back_rotor.obj");

  // Load textures..
  // The texture used for the terrain
  groundTexture = loadTexture("textures/skybox/negativeY.jpg");//"../textures/TropicalFoliage0025_1_S.jpg");

  // Texture for the lake
  lakeTexture = loadTexture("textures/water.jpg");

  // Tree texture
  textureTree = loadTexture("textures/tree.jpg");

  // Helicopter texture
  helicopterTexture = loadTexture("textures/helicopter.jpg");

  textureRotor = loadTexture("textures/rotor.jpg");

  // SkyBox textures
  textureNegativeZ = loadTexture("textures/skybox/negativeZ.jpg");
  texturePositiveZ = loadTexture("textures/skybox/positiveZ.jpg");
  textureNegativeY = loadTexture("textures/skybox/negativeY.jpg");
  texturePositiveY = loadTexture("textures/skybox/positiveY.jpg");
  textureNegativeX = loadTexture("textures/skybox/negativeX.jpg");
  texturePositiveX = loadTexture("textures/skybox/positiveX.jpg");

  // A heightmap image
  imagedata = readppm("textures/heightmap/heightmap.ppm", &height, &width);

  // terrain and imagedata vars.
  terrainWidth = terrainHeight = 750;
  rows = height-1;
  cols = width-1;
  tileWidth = (float)terrainWidth/cols;
  tileHeight = (float)terrainHeight/rows;
  heightScale = 0.3;

  // The size of the SkyBox
  skyBoxSize = 200;
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
  //GLfloat mat_specularColor[] = { 1.0, 1.0, 1.0, 0.5 };

  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuseColor);
  //glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specularColor);

  // Enable lighting and light 0
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_NORMALIZE);

  // Enable Z-buffering
  glEnable(GL_DEPTH_TEST);

  // Enable Gouraud shading
  glShadeModel(GL_SMOOTH);

  // Enable backface culling
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glPushMatrix();

  // Enable texturing
  glEnable(GL_TEXTURE_2D);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  // Render the terrain
  renderTerrain();

  // Render some random props (like trees)
  renderProps();

  // Check if the helicopter hits the ground (if so, exit!)
  handleCollisions();

  // Update the camera (and helicopter) position
  updatePosition();

  // Render the helicopter
  renderHelicopter();

    // Disable textures
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

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
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

  // Print the title
  printf("\nApache mod_helicopter\n\n");

  // Handle arguments
  int window_mode;
  window_mode = 0;
  // Use (forced) window mode?
  if(argc > 1 && (strcmp(argv[1], "-w") == 0)) {
    printf("	-w	Using window mode\n");
    window_mode = 1;

  }

  // Clear some space before program output
  printf("\n\n");

  // Setup window (1024x768)
  // Check if fullscreen is possible
  if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE) && !(window_mode == 1)) {
    // Start fullscreen game mode
    glutGameModeString("1024x768:32@60");
    glutEnterGameMode();
  } else {
    // Use regular window
    glutInitWindowSize(1024, 768);
    glutCreateWindow("Apache mod_helicopter");
  }

  // Hide the mouse cursor
  glutSetCursor(GLUT_CURSOR_NONE);

  // Initialize everything!
  initHelperLibrary();
  init();

  // Register our display- and idle-functions with GLUT
  glutDisplayFunc(display);
  glutIdleFunc(idle);

  // Enter GLUT's main loop; this function will never return
  glutMainLoop();

  return 0;
}
