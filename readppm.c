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
