#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>

#include "zpr.h"
#ifndef mymax
	#define mymax( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef mymin
	#define mymin( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

#define bool int
#define true 1
#define false 0

static double _left   = 0.0;
static double _right  = 0.0;
static double _bottom = 0.0;
static double _top    = 0.0;

// Booleans to monitor when keys (controls) are pressed
static bool _leftPressed   = false;
static bool _rightPressed = false;
static bool _upPressed  = false;
static bool _downPressed  = false;
static bool _forwardPressed = false;
static bool _backwardPressed = false;
static bool _tiltUpPressed = false;
static bool _tiltDownPressed = false;
static bool _turnLeftPressed = false;
static bool _turnRightPressed = false;

static double _objectMatrix[16];
static double _cameraMatrix[16];

static void zprReshape(int w,int h);

// Special
static void pressKey(unsigned char key, int x, int y);
static void releaseKey(unsigned char key, int x, int y);
static void pressSpecialKey(int key, int x, int y);
static void releaseSpecialKey(int key, int x, int y);

static float objectPositionZ = -0.5f;
static float objectRotationHeading = 0.f;
static float objectRotationPitch = 0.f;

static float cameraRotationHeading = 0.f;
static float cameraRotationPitch = 0.f;
static float cameraPositionX = 0.f;
static float cameraPositionY = -100.f;
static float cameraPositionZ = 0.f;

// The position of the helicopter
static GLfloat helicopterCameraPosX = 0;
static GLfloat helicopterCameraPosY = -5;
static GLfloat helicopterCameraPosZ = -10;

// The speed in each direction
float speedX = 0.f;
float speedY = 0.f;
float speedZ = 0.f;

// Default speed multiplier
float speed = 0.5f;

// The maxspeed (in each direction)
float maxSpeed = 2.0f;

// The speed increase step 
float speedInc = 0.1f;
float speedDec = 0.05f;

// The turn speed
float turnSpeed = 0.0f;
float turnSpeedInc = 1.0f;
float turnSpeedDec = 0.05f;
float maxTurnSpeed = 1.5f;

// The tilt speed
float tiltSpeed = 0.0f;
float tiltSpeedInc = 0.5f;
float tiltSpeedDec = 0.05f;
float maxTiltSpeed = 1.0f;

static void updateObjectMatrix()
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glLoadIdentity();
  glTranslatef(0, 0, objectPositionZ);
  glRotatef(objectRotationPitch, 1, 0, 0);
  glRotatef(objectRotationHeading, 0, 1, 0);
  glGetDoublev(GL_MODELVIEW_MATRIX,_objectMatrix);

  glPopMatrix();
  glPopAttrib();
}

static void updateCameraMatrix()
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glLoadIdentity();
  glTranslatef(helicopterCameraPosX, helicopterCameraPosY, helicopterCameraPosZ);
  glRotatef(cameraRotationPitch, 1, 0, 0);
  glRotatef(cameraRotationHeading, 0, 1, 0);
  glTranslatef(cameraPositionX, cameraPositionY, cameraPositionZ);
  glGetDoublev(GL_MODELVIEW_MATRIX,_cameraMatrix);

  glPopMatrix();
  glPopAttrib();
}

void
zprInit()
{
  updateObjectMatrix();
  updateCameraMatrix();

  glutReshapeFunc(zprReshape);

  // Special
  glutSpecialFunc(pressSpecialKey);
  glutSpecialUpFunc(releaseSpecialKey);

  glutKeyboardFunc(pressKey);
  glutKeyboardUpFunc(releaseKey);
}

static void pressSpecialKey(int key, int x, int y) {
  switch (key) {
    case GLUT_KEY_LEFT : _turnLeftPressed = true; break;
    case GLUT_KEY_RIGHT : _turnRightPressed = true; break;
    case GLUT_KEY_UP : _tiltDownPressed = true; break;
    case GLUT_KEY_DOWN : _tiltUpPressed = true; break;
  }
}

static void pressKey(unsigned char key, int x, int y) {
  switch (key) {
    case 27 : zprExit(); break; // Exit when 'Escape' is hit
    case 'a' : _leftPressed = true; break;
    case 'd' : _rightPressed = true; break;
    case 'w' : _forwardPressed = true; break;
    case 's' : _backwardPressed = true; break;
    case 'q' : _upPressed = true; break;
    case 'e' : _downPressed = true; break;
  }
}

static void releaseSpecialKey(int key, int x, int y) {
  switch (key) {
    case GLUT_KEY_LEFT : _turnLeftPressed = false; break;
    case GLUT_KEY_RIGHT : _turnRightPressed = false; break;
    case GLUT_KEY_UP : _tiltDownPressed = false; break;
    case GLUT_KEY_DOWN : _tiltUpPressed = false; break;
  }
}

static void releaseKey(unsigned char key, int x, int y) {
  switch (key) {
    case 'a' : _leftPressed = false; break;
    case 'd' : _rightPressed = false; break;
    case 'w' : _forwardPressed = false; break;
    case 's' : _backwardPressed = false; break;
    case 'q' : _upPressed = false; break;
    case 'e' : _downPressed = false; break;
  }
}

GLdouble*
zprGetObjectMatrix()
{
  return _objectMatrix;
}

GLdouble*
zprGetCameraMatrix()
{
  return _cameraMatrix;
}

void zprUpdate() {
  // Update the helicopter movement.

  // Update speedX
  if (_leftPressed)
    speedX += speedInc;
  else if (speedX > 0.0f) {
      speedX -= speedDec;
      speedX = mymax(speedX, 0.0f);
  }
  if (_rightPressed)
    speedX -= speedInc;
  else if (speedX < 0.0f) {
      speedX += speedDec;
      speedX = mymin(speedX, 0.0f);
  }

  // Update speedZ
  if (_forwardPressed) 
    speedZ += speedInc;
  else {
    if (speedZ > 0.0f) {
      speedZ -= speedDec;
      speedZ = mymax(speedZ, 0.0f);
    }
  }
  if (_backwardPressed)
    speedZ -= speedInc;
  else {
    if (speedZ < 0.0f) {
      speedZ += speedDec;
      speedZ = mymin(speedZ, 0.0f);
    }
  }

  // Update speedY
  if (_upPressed)
    speedY -= speedInc;
  else {
    if (speedY < 0.0f) {
      speedY += speedDec;
      speedY = mymin(speedY, 0.0f);
    }
  }
  if (_downPressed)
    speedY += speedInc;
  else {
    if (speedY > 0.0f) {
      speedY -= speedDec;
      speedY = mymax(speedY, 0.0f);
    }
  }

  // Tilting (rotation around x-axis)
  if (_tiltUpPressed) {
    tiltSpeed -= tiltSpeedInc;
  } else if (tiltSpeed < 0) {
    tiltSpeed += tiltSpeedDec;
    tiltSpeed = mymin(tiltSpeed, 0.0f);
  }
  if (_tiltDownPressed) {
    tiltSpeed += tiltSpeedInc;
  } else if (tiltSpeed > 0) {
    tiltSpeed -= tiltSpeedDec;
    tiltSpeed = mymax(tiltSpeed, 0.0f);
  }

  // Turning (rotation around y-axis)
  if (_turnLeftPressed) {
    turnSpeed -= turnSpeedInc;
  } else if (turnSpeed < 0) {
    turnSpeed += turnSpeedDec;
    turnSpeed = mymin(turnSpeed, 0.0f);
  }
  if (_turnRightPressed) {
    turnSpeed += turnSpeedInc;
  } else if (turnSpeed > 0) {
    turnSpeed -= turnSpeedDec;
    turnSpeed = mymax(turnSpeed, 0.0f);
  }

  // Handle maximum speed
  if (speedX > maxSpeed)
    speedX = maxSpeed;
  if (speedX < -maxSpeed)
    speedX = -maxSpeed;
  if (speedY > maxSpeed)
    speedY = maxSpeed;
  if (speedY < -maxSpeed)
    speedY = -maxSpeed;
  if (speedZ > maxSpeed)
    speedZ = maxSpeed;
  if (speedZ < -maxSpeed)
    speedZ = -maxSpeed;
  if(turnSpeed > maxTurnSpeed)
    turnSpeed = maxTurnSpeed;
  if(turnSpeed < -maxTurnSpeed)
    turnSpeed = -maxTurnSpeed;
  if(tiltSpeed > maxTiltSpeed)
    tiltSpeed = maxTiltSpeed;
  if(tiltSpeed < -maxTiltSpeed)
    tiltSpeed = -maxTiltSpeed;

  // Do turn
  cameraRotationHeading += turnSpeed;
  objectRotationHeading += turnSpeed;

  // Do tilt
  objectRotationPitch += tiltSpeed;
  cameraRotationPitch += tiltSpeed;

  // Do move
  cameraPositionX += _cameraMatrix[0] * speed * speedX;
  cameraPositionY += _cameraMatrix[4] * speed * speedX;
  cameraPositionZ += _cameraMatrix[8] * speed * speedX;

  cameraPositionX += _cameraMatrix[1] * speed * speedY;
  cameraPositionY += _cameraMatrix[5] * speed * speedY;
  cameraPositionZ += _cameraMatrix[9] * speed * speedY;

  cameraPositionX += _cameraMatrix[2] * speed * speedZ;
  cameraPositionY += _cameraMatrix[6] * speed * speedZ;
  cameraPositionZ += _cameraMatrix[10] * speed * speedZ;

  // Update matrices
  updateObjectMatrix();
  updateCameraMatrix();

}

static void
zprReshape(int w,int h)
{
  _top    =  1.0;
  _bottom = -1.0;
  _left   = -(double)w/(double)h;
  _right  = -_left;
}

void zprExit() {
  // Leave game mode if active
  if (glutGameModeGet(GLUT_GAME_MODE_ACTIVE)) {
    glutLeaveGameMode();
  }
  // Exit program
  exit(0);
}

