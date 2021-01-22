#ifndef _E2T_H_
#define _E2T_H_

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>

extern FILE* yyin;
extern FILE* yyout;
extern FILE* checkfile;
extern int yylineno;

extern struct TreeNode* root;
extern std :: map <std :: string, std :: string> g_env;
extern std :: vector <std :: string> g_env_array;

int yylex();
void yyerror(char*);
int toint(char*);
std :: string itos(int i);
int first(struct TreeNode*, bool, int);

struct TreeNode {
	int type;
	int val;
	std :: string name, callid, op;
	std :: vector <struct TreeNode*> child;
	int width, offset;
	std :: string code;
	std :: map <std :: string, int> env;
	TreeNode(int T) {type = T; child.clear(); val = 0; name = ""; callid = ""; op = ""; code = "";
				width = offset = 0; }
};

#define NODE_GOALLIST 		1000
#define NODE_VARDEFN_1		1001
#define NODE_VARDEFN_2		1002
#define NODE_FUNCDEFN		1003
#define NODE_STMTLIST		1004
#define NODE_EXPR_1		1005
#define NODE_EXPR_2		1006
#define NODE_EXPR_3		1007
#define NODE_EXPR_4		1008
#define NODE_EXPR_5		1009
#define NODE_EXPR_6		1010
#define NODE_EXPR_7		1011
#define NODE_EXPR_8		1012
#define NODE_EXPR_9		1013
#define NODE_EXPR_10		1014
#define NODE_EXPR_11		1015
#define NODE_EXPR_12		1016
#define NODE_EXPR_13		1017
#define NODE_EXPR_14		1018
#define NODE_EXPR_15		1019
#define NODE_RVAL_1		1020
#define NODE_RVAL_2		1021

#endif
