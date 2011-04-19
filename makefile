
all : project

project : project.c inverse.c helpers.c readppm.c readjpeg.c zpr.c loadobj.c
	gcc -Wall -o project -lGL -lglut -ljpeg project.c inverse.c helpers.c readppm.c readjpeg.c zpr.c loadobj.c

clean :
	rm project
