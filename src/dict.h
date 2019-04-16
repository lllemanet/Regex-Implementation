#ifndef DICT_H
#define DICT_H

#include <stdio.h>
#include "linkedlist.h"

typedef struct DictNode {
	void *key;
	void *val;
} *DictNode;

void* getDictVal(Element dict, void *key, int(*compare)(void *el1, void *el2));
void insertInDict(Element dict, void* key, void* elem);
void displayDictionaryWithIntAndElement(Element dict, FILE *stream, char* (*toString)(void* el));
DictNode createDictNode(void* key, void* val);
#endif