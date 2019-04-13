#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


#define MAX_STR 100
#define retifnull(e1, e2) { \
	if (e1 == NULL && e2 == NULL) return 0; \
	if (e1 == NULL) return -1; \
	if (e2 == NULL) return 1; \
} 


//linked list element
typedef struct Element {
	int val;
	struct Element *next;
} *Element;

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
void initializeAlphabet();
Node initializeNodeInfo(Node n);
int nullable(Node n);
Element firstpos(Node n);
Element lastpos(Node n);

/*linked list funcs*/
void insertInList(Element start, int val);
void removeFromList(Element *start, int val);
int compareList(Element el1, Element el2);
int getLengthList(Element el);
Element copyList(Element el);
Element createElement(int val);
Element unionList(Element el1, Element el2);
void displayList(Element el, FILE *stream);

/*miscellaneous*/
void readline(char* in);


static char input[MAX_STR];
static char lookahead;
static int ind;
static char *alphabet;

/*
 *	input any regex using * () | operators and string to match
 */
int main()
{
	element list1 = malloc(sizeof(struct element));
	list1->next = null;
	list1->val = 0;
	list1->next = createelement(2);
	
	element list2 = malloc(sizeof(struct element));
	list2->next = null;
	list2->val = 0;
	list2->next = createelement(2);
	printf("comparelist(l1, l2) = %d\n", comparelist(list1, list2));

	
	
	
	readline(input);
	lookahead = input[ind];
	initializeAlphabet();
	printf("Alphabet: %s\n", alphabet);
	lookahead = input[ind];
	
	Node r = R();
	printf("Syntax tree:\n");
	initializeNodeInfo(r);
	preorderTraverse(r, 0);
	return 0;
}

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
	static int position = 1;	
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
	displayList(node->firstpos, stdout);
	printf(" lastpos = ");
	displayList(node->lastpos, stdout);
	printf("}\n");
	if (node->left != NULL)
		preorderTraverse(node->left, depth + 1);
	if (node->right != NULL)
		preorderTraverse(node->right, depth + 1);

}

void omitc(char c) {
	if (lookahead == c) {
		lookahead = input[++ind];
	}
}

void omit() {
	if (lookahead != '\0') {
		lookahead = input[++ind]; }
}

/*functions for lexical analyzer*/
void initializeAlphabet() {
	char buf[MAX_STR];
	char *b = buf;
	char *c = input;
	int num = 0;
	while (*c != '\0') {
		if (isalpha(*c)) { //TODO: extend to allow numbers
			*b++ = *c; //don't care about buffer overflow SO intruder will easily break our system
			num++;
		}
		c++;
	}
	alphabet = malloc(sizeof(char) * (num + 1));
	c = alphabet;
	b = buf;

	while (num--) {
		*c++ = *b++;
	}
	*c = '\0';
}

Node initializeNodeInfo(Node n) {
	if (n->left != NULL) {
		initializeNodeInfo(n->left);
	}
	if (n->right != NULL) {
		initializeNodeInfo(n->right);
	}
		

	n->isNullable = nullable(n);
	n->firstpos = firstpos(n);
	n->lastpos = lastpos(n);
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
		return unionList(copyList(n->left->firstpos), copyList(n->right->firstpos));
	case '&':
		if (n->left->isNullable)
			return unionList(copyList(n->left->firstpos), copyList(n->right->firstpos));
		else
			return n->left->firstpos;
	default:
		if (isalpha(n->val))
			return createElement(n->position);
		else
			return NULL;
	}
}

Element lastpos(Node n) 
{
	if (n->val != '|' && n->val != '&')
		return firstpos(n);
	Node left = n->left;
	n->left = n->right;
	n->right = left;
	return firstpos(n);
}

/*linked list funcs*/
void insertInList(Element el, int val) {
	while (el->next != NULL)
		el = el->next;
	Element newel = malloc(sizeof(struct Element));
	newel->val = val;
	newel->next = NULL;
	el->next = newel;
}

void removeFromList(Element *start, int val) {
	Element *indirect = start;
	
	while ((*indirect)->val != val)
	{ 
		if ((*indirect)->next != NULL)
			indirect = &(*indirect)->next;
		else
			return;
	}

	*indirect = (*indirect)->next;
}

int compareList(Element el1, Element el2) {
	retifnull(el1, el2);

	int temp;
	if (getLengthList(el1) != getLengthList(el2))
		return getLengthList(el1) - getLengthList(el2);
	while(1) {
		if ((temp = el1->val - el2->val) != 0) return temp;
		el1 = el1->next;
		el2 = el2->next;
		retifnull(el1, el2);
	}
}

int getLengthList(Element el) {
	if (el == NULL) return 0;
	int length = 1;
	while (el->next != NULL) {
		length++;
		el = el->next;
	}
	return length;
}

Element copyList(Element el) 
{
	Element res = createElement(el->val);
	Element curRes = res;
	Element curArg = el;
	while (curArg->next != NULL) {
		curArg = curArg->next;
		curRes->next = createElement(curArg->val);
		curRes = curRes->next;
	}
	return res;
}

Element createElement(int val) {
	Element res = malloc(sizeof(struct Element));
	res->val = val;
	res->next = NULL;
	return res;
}

Element unionList(Element el1, Element el2) {
	Element cur = el1;
	while (cur->next != NULL)
		cur = el1->next;
	cur->next = el2;
	return el1; //fixed later
}

void displayList(Element el, FILE *stream) {
	char buf[MAX_STR];
	putc('{', stream);
	while (1) {
		fputs(itoa(el->val, buf, 10), stream);
		el = el->next;
		if (el != NULL) putc(',', stream);
		else break;
	}
	putc('}', stream);
}

