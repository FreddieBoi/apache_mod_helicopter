#ifndef inverse_h
#define inverse_h

#include <GL/glut.h>

void inverse(GLdouble* a, GLdouble* inver);

GLdouble detrm(GLdouble a[4][4], float k);

void cofact(GLdouble num[4][4], GLdouble fac[4][4], float f);

void trans(GLdouble num[4][4], GLdouble fac[4][4], GLdouble inv[4][4], float r);

#endif
