/*
 * regexTool.h
 *
 *  Created on: 13 Apr. 2018
 *      Author: ben
 */

#ifndef UTILITY_REGEXTOOL_H_
#define UTILITY_REGEXTOOL_H_

char* extractMatch(char* regex, char* searchString, char** destination);
int isMatch(char* regex, char* searchString);

#endif /* UTILITY_REGEXTOOL_H_ */
