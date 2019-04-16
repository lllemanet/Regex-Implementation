#include <stdio.h>
#include <stdlib.h>	
#include "linkedlist.h"

#define MAX_STR_LENGTH 100

void* getElement(Element list, void* val, int(*compare)(void*, void*)) {
	while (list != NULL) {
		if (compare(list->val, val) == 0)
			return list->val;
		list = list->next;
	}
	return NULL;
}

void insertInList(Element* el, void *val) {
	if (*el == NULL)
		*el = createElement(val);
	Element cur = *el;
	while (cur->next != NULL)
		cur = cur->next;
	Element newel = malloc(sizeof(struct Element));
	newel->val = val;
	newel->next = NULL;
	cur->next = newel;
}

/*TODO Should return the Element to be able to be freed*/
void removeFromList(Element *start, void *val, int(*compare)(void *el1, void *el2)) {
	Element *indirect = start;

	while (compare((*indirect)->val, val) != 0)
	{
		if ((*indirect)->next != NULL)
			indirect = &(*indirect)->next;
		else
			return;
	}

	free((*indirect)->val);
	*indirect = (*indirect)->next;
}

int compareList(Element el1, Element el2, int(*compare)(void *el1, void *el2)) {
	retifnull(el1, el2);

	int temp;
	if (getLengthList(el1) != getLengthList(el2))
		return getLengthList(el1) - getLengthList(el2);
	while (1) {
		if ((temp = compare(el1->val, el2->val)) != 0) return temp;
		el1 = el1->next;
		el2 = el2->next;
		retifnull(el1, el2);
	}
}

int getLengthList(Element el) {
	int length = 0;
	while (el != NULL) {
		length++;
		el = el->next;
	}
	return length;
}

Element copyList(Element el, void* (*getCopy)(void*))
{
	Element res = createElement(getCopy(el->val));
	Element curRes = res;
	Element curArg = el;
	while (curArg->next != NULL) {
		curArg = curArg->next;
		curRes->next = createElement(getCopy(curArg->val));
		curRes = curRes->next;
	}
	return res;
}

Element createElement(void *val) {
	Element res = malloc(sizeof(struct Element));
	res->val = val;
	res->next = NULL;
	return res;
}

/*TODO el1 can be NULL. error-correcting code? not now*/
Element unionList(Element el1, Element el2, int(*compare)(void *el1, void *el2)) {
	if (el1 == NULL) return el2;
	Element cur1, end = el1;
	Element cur2 = el2;
	while (end->next != NULL)
		end = end->next;
	int isRestart = 0;
	while (cur2 != NULL) {
		cur1 = el1;
		while (cur1 != NULL) {
			if (compare(cur1->val, cur2->val) == 0) {
				removeFromList(&el2, cur2->val, compare);
				isRestart = 1;
				break;
			}
			cur1 = cur1->next;
		}
		if (isRestart) {
			cur2 = el2;
			isRestart = 0;
		}
		else
			cur2 = cur2->next;
	}
	end->next = el2;
	return el1;
}

void displayList(Element el, FILE* stream, char* (*toString)(void*)) {
	if (el == NULL) {
		fputs("[]", stream);
		return;
	}
	char buf[MAX_STR_LENGTH];
	putc('[', stream);
	while (1) {
		fputs(toString(el->val), stream);
		el = el->next;
		if (el != NULL) putc(',', stream);
		else break;
	}
	putc(']', stream);
}