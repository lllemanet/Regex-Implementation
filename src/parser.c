#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "parser.h"

static char lookahead = 0;
static char* regex = NULL;
static int position = 1;
static int ind = 0;

//it's really just stub
Node R(char* reg) {
	regex = reg;
	lookahead = *regex;
	return R_real();
	position = 1;
	ind = 0;
}

static Node R_real() {
	if (lookahead == '(' || isalpha(lookahead)) {
		Node t = T();
		Node r1 = R1(t);
		return r1;
	}
	else {
		omit(); //error
	}
}

static Node R1(Node left) {
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
		Node r = R_real();
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

void omitc(char c) {
	if (lookahead == c) {
		lookahead = regex[++ind];
	}
}

void omit() {
	if (lookahead != '\0')
		lookahead = regex[++ind];
}
