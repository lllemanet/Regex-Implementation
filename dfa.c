#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


#define MAX_STR 100


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
Node initializeInfoNode(Node n);
int nullable(Node n);
Element firstpos(Node n);
Element lastpos(Node n);

/*linked list funcs*/
void insertInList(Element start, int val);
void removeFromList(Element *start, int val);
void copyList(Element el);

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
	Element list = malloc(sizeof(struct Element));
	list->val = 0;
	insertInList(list, 1);
	printf("prev: %d; next: %d\n", list->val, list->next->val);
	removeFromList(&list, 1);
	printf("prev: %d; next null? %d\n", list->val, list->next == NULL);
	insertInList(list, 1);
	removeFromList(&list, 0);
	printf("Afteer remove first element: start: %d;" list->val);
	
	
	readline(input);
	lookahead = input[ind];
	initializeAlphabet();
	printf("Alphabet: %s\n", alphabet);
	lookahead = input[ind];
	
	Node r = R();
	//printf("%c\n", r->right->val);
	preorderTraverse(r, 1);
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
	return node;
}

void preorderTraverse(Node node, int depth) {
	int i = depth;
	while (i-- > 0)
		putchar(' ');
	putchar(node->val);
	putchar('\n');
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

Node initializeInfoNode(Node n) {
	if (n->left != NULL)
		initializeInfoNode(n->left);
	if (n->right != NULL)
		initializeInfoNode(n->right);

	n->isNullable = nullable(n);
	n->firstpos = firstpos(n);
	n->lastpos = lastpos(n);
}

int nullable(Node n) {
	return n->position == 0 &&
		(
			(n->val == '*') ||
			(n->val == '|' && (n->left->isNullable || n->right->isNullable)) ||
			(n->val = '&' && (n->left->isNullable && n->right->isNullable))
		);
}

Element firstpos(Node n) {}
Element lastpos(Node n) {}

/*linked list funcs*/
void insertInList(Element el, int val) {
	while (el->next != NULL)
		el = el->next;
	Element newEl = malloc(sizeof(struct Element));
	newEl->val = val;
	el->next = newEl;
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

	*indirect = (*indirect)->next; //
}
void copyList(Element el) {}
