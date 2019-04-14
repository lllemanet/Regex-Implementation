#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


#define MAX_STR_LENGTH 100

#define retifnull(el0x1, el0x2) { \
	if (el0x1 == NULL && el0x2 == NULL) return 0; \
	if (el0x1 == NULL) return -1; \
	if (el0x2 == NULL) return 1; \
}

#define deref(type, voidEl) *(type*)(voidEl)

//weird name to prevent symbol conflicts
#define makeptr(type, val, dest) { \
	type* ptr0x14F24 = malloc(sizeof(type)); \
	*ptr0x14F24 = val; \
	dest = ptr0x14F24; \
}

//general linked list
typedef struct Element {
	void *val;
	struct Element *next;
} *Element;

typedef struct DictNode {
	void *key;
	void *val;
} *DictNode;

typedef struct Node {
	char val;
	struct Node *left;
	struct Node *right;
	Element firstpos;
	Element lastpos;
	Element followpos;
	int isNullable;
	int position;
} *Node; //refactor to be just Node

typedef struct Dtran{
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
Node R();
Node R1(Node node);
Node T();
Node T1(Node node);
Node F();
Node F1(Node node);
Node P();
void omitc(char c);
void omit();
Node createNode(char val, Node left, Node right);
void preorderTraverse(Node node, int depth);

/*functions for lexical analyzer*/
void initAlphabet();
Element initNodeInfo(Node n, Element followpositions);
int nullable(Node n);
Element firstpos(Node n);
Element lastpos(Node n);

/*linked list funcs*/
void insertInList(Element* el, void *val);
void removeFromList(Element *start, void *val, int(*compare)(void *el1, void *el2));
int compareList(Element el1, Element el2, int(*compare)(void *el1, void *el2));
int getLengthList(Element el);
Element copyList(Element el, void* (*getCopy)(void*));
Element createElement(void *val);
Element unionList(Element el1, Element el2, int (*compare)(void *el1, void *el2));
void displayList(Element el, FILE *stream, char* (*toString)(void*));

/*functions for general int list*/
int compareVoidInts(void *el1, void *el2);
char* toStringVoidInt(void *val);
//be careful with made copies
void* makeVoidInt(int val); //malloc
void* getCopyVoidInt(void *val); //malloc

/*dictionary funcs*/
void* getDictVal(Element dict, void *key, int(*compare)(void *el1, void *el2));
void insertInDict(Element dict, void* key, void* elem);
void displayDictionaryWithIntAndElement(Element dict, FILE *stream, char* (*toString)(void* el));
DictNode createDictNode(void* key, void* val);

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


static char regex[MAX_STR_LENGTH];
static char lookahead;
static int ind;
static char* alphabet;
static char* symbols;
static int position = 1;


/*
 *	input any regex using * () | operators and string to match
 */
int main()
{
	printf("Input regular expression:\n");
	readline(regex);
	printf("Input string you want to match:\n");
	//readline()
	initAlphabet();
	lookahead = regex[ind];
	printf("Alphabet: %s\n", alphabet);
	printf("Symbols: %s\n", symbols);

	Node r = R();
	DictNode dictNode = createDictNode(makeVoidInt(-1), NULL);
	Element followpos = createElement(dictNode);
	Element firstPos = initNodeInfo(r, followpos);

	/*TEST REGION*/
	
	/*Element pos = createElement(makeVoidInt(1));
	pos->next = createElement(makeVoidInt(5));
	State st = createState(2, pos, getDtranArr(alphabet), 0);
	Element res = getPosForDtranState('h', pos, symbols, followpos);
	displayList(res, stdout, toStringVoidInt);
	printf("\n%d\n\n", getStateWithPos(createElement(st), pos));*/

	/*END OF TEST REGION*/

	Element states = initStates(firstPos, followpos);
	Element tr = states;
	while (tr != NULL) {
		displayState(tr->val);
		tr = tr->next;
	}
	
	printf("Syntax tree:\n");
	preorderTraverse(r, 0);
	displayDictionaryWithIntAndElement(followpos, stdout, toStringVoidInt);

	return 0;
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

Node R() {
	if (lookahead == '(' || isalpha(lookahead)) {
		//derive R -> T R1
		Node t = T();
		Node r1 = R1(t);
		return r1;
	}
	else {
		omit(); //error
		//return NULL;
	}
}

Node R1(Node left) {
	if (lookahead == '|') {
		omit();
		Node t = T();
		left = createNode('|', left, t);
		Node r1 = R1(left);
		return r1;
	}
	else if (lookahead == '\0' || lookahead == ')') {
		return left;
	}
	else {
		omit(); //error and there should be another stmt 'return NULL;' but I am busy
	}
}

Node T() {
	if (lookahead == '(' || isalpha(lookahead)) {
		Node f = F();
		Node t1 = T1(f);
		return t1;
	}
	else
		omit(); //error
}

Node T1(Node left) {
	if (lookahead == '(' || isalpha(lookahead)) {
		Node f = F();
		left = createNode('&', left, f);
		Node t1 = T1(left);
		return t1;
	}
	else if (lookahead == '|' || lookahead == ')' || lookahead == '\0') {
		return left;
	}
	else
		omit(); //error
}

Node F() {
	if (lookahead == '(' || isalpha(lookahead)) {
		Node p = P();
		Node f1 = F1(p);
		return f1;
	}
	else
		omit(); //error
}

Node F1(Node left) {
	if (lookahead == '*') {
		omitc('*');
		return createNode('*', left, NULL);
	}
	else if (isalpha(lookahead) || lookahead == '(' || lookahead == ')' || lookahead == '\0' || lookahead == '|')
		return left;
	else
		omit(); //error
}

Node P() {
	if (lookahead == '(') {
		omitc('(');
		Node r = R();
		omitc(')');
		return r;
	}
	else if (isalpha(lookahead)) {
		char l = lookahead;
		omitc(lookahead);
		Node n = createNode(l, NULL, NULL);
		n->position = position++;
		return n;
	}
	else
		omit(); //error
}

Node createNode(char val, Node left, Node right) {
	Node node = malloc(sizeof(struct Node));
	node->val = val;
	node->left = left;
	node->right = right;
	node->isNullable = 1;
	node->firstpos = NULL;
	node->lastpos = NULL;
	node->position = 0;
	return node;
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

void omitc(char c) {
	if (lookahead == c) {
		lookahead = regex[++ind];
	}
}

void omit() {
	if (lookahead != '\0') {
		lookahead = regex[++ind]; }
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
			(n->val == '*') ||
			(n->val == '|' && (n->left->isNullable || n->right->isNullable)) ||
			(n->val == '&' && (n->left->isNullable && n->right->isNullable))
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

int compareList(Element el1, Element el2, int (*compare)(void *el1, void *el2)) {
	retifnull(el1, el2);

	int temp;
	if (getLengthList(el1) != getLengthList(el2))
		return getLengthList(el1) - getLengthList(el2);
	while(1) {
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
Element unionList(Element el1, Element el2, int (*compare)(void *el1, void *el2)) {
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

void displayList(Element el, FILE *stream, char* (*toString)(void*)) {
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

/*dictionary funcs*/
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