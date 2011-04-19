
#ifndef readppm_h
#define readppm_h

// Load .PPM format image from disk.
// input:
//  filename - on-disk name of file
// output:
//  height - if file is loaded, the height of the image
//           will be written here
//  width - if file is loaded, the width of the image
//          will be written here
//  return value - pointer to pixel data, or 0 on error

char *readppm(char *filename, int *height, int *width);


#endif
