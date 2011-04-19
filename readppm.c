#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glut.h>


char *readppm(char *filename, int *height, int *width)
{
	FILE *fd;
	int  k;
	char c;
	int i,j;
	char b[100];
	int red, green, blue;
	long numbytes;
	int n;
	int m;
	char *image;
	
	fd = fopen(filename, "r");
	if (fd == NULL)
	{
		printf("Could not open %s\n", filename);
		return NULL;
	}
	c = getc(fd);
	if (c=='P' || c=='p')
		c = getc(fd);
	
	if (c == '3')
	{
		printf("%s is a PPM file (plain text version)\n", filename);
		
		// NOTE: This is not very good PPM code! Comments are not allowed
		// except immediately after the magic number.
		c = getc(fd);
		if (c == '\n' || c == '\r') // Skip any line break and comments
		{
			c = getc(fd);
			while(c == '#') 
			{
				fscanf(fd, "%[^\n\r] ", b);
				printf("%s\n",b);
				c = getc(fd);
			}
			ungetc(c,fd); 
		}
		fscanf(fd, "%d %d %d", &n, &m, &k);

		printf("%d rows  %d columns  max value= %d\n",n,m,k);

		numbytes = n * m * 3;
		image = (char *) malloc(numbytes);
		if (image == NULL)
		{
			printf("Memory allocation failed!\n"); 
			return NULL;
		}
		for(i=n-1;i>=0;i--) for(j=0;j<m;j++)
		{
			fscanf(fd,"%d %d %d",&red, &green, &blue );
			image[(i*n+j)*3]=red * 255 / k;
			image[(i*n+j)*3+1]=green * 255 / k;
			image[(i*n+j)*3+2]=blue * 255 / k;
		}
	}
	else
	if (c == '6')
	{
		printf("%s is a PPM file (raw version)!\n", filename); 

		c = getc(fd);
		if (c == '\n' || c == '\r') // Skip any line break and comments
		{
			c = getc(fd);
			while(c == '#') 
			{
				fscanf(fd, "%[^\n\r] ", b);
				printf("%s\n",b);
				c = getc(fd);
			}
			ungetc(c,fd); 
		}
		fscanf(fd, "%d %d %d", &n, &m, &k);
		printf("%d rows  %d columns  max value= %d\n",n,m,k);
		c = getc(fd); // Skip the last whitespace
		
		numbytes = n * m * 3;
		image = (char *) malloc(numbytes);
		if (image == NULL)
		{
			printf("Memory allocation failed!\n"); 
			return NULL;
		}
		// Read and re-order as necessary
		for(i=n-1;i>=0;i--) for(j=0;j<m;j++)
		{
			image[(i*n+j)*3+0]=getc(fd);
			image[(i*n+j)*3+1]=getc(fd);
			image[(i*n+j)*3+2]=getc(fd);
		}
	}
	else
	{
		printf("%s is not a PPM file!\n", filename); 
		return NULL;
	}
	
	printf("read image\n");
	
	*height = n;
	*width = m;
	return image;
}

/*

int n;
int m;

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2i(0, 0);
		glTexCoord2f(0, 1);
		glVertex2i(0, m);
		glTexCoord2f(1, 1);
		glVertex2i(n ,m);
		glTexCoord2f(1, 0);
		glVertex2i(n, 0);
	glEnd();
	glFlush();
}

void myreshape(int h, int w)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLfloat) n, 0.0, (GLfloat) m);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0,0,h,w);
}

char *image;

int main(int argc, char **argv)
{
    image = readppm("isy.ppm", &n, &m);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(n, m);
	glutInitWindowPosition(50,50);
    glutCreateWindow("image");
    
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glEnable(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D,0,3,n,m,0,GL_RGB,GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glClearColor(1, 1, 1, 1);
	glColor3f(1,1,1);
	
   	glutReshapeFunc(myreshape);
    glutDisplayFunc(display);
	glutMainLoop();
}
*/
