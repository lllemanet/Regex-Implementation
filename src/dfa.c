#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "linkedlist.h"
#include "dict.h"
#include "parser.h"


#define MAX_STR_LENGTH 100



typedef struct Dtran {
	char sym;
	int state;
} Dtran;

typedef struct State {
	int id;
	Element pos;
	Dtran *dtrans;
	int isMarked;
} *State;

/*functions for parsing*/


/*functions for lexical analyzer*/
void initAlphabet();
Element initNodeInfo(Node n, Element followpositions);
int nullable(Node n);
Element firstpos(Node n);
Element lastpos(Node n);

/*functions for general int list*/
int compareVoidInts(void *el1, void *el2);
char* toStringVoidInt(void *val);
//be careful with made copies
void* makeVoidInt(int val); //malloc
void* getCopyVoidInt(void *val); //malloc

/*State funcs*/
Dtran* getDtranArr(char* str);
Element initStates(Element firstPos, Element followpos);
State createState(int id, Element pos, Dtran* dtrans, int isMarked);
Element getPosForDtranState(char sym, Element pos, char* str, Element followpos);
int getStateWithPos(Element states, Element pos);

void displayState(State st);
void displayDtrans(Dtran* dtrans, int size, FILE* stream);


/*miscellaneous*/
void readline(char* in);
void addForEachInList(Element dict, Element to, Element from);
char* strrepeatsoff(char* src);
int compareStateId(void* state, void* id);
void preorderTraverse(Node node, int depth);

/*match*/
int match(Element states, char* input);

static char input[MAX_STR_LENGTH];
char regex[MAX_STR_LENGTH];
static char* alphabet;
static char* symbols;


/*
 *	input any regex using * () | operators and string to match
 */
int main()
{
	printf("Input regular expression:\n");
	readline(regex);
	initAlphabet();
	printf("Alphabet: %s\n", alphabet);
	printf("Symbols: %s\n", symbols);

	Node r = R(regex);
	DictNode dictNode = createDictNode(makeVoidInt(-1), NULL);
	Element followpos = createElement(dictNode);
	Element firstPos = initNodeInfo(r, followpos);

	Element states = initStates(firstPos, followpos);
	Element tr = states;
	printf("States:\n");
	while (tr != NULL) {
		displayState(tr->val);
		tr = tr->next;
	}

	printf("Syntax tree:\n");
	preorderTraverse(r, 0);
	printf("Followpos:\n");
	displayDictionaryWithIntAndElement(followpos->next, stdout, toStringVoidInt);

	while (1) {
		printf("Input string you want to match (q to quit):\n");
		readline(input);
		if (*input == 'q')
			return 200;
		printf("%s\n", match(states, input) ? "Yes" : "No");
	}

	return 0;
}

int match(Element states, char* input) {
	State state = (State)states->val;
	int isInAlphabet;
	while (1) {
		isInAlphabet = 0;
		Dtran* dtrans = state->dtrans;
		for (int i = 0; i < strlen(alphabet); i++) {
			if (dtrans[i].sym == *input) {
				isInAlphabet = 1;
				if (dtrans[i].state == -1) {
					return 0;
				}
				else {
					state = (State)getElement(states, makeVoidInt(dtrans[i].state), compareStateId);
					break;
				}
			}
		}
		if (!isInAlphabet)
			return *input == '\0' && getElement(state->pos, makeVoidInt(0), compareVoidInts) != NULL;
		input++;
	}
}

Element initStates(Element firstPos, Element followpos) {
	int idcnt = 1;
	State startState = createState(idcnt++, firstPos, NULL, 0);
	Element states = createElement(startState);

	int isMarked = 0;
	while (isMarked == 0) {
		isMarked = 1;
		Element curel = states;
		while (curel != NULL) {
			State curstate = (State)curel->val;
			if (!curstate->isMarked) {
				isMarked = 0;
				Dtran* dtrans = getDtranArr(alphabet);
				for (int i = 0; i < strlen(alphabet); i++) {
					Element posForDtranState = getPosForDtranState(dtrans[i].sym, curstate->pos, symbols, followpos);
					if (posForDtranState == NULL) {
						dtrans[i].state = -1;
						continue;
					}

					int id = getStateWithPos(states, posForDtranState);
					if (id != -1)
						dtrans[i].state = id;
					else {
						dtrans[i].state = idcnt;
						State newState = createState(idcnt++, posForDtranState, NULL, 0);
						insertInList(&states, newState);
					}
				}
				curstate->dtrans = dtrans;
				curstate->isMarked = 1;
			}
			curel = curel->next;
		}
	}
	return states;
}

int getStateWithPos(Element states, Element pos) {
	while (states != NULL) {
		State state = (State)states->val;
		if (compareList(state->pos, pos, compareVoidInts) == 0)
			return state->id;
		states = states->next;
	}
	return -1;
}

Element getPosForDtranState(char sym, Element pos, char* str, Element followpos) {
	Element posForDtranState = NULL;
	while (pos != NULL) {
		int ipos = deref(int, pos->val);
		if (str[ipos - 1] == sym) {
			Element followforpos = *((Element *)getDictVal(followpos, pos->val, compareVoidInts));
			posForDtranState = unionList(posForDtranState, copyList(followforpos, getCopyVoidInt), compareVoidInts);
		}
		pos = pos->next;
	}
	return posForDtranState;
}

Dtran* getDtranArr(char* str) {
	Dtran* dtran = malloc(sizeof(struct Dtran) * strlen(str));
	Dtran* dtrancur = dtran;
	while (*str != '\0') {
		dtrancur->sym = *str++;
		dtrancur->state = -1;
		dtrancur++;
	}
	return dtran;
}

State createState(int id, Element pos, Dtran* dtrans, int isMarked) {
	State state = malloc(sizeof(struct State));
	state->id = id;
	state->pos = pos;
	state->dtrans = dtrans;
	state->isMarked = isMarked;
	return state;
}

void displayState(State st) {
	printf("#%d: pos=", st->id);
	displayList(st->pos, stdout, toStringVoidInt);
	displayDtrans(st->dtrans, strlen(alphabet), stdout);
	printf("\n");
}

void displayDtrans(Dtran* dtrans, int size, FILE* stream) {
	if (dtrans == NULL) {
		fprintf(stream, "[]");
		return;
	}

	fprintf(stream, "[");
	for (int i = 0; i < size; i++)
		fprintf(stream, "%c->%d%s", dtrans[i].sym, dtrans[i].state, (i == size - 1) ? "" : "; ");
	fprintf(stream, "]");
}

/*miscellaneous*/
void readline(char* in)
{
	while ((*in++ = getchar()) != '\n'); //easy place to hack
	*--in = '\0';
}


/*functions for lexical analyzer*/
void initAlphabet() {
	char buf[MAX_STR_LENGTH];
	char *b = buf;
	char *c = regex;
	int num = 0;
	while (*c != '\0') {
		if (isalpha(*c)) { //TODO: extend to allow numbers
			*b++ = *c; //don't care about buffer overflow SO intruder will easily break our system
			num++;
		}
		c++;
	}
	symbols = malloc(sizeof(char) * (num + 1));
	c = symbols;
	b = buf;

	while (num--) {
		*c++ = *b++;
	}
	*c = '\0';
	alphabet = strrepeatsoff(symbols);
}

Element initNodeInfo(Node n, Element followpositions) {
	static int check = 1;
	int isRoot = --check == 0 ? 1 : 0;
	if (n->left != NULL)
		initNodeInfo(n->left, followpositions);
	if (n->right != NULL)
		initNodeInfo(n->right, followpositions);

	if (n->val == '&')
		addForEachInList(followpositions, n->left->lastpos, n->right->firstpos);
	if (n->val == '*')
		addForEachInList(followpositions, n->left->lastpos, n->left->firstpos);

	n->isNullable = nullable(n);
	n->firstpos = firstpos(n);
	n->lastpos = lastpos(n);
	if (isRoot) {
		addForEachInList(followpositions, n->lastpos, createElement(makeVoidInt(0)));
		if (n->isNullable)
			return unionList(n->firstpos, createElement(makeVoidInt(0)), compareVoidInts);
		return n->firstpos;
	}
}

void addForEachInList(Element dict, Element to, Element from) {
	while (to != NULL) {
		Element *val = (Element *)getDictVal(dict, to->val, compareVoidInts);
		if (val == NULL)
			insertInDict(dict, makeVoidInt(deref(int, to->val)), copyList(from, getCopyVoidInt));
		else {
			*val = unionList(*val, copyList(from, getCopyVoidInt), compareVoidInts); //makes some memory leak
		}
		to = to->next;
	}
}

int nullable(Node n) {
	return n->position == 0 &&
		(
			(n->val == '*')
			|| (n->val == '|' && (n->left->isNullable || n->right->isNullable))
			|| (n->val == '&' && (n->left->isNullable && n->right->isNullable))
		);
}

Element firstpos(Node n)
{
	switch (n->val) {
	case '*':
		return n->left->firstpos;
	case '|':
		return unionList(copyList(n->left->firstpos, getCopyVoidInt), copyList(n->right->firstpos, getCopyVoidInt),
			compareVoidInts);
	case '&':
		if (n->left->isNullable)
			return unionList(copyList(n->left->firstpos, getCopyVoidInt), copyList(n->right->firstpos,
				getCopyVoidInt), compareVoidInts);
		else
			return n->left->firstpos;
	default:
		if (isalpha(n->val)) {
			void *par;
			makeptr(int, n->position, par);
			return createElement(par);
		}
		else
			return NULL;
	}
}

Element lastpos(Node n)
{
	switch (n->val) {
	case '*':
		return n->left->lastpos;
	case '|':
		return unionList(copyList(n->left->lastpos, getCopyVoidInt), copyList(n->right->lastpos, getCopyVoidInt),
			compareVoidInts);
	case '&':
		if (n->right->isNullable)
			return unionList(copyList(n->left->lastpos, getCopyVoidInt), copyList(n->right->lastpos,
				getCopyVoidInt), compareVoidInts);
		else
			return n->right->lastpos;
	default:
		if (isalpha(n->val)) {
			void *par;
			makeptr(int, n->position, par);
			return createElement(par);
		}
		else
			return NULL;
	}
}

/*linked list funcs*/

/*dictionary funcs*/


/*int methods*/
int compareVoidInts(void *el1, void *el2) {
	return deref(int, el1) - deref(int, el2);
}

void *getCopyVoidInt(void *val) {
	void *ptr;
	makeptr(int, deref(int, val), ptr);
	return ptr;
}

char* toStringVoidInt(void *val) {
	static char buf[MAX_STR_LENGTH];
	itoa(deref(int, val), buf, 10);
	return buf;
}

void* makeVoidInt(int val) {
	void *var;
	makeptr(int, val, var);
	return var;
}

char* strrepeatsoff(char* src) {
	if (*src == '\0')
		return "\0";
	char buf[MAX_STR_LENGTH];
	char* bufcur = buf;
	char* srccur = src; //check to add

	while (1) {
		char* bufch = buf; //check if appeared in buf
		while (bufch != bufcur) {
			if (*bufch == *srccur) {
				srccur++; //since if srctr == '\0' it will not be equal to any bufch before buf
				bufch = buf - 1;
			}
			bufch++;
		}
		*bufcur = *srccur;
		bufcur++;
		if (*srccur == '\0') break;
		srccur++;
	}

	char *res = malloc(sizeof(char) * (bufcur - buf));
	memcpy(res, buf, bufcur - buf);
	return res;
}

int compareStateId(void* state, void* id) {
	return ((State)state)->id - deref(int, id);
}

void preorderTraverse(Node node, int depth) {
	int i = depth;
	while (i-- > 0)
		putchar(' ');
	putchar(node->val);
	printf(" firstpos = ");
	displayList(node->firstpos, stdout, toStringVoidInt);
	printf(" lastpos = ");
	displayList(node->lastpos, stdout, toStringVoidInt);
	printf("\n");
	if (node->left != NULL)
		preorderTraverse(node->left, depth + 1);
	if (node->right != NULL)
		preorderTraverse(node->right, depth + 1);

}