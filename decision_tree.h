#ifndef DECISION_TREE
#define DECISION_TREE

#include "general/general_c/strings_and_files.h"
#include "general/general_cpp/string.h"
#include "general/general_cpp/vector.h"

//=============================================================================
// Abstracts ==================================================================

class AbstractDecisionStatement {
public:
	virtual int  state(char *end = (char*) "") = 0;
	virtual int  size() = 0;
	//virtual bool operator==(const AbstractDecisionStatement &other) const = 0;
	virtual void dump(FILE *file_ptr = nullptr) const = 0;
};

class DecisionAbstractQuestion : public AbstractDecisionStatement {
public:
	virtual int get_answer() = 0;
};

//=============================================================================
// Statement & Question =======================================================

class DecisionStatement : public AbstractDecisionStatement {
private:
	String *statement;

public:
	DecisionStatement(String *statement_);
	int  state(char *end = (char*) "");
	int  size();
	String &get_statement() const;
	bool operator==(const AbstractDecisionStatement &other) const;
	bool operator==(const DecisionStatement &other) const;
	void dump(FILE *file_ptr = nullptr) const;
};

class DecisionDefinition : public DecisionStatement {
public:
	DecisionDefinition(String *statement_) : DecisionStatement(statement_) {};

	int state(char *end = (char*) "");
	int get_answer();
};


class DecisionQuestion : public DecisionStatement {
public:
	DecisionQuestion(String *statement_) : DecisionStatement(statement_) {};

	int state(char *end = (char*) "");
	int get_answer();
};

//=============================================================================
// Node =======================================================================

class DecisionTreeNode {
private:
	AbstractDecisionStatement *statement;
	DecisionTreeNode *node_true;
	DecisionTreeNode *node_false;

public:
	DecisionTreeNode();
	DecisionTreeNode(AbstractDecisionStatement *statement_, DecisionTreeNode* node_true_ = nullptr, DecisionTreeNode *node_false_  = nullptr);

	void set_true (DecisionTreeNode* node);
	void set_false(DecisionTreeNode* node);

	bool is_question() const;
	bool is_defenition() const;

	DecisionTreeNode *get_node_true () const;
	DecisionTreeNode *get_node_false() const;
	String &get_statement() const;

	int statement_length();

	int state();
	void dump(FILE *file_ptr = nullptr) const;

	DecisionTreeNode *proceed(const int answer);
};

//=============================================================================
// Tree =======================================================================

const char SYMB_OPEN_NODE  = '[';
const char SYMB_CLOSE_NODE = ']';
const char SYMB_QUOTE      = '"';

const int MAX_STATEMENT_LEN = 50;

enum GUESS_GAME_OUTCOMES {
	GUESS     = 2,
	QUESTION  = 0,
	NO  	  = 0,
	YES 	  = 1,
	GUESS_NO  = 2,
	GUESS_YES = 3,
};

class DecisionTree {
private:
	DecisionTreeNode *root;

	DecisionTreeNode* load_node(File *file_ptr);
	int               save_node(const DecisionTreeNode* node, int depth, bool is_false_node, FILE *file_ptr);

	bool node_find_definition_way(const String &definition, const DecisionTreeNode *cur_node, Vector<char> *buffer);
	Vector<char> *find_definition_way(const String &statement);

	void dump(DecisionTreeNode *node, int depth, int to_format_cnt, int maxlen, FILE *file_ptr) const;

	int print_definition(const String &defenition);
	int run_new_node_generation(DecisionTreeNode *cur_node, DecisionTreeNode* prev_node, const int prev_ans);

	void print_prefixed_statement(const String &statement, const bool truth) const;
	void print_definition_by_way(const Vector<char> &way, const int min_depth = 0, const int max_depth = -1) const;

	DecisionTreeNode *merge_node(DecisionTreeNode *fisrt, DecisionTreeNode *second);

public:
	DecisionTree();

	int load(const char *file_name);
	int save(const char *file_name);

	DecisionTreeNode *get_root() const;

	int run_guess();
	int run_define();
	int run_difference();

	int run_interaction();

	void dump(FILE *file_ptr = nullptr) const;

	void merge(const DecisionTree &tree);
};

#endif // DECISION_TREE