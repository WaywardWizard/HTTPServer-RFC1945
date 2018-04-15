/*
 * byteString.c
 *
 *  Created on: 12 Apr. 2018
 *      Author: ben
 */
#include "byteString.h"
#include <string.h>
#include <stdlib.h>


byteString_t *bsInit() {
	byteString_t *b=malloc(sizeof(byteString_t));
	b->string=NULL;
	b->length=0;
	return(b);
}


void bsWrite(byteString_t *b, void* byteChain, int length) {
	free(b->string);
	b->string=malloc(length);
	bcopy(byteChain, b->string, length);
	b->length=length;
}


void bsAppend(byteString_t *b, void* byteChain, int length) {
	if(b==NULL) {return;}
	int newLength=length+b->length;
	b->string=realloc(b->string,newLength);
	bcopy(byteChain, b->string, length);
	b->length=newLength;
}


void bsFree(byteString_t *b) {
	if(b!=NULL){
		free(b->string);
	}
}


byteString_t *bsCopy(byteString_t *b) {
	byteString_t *b2=bsInit();
	bsWrite(b2, b->string, b->length);
	return(b2);
}


void bsShrink(byteString_t *b, int size) {
	if(b==NULL || size>(b->length)) {
		return;
	}
	b->string=realloc(b->string, size);
	b->length=size;
}


void bsDestruct(void* b) {
	byteString_t* bs=b;
	bsFree(bs);
}
