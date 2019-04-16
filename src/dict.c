#include <stdlib.h>
#include "dict.h"

void* getDictVal(Element dict, void *key, int(*compare)(void *el1, void *el2)) {
	while (dict != NULL) {
		DictNode n = (DictNode)dict->val;
		if (compare(key, n->key) == 0)
			return (&(n->val));
		dict = dict->next;
	}
	return NULL;
}

void insertInDict(Element dict, void* key, void* val) {
	insertInList(&dict, createDictNode(key, val));
}

void displayDictionaryWithIntAndElement(Element dict, FILE *stream, char* (*toString)(void* el)) {
	while (dict != NULL) {
		DictNode d = (DictNode)dict->val;
		fprintf(stream, "key = %d; val = ", deref(int, d->key)); //TODO not generic key
		displayList(d->val, stream, toString); //TODO not generic val display 
		printf("\n");
		dict = dict->next;
	}
}

DictNode createDictNode(void* key, void* val) {
	DictNode node = malloc(sizeof(struct DictNode));
	node->key = key;
	node->val = val;
	return node;
}