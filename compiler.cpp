#include "e2t.h"

std :: map <std :: string, int> l_env; //local environment
int param_cnt = 0;

std :: string itos(int n) {return std :: to_string(n);}
std :: string load(int n, std :: string s) {return "load " + itos(n) + " " + s + "\n";}
std :: string load(std :: string s1, std :: string s2) {return "load " + s1 + " " + s2 + "\n";}
std :: string store(int n, std :: string s) {return "store " + s + " " + itos(n) + "\n";}
std :: string assign(std :: string s1, std :: string s2) {return s2 + " = " + s1 + "\n";}
std :: string assign(int n, std :: string s) {return s + " = " + itos(n) + "\n";}
std :: string loadaddr(int n, std :: string s2) {return "loadaddr " + itos(n) + " " + s2 + "\n";}
std :: string loadaddr(std :: string s1, std :: string s2) {return "loadaddr "+ s1 + " " + s2 + "\n";}
std :: string gstore(std :: string s1, std :: string s2, std :: string index) {
	if (index == "")
		return loadaddr(s1, "s0") + ("s0 [0] = " + s2 + "\n");
	else
		return load(s1, "s0") + ("s0 = s0 + " + index + "\n") + ("s0 [0] = " + s2 + "\n");
		//return loadaddr(s1, "s0") + (index == "" ? "" : "s0 = s0 + " + index + "\n") + ("s0 [0] = " + s2 + "\n");
}
std :: string load_rvaltoreg(struct TreeNode* root, std :: string reg) {
	std :: string ret = "";
	if (root->type == NODE_RVAL_1) {
		if (l_env.find(root->name) != l_env.end()) {
			ret += load(l_env[root->name], reg);
		}else{
			ret += load(g_env[root->name], reg);
		}
	}else{
		ret += assign(root->val, reg);
	}
	return ret;
}
std :: string load_rvaltoreg(std :: string val, std :: string reg, std :: string index = "") {
	std :: string ret = "";
	if (index == "") {
		if (l_env.find(val) != l_env.end()) {
			ret = load(l_env[val], reg);
		}else{
			ret = load(g_env[val], reg);
		}
	}else{
		if (l_env.find(val) != l_env.end()) {
			//ret = loadaddr(l_env[val], "s0") + ("s0 = s0 + " + index + "\n") + (reg + " = s0 [0]\n");
			ret = load(l_env[val], "s0") + ("s0 = s0 + " + index + "\n") + (reg + " = s0 [0]\n");
		}else{
			//ret = loadaddr(g_env[val], "s0") + ("s0 = s0 + " + index + "\n") + (reg + " = s0 [0]\n");
			ret = load(g_env[val], "s0") + ("s0 = s0 + " + index + "\n") + (reg + " = s0 [0]\n");
		}
	}
	return ret;
}
std :: string store_regtovar(std :: string reg, std :: string var, std :: string index = "") {
	std :: string ret;
	if (l_env.find(var) != l_env.end()) {
		if (index == "")
			ret += store(l_env[var], reg);
		else{
			//ret += loadaddr(l_env[var], "s0") + ("s0 = s0 + " + index + "\n") + ("s0 [0] = " + reg + "\n");
			ret += load(l_env[var], "s0") + ("s0 = s0 + " + index + "\n") + ("s0 [0] = " + reg + "\n");
		}
	}else{
		ret += gstore(g_env[var], reg, index);
	}
	return ret;
}

int first(struct TreeNode* root, bool infunc = 0, int offset = 0) {
	if (!root) return 0;
	switch (root->type) {
		case NODE_GOALLIST :
			for (struct TreeNode* t : root->child) {
				first(t);
				root->code += t->code;
			}
			break;
		case NODE_VARDEFN_1 :
			if (infunc) {
				l_env.emplace(root->name, offset);
				return 1;
			}else{
				g_env.emplace(root->name, "v" + itos(g_env.size()));
				root->code = g_env[root->name] + " = 0\n";
			}
			break;
		case NODE_VARDEFN_2 :
			if (infunc) {
				l_env.emplace(root->name, offset);
				root->code += loadaddr(offset+1, "s0") + store(offset, "s0");
				return root->val / 4 + 1;
			}else{
				g_env.emplace(root->name, "v" + itos(g_env.size()));
				g_env_array.push_back(root->name);
				root->code += g_env[root->name] + " = malloc " + itos(root->val + 4) + "\n";
			}	
			break;
		case NODE_FUNCDEFN :
			param_cnt = 0;
			root->child[0]->val = root->val;
			first(root->child[0], true, 0);
			root->width = root->child[0]->width;
			root->env = l_env;
			l_env.clear();
			root->code += root->name + " [" + itos(root->val) + "] [" + itos(root->width) + "]\n";
			if (root->name == "f_main") {
				for (std :: string s : g_env_array) {
					root->code += loadaddr(g_env[s], "s0") + ("s1 = s0 + 4\n") + store_regtovar("s1", s);
				}
			}
			root->code += root->child[0]->code + ("end " + root->name + "\n");
			break;
		case NODE_STMTLIST :
			for (int i = 0; i < root->val; ++i) {
				l_env.emplace("p" + itos(i), root->width++);
				root->code += store(i, "a" + itos(i));
			}
			for (struct TreeNode* t : root->child) {
				root->width += first(t, true, root->width);
				root->code += t->code;
			}
			break;
		case NODE_EXPR_1 :
			root->code += load_rvaltoreg(root->child[0], "s1");
			root->code += load_rvaltoreg(root->child[1], "s2");
			root->code += "s3 = s1 " + root->op + " s2\n";
			root->code += store_regtovar("s3", root->name);
			break;
		case NODE_EXPR_2 :
			break;
		case NODE_EXPR_3 :
			break;
		case NODE_EXPR_4 :
			root->code += load_rvaltoreg(root->child[0], "s1");
			root->code += "s2 = " + root->op + " s1\n";
			root->code += store_regtovar("s2", root->name);
			break;
		case NODE_EXPR_5 :
			root->code += load_rvaltoreg(root->child[0], "s1");
			root->code += store_regtovar("s1", root->name);
			break;
		case NODE_EXPR_6 :
			param_cnt = 0;
			root->code += "call " + root->callid + "\n"
					+ store_regtovar("a0", root->name);
			break;
		case NODE_EXPR_7 :
			root->code += load_rvaltoreg(root->child[0], "s1");
			root->code += load_rvaltoreg(root->child[1], "s2");
			root->code += store_regtovar("s2", root->name, "s1");
			break;
		case NODE_EXPR_8 :
			root->code += load_rvaltoreg(root->child[0], "s1");
			root->code += load_rvaltoreg(root->callid, "s2", "s1");
			root->code += store_regtovar("s2", root->name);
			break;
		case NODE_EXPR_9 :
			root->code += load_rvaltoreg(root->child[0], "s1")
					+ load_rvaltoreg(root->child[1], "s2")
					+ "if s1 " + root->op + " s2 goto " + root->name + "\n";
			break;
		case NODE_EXPR_10 :
			root->code += "goto " + root->name + "\n";
			break;
		case NODE_EXPR_11 :
			root->code += load_rvaltoreg(root->child[0], "a" + itos(param_cnt++));
			break;
		case NODE_EXPR_12 :
			root->code += load_rvaltoreg(root->child[0], "a0");
			root->code += "return\n";
			break;
		case NODE_EXPR_13 :
			root->code += root->name + " :\n";
			break;
		case NODE_EXPR_14 :
			root->code += "return\n";
			break;
		case NODE_EXPR_15 :
			param_cnt = 0;
			root->code += "call " + root->name + "\n";
			break;
		case NODE_RVAL_1 :
			break;
		case NODE_RVAL_2 :
			break;
	}
	return 0;
}



/*

void first(struct TreeNode* root) {
	if (!root) return;
	switch (root->type) {
		case GOALLIST :
			break;
		case NODE_VARDEFN_1 :
			break;
		case NODE_VARDEFN_2 :
			break;
		case NODE_FUNCDEFN :
			break;
		case NODE_STMTLIST :
			break;
		case NODE_EXPR_1 :
			break;
		case NODE_EXPR_2 :
			break;
		case NODE_EXPR_3 :
			break;
		case NODE_EXPR_4 :
			break;
		case NODE_EXPR_5 :
			break;
		case NODE_EXPR_6 :
			break;
		case NODE_EXPR_7 :
			break;
		case NODE_EXPR_8 :
			break;
		case NODE_EXPR_9 :
			break;
		case NODE_EXPR_10 :
			break;
		case NODE_EXPR_11 :
			break;
		case NODE_EXPR_12 :
			break;
		case NODE_EXPR_13 :
			break;
		case NODE_EXPR_14 :
			break;
		case NODE_EXPR_15 :
			break;
		case NODE_RVAL_1 :
			break;
		case NODE_RVAL_2 :
			break;
	}
}
*/
