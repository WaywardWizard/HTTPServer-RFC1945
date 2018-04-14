/*
 * byteString.h
 *
 *  Created on: 12 Apr. 2018
 *      Author: ben
 */

#ifndef UTILITY_BYTESTRING_H_
#define UTILITY_BYTESTRING_H_

struct byteString {
	char* string;
	int length;
};

typedef struct byteString byteString_t;

void bsWrite(byteString_t *b, void* byteChain, int length);
void bsFree(byteString_t *b);
byteString_t *bsCopy(byteString_t *b);
byteString_t *bsInit();
void bsShrink(byteString_t *b, int size);
void bsAppend(byteString_t *b, void* byteChain, int length);

#endif /* UTILITY_BYTESTRING_H_ */
