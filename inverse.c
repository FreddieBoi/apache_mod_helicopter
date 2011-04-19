#include<stdio.h>
#include<math.h>
#include "inverse.h"
#include <GL/glut.h>

void inverse(GLdouble* a, GLdouble* inver)
{
   GLdouble b[4][4];
   // The order of the matrix.
    float k;
    k = 4;
    //printf( "ENTER THE ORDER OF THE MATRIX:\n" );
    //scanf( "%f", &k );
    //printf( "ENTER THE ELEMENTS OF THE MATRIX:\n" );
    int i, j;
    for ( i = 0;i < k;i++ ) {
	for ( j = 0;j < k;j++ ) {
		b[i][j] = a[i*4+j];
		// printf("%f",b[i][j]);
	}
    }
 
    /*d = detrm( a, k );
    printf( "THE DETERMINANT IS=%f", d );*/
 
    if (detrm(b, k) == 0)
        printf( "\nMATRIX IS NOT INVERSIBLE\n" );
    else {
	GLdouble fac[4][4];
	cofact(b, fac, k);
	GLdouble myinv[4][4];
	trans(b, fac, myinv, k);
   
	int i, j;
	for ( i = 0;i < k;i++ ) {
		for ( j = 0;j < k;j++ ){
			inver[i*4+j] = myinv[i][j];
			//printf("%f, ", inver[i*4+j]);
		}
	}
	//printf("\n");
    }
}
 
/******************FUNCTION TO FIND THE DETERMINANT OF THE MATRIX************************/
 
GLdouble detrm(GLdouble a[4][4], float k )
{
    GLdouble b[4][4];
    GLdouble s = 1, det = 0;
    int i, j, m, n, c;
 
    if ( k == 1 )
        {
            return ( a[0][0] );
        }
    else
        {
            det = 0;
 
            for ( c = 0;c < k;c++ )
                {
                    m = 0;
                    n = 0;
 
                    for ( i = 0;i < k;i++ )
                        {
                            for ( j = 0;j < k;j++ )
                                {
                                    b[ i ][ j ] = 0;
 
                                    if ( i != 0 && j != c )
                                        {
                                            b[ m ][ n ] = a[ i ][ j ];
 
                                            if ( n < ( k - 2 ) )
                                                n++;
                                            else
                                                {
                                                    n = 0;
                                                    m++;
                                                }
                                        }
                                }
                        }
                    det = det + s * ( a[ 0 ][ c ] * detrm( b, k - 1 ) );
                    s = -1 * s;
                }
        }
    return ( det );
}
 
/*******************FUNCTION TO FIND COFACTOR*********************************/
 
void cofact(GLdouble num[4][4], GLdouble fact[4][4], float f)
{
    GLdouble b[4][4];
    int p, q, m, n, i, j;
 
    for ( q = 0;q < f;q++ )
        {
            for ( p = 0;p < f;p++ )
                {
                    m = 0;
                    n = 0;
 
                    for ( i = 0;i < f;i++ )
                        {
                            for ( j = 0;j < f;j++ )
                                {
                                    b[ i ][ j ] = 0;
 
                                    if ( i != q && j != p )
                                        {
                                            b[ m ][ n ] = num[ i ][ j ];
 
                                            if ( n < ( f - 2 ) ) {
                                                n++;
					    }
                                            else
                                                {
                                                    n = 0;
                                                    m++;
                                                }
                                        }
                                }
                        }
 
                    fact[ q ][ p ] = pow( -1, q + p ) * detrm( b, f - 1 );
                }
        }
}
 
/*************FUNCTION TO FIND TRANSPOSE AND INVERSE OF A MATRIX**************************/
 
void trans(GLdouble num[4][4], GLdouble fact[4][4], GLdouble inv[4][4], float r)
{
    int i, j;
    GLdouble b[4][4];
    GLdouble d;
 
    for ( i = 0;i < r;i++ )
        {
            for ( j = 0;j < r;j++ )
                {
                    b[ i ][ j ] = fact[ j ][ i ];
                }
        }
 
    d = detrm( num, r );
    inv[ i ][ j ] = 0;
 
    for ( i = 0;i < r;i++ )
        {
            for ( j = 0;j < r;j++ )
                {
                    inv[ i ][ j ] = b[ i ][ j ] / d;
                }
        }
 
   /* printf( "\nTHE INVERSE OF THE MATRIX:\n" );
 
    for ( i = 0;i < r;i++ )
        {
            for ( j = 0;j < r;j++ )
                {
                    printf( "\t%f", inv[ i ][ j ] );
                }
 
            printf( "\n" );
        }*/

}
