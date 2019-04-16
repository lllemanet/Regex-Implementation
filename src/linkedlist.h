#include <stdio.h>

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#define deref(type, voidEl) *(type*)(voidEl)

//weird name to prevent symbol conflicts
#define makeptr(type, val, dest) { \
	type* ptr0x14F24 = malloc(sizeof(type)); \
	*ptr0x14F24 = val; \
	dest = ptr0x14F24; \
}

#define retifnull(el0x1, el0x2) { \
	if (el0x1 == NULL && el0x2 == NULL) return 0; \
	if (el0x1 == NULL) return -1; \
	if (el0x2 == NULL) return 1; \
}


// general linked list
typedef struct Element {
	void *val;
	struct Element *next;
} *Element;

/*linked list funcs*/
void* getElement(Element list, void* val, int(*compare)(void*, void*));
void insertInList(Element* el, void *val);
void removeFromList(Element *start, void *val, int(*compare)(void *el1, void *el2));
int compareList(Element el1, Element el2, int(*compare)(void *el1, void *el2));
int getLengthList(Element el);
Element copyList(Element el, void* (*getCopy)(void*));
Element createElement(void *val);
Element unionList(Element el1, Element el2, int(*compare)(void *el1, void *el2));
void displayList(Element el, FILE *stream, char* (*toString)(void*));

#endif