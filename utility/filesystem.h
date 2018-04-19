/*
 * Author: 			Ben Tomlin
 * Student Id:		btomlin
 * Student Nbr:		834198
 * Date:			Apr 2018
 */

#ifndef UTILITY_FILESYSTEM_H_
#define UTILITY_FILESYSTEM_H_

#include <unistd.h> //testFile() arguments
#include "bool.h" //testFile() return values

#define EFREAD		  73	// An error occured when reading a FILE*
#define EFOPEN 44 // Failed to open a FILE*


int testFile(char* path, int tests);
long getBinaryFileSize(FILE *f);
void handleFileOpenError();
void handleFileReadError();

#endif /* UTILITY_FILESYSTEM_H_ */
