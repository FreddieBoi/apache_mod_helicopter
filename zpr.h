#ifndef ZPR_H
#define ZPR_H

/*
 * Zoom-pan-rotate mouse manipulation module for GLUT
 *
 * Originally written by Nigel Stewart
 *
 * Hacked quite a bit by Mikael Kalms
 */

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/glut.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include <math.h>

void zprInit();
GLdouble* zprGetObjectMatrix();
GLdouble* zprGetCameraMatrix();
void zprUpdate();
void zprExit();

#ifdef __cplusplus
}
#endif

#endif
