/*
 * filesystem.h
 *
 *  Created on: 13 Apr. 2018
 *      Author: ben
 */

#ifndef UTILITY_FILESYSTEM_H_
#define UTILITY_FILESYSTEM_H_

#include <unistd.h> //testFile() arguments
#include "bool.h" //testFile() return values

#define EFREAD		  73	// An error occured when reading a FILE*
#define EFOPEN 44 // Failed to open a FILE*


int testFile(char* path, int tests);
void handleFileOpenError();
void handleFileReadError();

#endif /* UTILITY_FILESYSTEM_H_ */
