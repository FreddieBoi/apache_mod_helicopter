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

/*static int  _mouseX      = 0;
static int  _mouseY      = 0;
static bool _mouseLeft   = false;
static bool _mouseMiddle = false;
static bool _mouseRight  = false;*/

// Special
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
//static void zprMouse(int button, int state, int x, int y);
//static void zprMotion(int x, int y);
//static void zprKey(unsigned char key, int x, int y);

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

static GLfloat helicopterCameraPosX = 0;
static GLfloat helicopterCameraPosY = -5;
static GLfloat helicopterCameraPosZ = -10;

// Special
float speed = 0.5f;
float speedInc = 0.1f;
float speedX = 0.f;
float speedY = 0.f;
float speedZ = 0.f;
float turnSpeed = 0.0f;
float tiltSpeed = 0.0f;
float maxSpeed = 2.0f;
float speedDec = 0.05f;
float turnSpeedInc = 1.0f;
float turnSpeedDec = 0.05f;
float tiltSpeedInc = 0.5f;
float tiltSpeedDec = 0.05f;
float maxTurnSpeed = 1.5f;
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
  //glutMouseFunc(zprMouse);
  //glutMotionFunc(zprMotion);
  //glutKeyboardFunc(zprKey);

  // Special
  glutSpecialFunc(pressSpecialKey);
  glutSpecialUpFunc(releaseSpecialKey);

  glutKeyboardFunc(pressKey);
  glutKeyboardUpFunc(releaseKey);
}

// Special
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
    case 27 : zprExit(); break;
    case 'a' : _leftPressed = true; break;
    case 'd' : _rightPressed = true; break;
    case 'w' : _forwardPressed = true; break;
    case 's' : _backwardPressed = true; break;
    case 'q' : _upPressed = true; break;
    case 'e' : _downPressed = true; break;
  }
}

// Special
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

/*static void
zprMouse(int button, int state, int x, int y)
{
  _mouseX = x;
  _mouseY = y;

  if (state==GLUT_UP)
    switch (button)
      {
      case GLUT_LEFT_BUTTON:   _mouseLeft   = false; break;
      case GLUT_MIDDLE_BUTTON: _mouseMiddle = false; break;
      case GLUT_RIGHT_BUTTON:  _mouseRight  = false; break;
      }
  else
    switch (button)
      {
      case GLUT_LEFT_BUTTON:   _mouseLeft   = true; break;
      case GLUT_MIDDLE_BUTTON: _mouseMiddle = true; break;
      case GLUT_RIGHT_BUTTON:  _mouseRight  = true; break;
      }
}*/

/*static void
zprKey(unsigned char key, int x, int y)
{
  switch (key)
   {
   case 'w':
     speedZ = 1.f;
     break;
   case 's':
     speedZ = -1.f;
     break;
   case 'q':
     speedY = -1.f;
     break;
   case 'e':
     speedY = 1.f;
     break;
   case 'd':
     speedX = -1.f;
     break;
   case 'a':
     speedX = 1.f;
     break;
   }

  cameraPositionX += _cameraMatrix[0] * speed * speedX;
  cameraPositionY += _cameraMatrix[4] * speed * speedX;
  cameraPositionZ += _cameraMatrix[8] * speed * speedX;
  
  cameraPositionX += _cameraMatrix[1] * speed * speedY;
  cameraPositionY += _cameraMatrix[5] * speed * speedY;
  cameraPositionZ += _cameraMatrix[9] * speed * speedY;

  cameraPositionX += _cameraMatrix[2] * speed * speedZ;
  cameraPositionY += _cameraMatrix[6] * speed * speedZ;
  cameraPositionZ += _cameraMatrix[10] * speed * speedZ;

  updateCameraMatrix();
}*/

/*static void
zprMotion(int x, int y)
{
  bool changed = true;

  const int dx = x - _mouseX;
  const int dy = y - _mouseY;

  if (dx==0 && dy==0)
    return;

  if (_mouseRight)
    {
      objectPositionZ += dy * 0.01f;
      changed = true;
    }
  else if (_mouseLeft)
    {
      objectRotationHeading += dx * 0.3f;
      objectRotationPitch += dy * 0.3f;
      changed = true;
    }

  if (changed)
    {
      updateObjectMatrix();
    }


  if (_mouseMiddle)
    {
      float speed = 0.05f;
      cameraPositionX += _cameraMatrix[0] * speed * -dx;
      cameraPositionY += _cameraMatrix[4] * speed * -dx;
      cameraPositionZ += _cameraMatrix[8] * speed * -dx;

      cameraPositionX += _cameraMatrix[1] * speed * dy;
      cameraPositionY += _cameraMatrix[5] * speed * dy;
      cameraPositionZ += _cameraMatrix[9] * speed * dy;
    }
  else if (_mouseRight)
    {
      float speed = 0.05f;
      cameraPositionX += _cameraMatrix[2] * speed * -dy;
      cameraPositionY += _cameraMatrix[6] * speed * -dy;
      cameraPositionZ += _cameraMatrix[10] * speed * -dy;
    }
  else if (_mouseLeft)
    {
      cameraRotationHeading += dx * 0.3f;
      cameraRotationPitch += dy * 0.3f;
    }


  updateCameraMatrix();

  _mouseX = x;
  _mouseY = y;

}*/
