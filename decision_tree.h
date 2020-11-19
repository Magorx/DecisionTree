#ifndef DECISION_TREE
#define DECISION_TREE

#include "general/general_c/warnings.h"
#pragma GCC diagnostic ignored "-Weffc++"

#include "general/general_c/strings_and_files.h"
#include "general/general_cpp/string.h" // todo redo
#include "general/general_cpp/vector.h"

const char SYMB_OPEN_NODE  = '[';
const char SYMB_CLOSE_NODE = ']';
const char SYMB_QUOTE      = '"';

const int MAX_STATEMENT_LEN = 10;

enum GUESS_GAME_OUTCOMES {
	GUESS     = 2,
	QUESTION  = 0,
	NO  	  = 0,
	YES 	  = 1,
	GUESS_NO  = 2,
	GUESS_YES = 3,
};

//=============================================================================
// DecisionTreeNode ===========================================================

class DecisionTreeNode {
private:
// data =======================================================================
	String           *statement;
	DecisionTreeNode *node_true;
	DecisionTreeNode *node_false;
// meth =======================================================================

	int answer_is_yes(const char *answer) {
		return (answer[0] == 'y' || answer[0] == 'Y' || answer[0] == '+' || answer[0] == '1');
	}

	int get_and_process_answer() {
		printf("> ");
		char answer[MAX_STATEMENT_LEN];
		scanf("%50s", answer);

		if (answer_is_yes(answer)) {
			return YES;
		} else {
			return NO;
		}
	}

public:
	DecisionTreeNode () {}; 
	
	void ctor() {
		statement  = nullptr;
		node_true  = nullptr;
		node_false = nullptr;
	}

	static DecisionTreeNode *NEW() {
		DecisionTreeNode *cake = (DecisionTreeNode*) calloc(1, sizeof(DecisionTreeNode));
		if (!cake) {
			return nullptr;
		}

		cake->ctor();
		return cake;
	}

	void ctor(String *statement_, DecisionTreeNode  *node_true_  = nullptr, DecisionTreeNode *node_false_ = nullptr) {
		statement  = statement_;
		node_true  = node_true_;
		node_false = node_false_;
	}

	static DecisionTreeNode *NEW(String *statement_, DecisionTreeNode  *node_true_  = nullptr, DecisionTreeNode *node_false_ = nullptr) {
		DecisionTreeNode *cake = (DecisionTreeNode*) calloc(1, sizeof(DecisionTreeNode));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(statement_, node_true_, node_false_);
		return cake;
	}

	~DecisionTreeNode() {};

	void dtor() {
		String::DELETE(statement);
		statement = nullptr;
	}

	static void DELETE(DecisionTreeNode *node) {
		if (!node) {
			return;
		}

		node->dtor();
		free(node);
	}

	void set_true (DecisionTreeNode* node) {
		node_true = node;
	}

	void set_false(DecisionTreeNode* node) {
		node_false = node;
	}

	bool is_question  () const {
		return node_true && node_false;
	}

	bool is_defenition() const {
		return !(node_true && node_false);
	}

	DecisionTreeNode *get_node_true () const {
		return node_true;
	}

	DecisionTreeNode *get_node_false() const {
		return node_false;
	}

	const String &get_statement () const {
		return *statement;
	}

	int state() {
		if (is_defenition()) {
			return state_definition();
		} else {
			return state_question();
		}
	}

	int state_definition() {
		printf("Yout object is ");
		statement->print();
		printf("!\n");
		printf("Am I right?\n");

		return GUESS + get_and_process_answer();
	}

	int state_question() {
		printf("Is it true that your object ");
		statement->print();
		printf("?\n");

		return QUESTION + get_and_process_answer();
	}

	void dump(FILE *file_ptr = stdout) const {
		statement->print(file_ptr);
	}

	DecisionTreeNode *proceed(const int answer) const {
		if (answer == YES) {
			return node_true;
		} else {
			return node_false;
		}
	}
};


//=============================================================================
// DecisionTree ===============================================================


class DecisionTree {
private:
// data =======================================================================
	DecisionTreeNode *root;
//=============================================================================
// meth =======================================================================
	DecisionTreeNode *load_node(File *file) {
		const unsigned char *c = file->cc;
		Char_get_next_symb(&c);
		if (*c != SYMB_QUOTE) {
			printf("[ERR]<DeTreeNode>: invalid file being loaded\n");
			return nullptr;
		}

		++c;
		String *node_statement = new String();
		c += node_statement->read(c, false, '"');

		++c;
		Char_get_next_symb(&c);
		file->cc = c;

		if (*c == SYMB_OPEN_NODE) {
			DecisionTreeNode *node = DecisionTreeNode::NEW(node_statement);

			++file->cc;

			node->set_true (load_node(file));
			node->set_false(load_node(file));

			Char_get_next_symb(&file->cc);
			++file->cc;

			return node;
		} else if (*c == SYMB_CLOSE_NODE || *c == SYMB_QUOTE) {
			DecisionTreeNode *node = DecisionTreeNode::NEW(node_statement);

			return node;
		} else {
			printf("[ERR]<DeTreeNode>: invalid file being loaded\n");
			return nullptr;
		}
	}

	void file_printf_tab(FILE *file_ptr, const int tab) {
		for (int i = 0; i < tab; ++i) {
			fprintf(file_ptr, "    ");
		}
	}

	int save_node(const DecisionTreeNode* node, int depth, bool is_true_node, FILE *file_ptr) {
		if (!node) {
			printf("Invalid tree: a node is not presented\n");
			return -1;
		}

		file_printf_tab(file_ptr, depth);
		fprintf(file_ptr, "%c", SYMB_QUOTE);
		node->dump(file_ptr);
		fprintf(file_ptr, "%c", SYMB_QUOTE);

		fprintf(file_ptr, "\n");
		if (node->get_node_true()) {
			file_printf_tab(file_ptr, depth);
			fprintf(file_ptr, "%c\n", SYMB_OPEN_NODE);

			save_node(node->get_node_true(),  depth + 1, true,  file_ptr);
			save_node(node->get_node_false(), depth + 1, false, file_ptr);

			file_printf_tab(file_ptr, depth);
			fprintf(file_ptr, "%c\n", SYMB_CLOSE_NODE);
		}

		if (is_true_node) {
			fprintf(file_ptr, "\n");
		}

		return 0;
	}

	bool node_find_definition_way(const String &definition, const DecisionTreeNode *cur_node, Vector<char> *buffer) {
		if (cur_node->is_question()) {
			buffer->push_back(1);
			if (node_find_definition_way(definition, cur_node->get_node_true(), buffer)) {
				return buffer;
			} else {
				buffer->pop_back();
			}

			buffer->push_back(0);
			if (node_find_definition_way(definition, cur_node->get_node_false(), buffer)) {
				return buffer;
			} else {
				buffer->pop_back();
			}
		} else {
			if (cur_node->get_statement() == definition) {
				return true;
			}
		}

		return false;
	}

	Vector<char> *find_definition_way (const String &definition) {
		Vector<char> *buffer = new Vector<char>();
		node_find_definition_way(definition, root, buffer);
		return buffer;
	}

	int print_definition(const String &definition) {
		Vector<char> *way = find_definition_way(definition);
		if (way->size() == 0) {
			printf("I don't know what [");
			definition.print();
			printf("] is!\n");
			return 0;
		}

		definition.print();
		printf(" ");
		print_definition_by_way(*way);

		delete way;
		return 0;
	}

	void dump(DecisionTreeNode *node, int depth, int to_format_cnt, int maxlen, FILE *file_ptr) const {
		if (!node) {return;}

		dump(node->get_node_true(), depth + 1, to_format_cnt + 1, maxlen, file_ptr);

		for (int i = 0; i < depth; ++i) {
			for (int j = 0; j < maxlen; ++j) {
				printf(" ");
			}
			if (depth - to_format_cnt- 1 <= i) {
				printf("|");
			} else {
				printf(" ");
			}
		}

		node->dump(file_ptr);
		for (int i = 0; i < maxlen - (int) node->get_statement().length() - 1; ++i) {
			printf("-");
		}
		printf("->|\n");
		dump(node->get_node_false(), depth + 1, to_format_cnt + 1, maxlen, file_ptr);
	}

	int run_new_node_generation(DecisionTreeNode *cur_node, DecisionTreeNode* prev_node, const int prev_ans) {
		printf("Well, okay...\n");
		printf("What is your object?\n> ");

		char str[MAX_STATEMENT_LEN];
		scanf ("%[^\n]%*c", str);
		String *definition = new String(str);
		DecisionTreeNode *new_defenition_node = DecisionTreeNode::NEW(definition);

		printf("\nAnd how is [");
		new_defenition_node->dump();
		printf("] different from [");
		cur_node->dump();
		printf("]? It... /*continue the phrase*/\n> ");

		scanf ("%[^\n]%*c", str);
		String *question = new String(str);
		DecisionTreeNode *new_question_node = DecisionTreeNode::NEW(question);
		new_question_node->set_true (new_defenition_node);
		new_question_node->set_false(cur_node);

		if (prev_ans) {
			prev_node->set_true(new_question_node);
		} else {
			prev_node->set_false(new_question_node);
		}

		printf("\nI'll remember!\n");

		return 0;
	}

	void print_prefixed_statement(const String &statement, const bool truth) const {
		if (!statement.length()) {
			return;
		}

		if (truth) {
			statement.print();
			return;
		}

		if (statement[0] == 'i') {
			printf("isn't ");
			statement.print(stdout, 3);
		} else {
			printf("doesn't ");
			statement.print(stdout, 5);
		}
	}

	void print_definition_by_way(const Vector<char> &way, const int min_depth = 0, const int max_depth = -1) const {
		DecisionTreeNode *node = root;

		size_t way_size = 0;
		if (max_depth >= 0) {
			way_size = std::min((int) way.size(), max_depth);
		} else {
			way_size = way.size();
		}

		int definitions_printed = 0;
		for (size_t i = 0; i < way_size - 1; ++i) {
			if ((int) i >= min_depth) {
				++definitions_printed;
				print_prefixed_statement(node->get_statement(), way[i]);
				printf(", ");
			}

			if (way[i]) {
				node = node->get_node_true();
			} else {
				node = node->get_node_false();
			}
		}

		print_prefixed_statement(node->get_statement(), way[way_size - 1]);
	}

	DecisionTreeNode *merge_node(DecisionTreeNode *first, DecisionTreeNode *second) {
		if (first->is_question() && second->is_question()) {
			if (first->get_statement() == second->get_statement()) {
				first->set_true (merge_node(first->get_node_true (), second->get_node_true ()));
				first->set_false(merge_node(first->get_node_false(), second->get_node_false()));
			} else {
				printf("[");
				first->get_statement().print();
				printf("] vs [");
				second->get_statement().print();
				printf("]\n");
				printf("[ERR]<DeTree>: merge error, aborting\n");
				exit(0);
			}
		}

		if (first->is_question()) {
			return first;
		} else {
			return second;
		}
	}

//=============================================================================
public:
	DecisionTree () {};

	void ctor() {
		root = nullptr;
	}

	~DecisionTree() {};

	void dtor() {
		if (root) {
			DecisionTreeNode::DELETE(root);
		}
	}

	int load(const char *file_name) {
		if (file_name == nullptr) {
			printf("[ERR]<detree>: [file_name](nullptr)\n");
			return -1;
		}

		File file = {};
		if (File_construct(&file, file_name) < 0) {
			printf("[ERR]<detree>: [file_name](%s) unexistance\n", file_name);
		}

		root = load_node(&file);
		File_destruct(&file);

		return 0;
	}

	int save(const char *file_name) {
		if (file_name == nullptr) {
			printf("[ERR]<detree>: [file_name](nullptr)\n");
			return -1;
		}

		FILE *file = fopen(file_name, "w");
		if (!file) {
			printf("[ERR]<detree>: [file_name](%s) can't be opened\n", file_name);
			return -1;
		}

		save_node(root, 0, false, file);

		fclose(file);
		return 0;
	}

	DecisionTreeNode *get_root() const {
		return root;
	}

//=============================================================================
// Run_modes ==================================================================
//=============================================================================

	int run_guess() {
		DecisionTreeNode *cur_node = root;

		int answer = QUESTION;
		int prev_ans = QUESTION;
		DecisionTreeNode *prev_node = nullptr;

		while (true) {
			prev_ans = answer;
			answer   = cur_node->state();
			printf("\n");

			if (answer < GUESS) {
				prev_node = cur_node;
				cur_node = cur_node->proceed(answer);
			} else {
				break;
			}
		}

		getchar(); // getting rid of \n after scanf

		if (answer == GUESS_YES) {
			printf("Accurate as always \\(>o<)/\n");
		} else {
			run_new_node_generation(cur_node, prev_node, prev_ans);
		}

		return 0;
	}

	int run_define() {
		printf("What object do you want me to define?\n> ");
		char str[MAX_STATEMENT_LEN];
		scanf("%[^\n]%*c", str);
		String defenition(str);

		printf("\n");
		print_definition(defenition);
		printf("\n");

		return 0;
	}

	int run_difference() {
		printf("What object do you want me to compare?\n> ");
		char c_first[MAX_STATEMENT_LEN];
		scanf("%[^\n]%*c", c_first);
		String first(c_first);

		printf("What should I compare it with?\n> ");
		char c_second[MAX_STATEMENT_LEN];
		scanf("%[^\n]%*c", c_second);
		String second(c_second);

		printf("\n");

		if (first == second) {
			printf("They are just the same, pathetic human...\n");
			return 0;
		}

		Vector<char> *way_first  = find_definition_way(first );
		Vector<char> *way_second = find_definition_way(second);

		if (!way_first->size()) {
			printf("Oh, I don't know what ");
			first.print();
			printf(" is\n");
			return 0;
		}

		if (!way_second->size()) {
			printf("Oh, I don't know what ");
			second.print();
			printf(" is\n");
			return 0;
		}

		int common_part = 0;
		for (; common_part < (int) way_first ->size()  && 
			   common_part < (int) way_second->size() &&
			   (*way_first)[common_part] == (*way_second)[common_part]; ++common_part);
		if (common_part == 0) {
			printf("They are so different...\n");
		} else {
			first.print();
			printf(" ");
			print_definition_by_way(*way_first, 0, common_part);
			printf(" and so is ");
			second.print();

			printf("\n\n~~But~~\n");
		}

		first.print();
		printf(" ");
		print_definition_by_way(*way_first, common_part);

		printf("\n~While~\n");

		second.print();
		printf(" ");
		print_definition_by_way(*way_second, common_part);
		printf("\n~~~~~~~\n");

		delete way_first;
		delete way_second;

		return 0;
	}

	int run_interaction() {
		printf("[ ] ~ Which mode of Ultra-De-Tree you want to use?\n");
		printf("[1] - play a guessing game\n");
		printf("[2] - get a definition of an object\n");
		printf("[3] - get a difference between two objects\n");
		printf("[4] - merge 'db1.db' with 'db2.db' into 'db_out.db'\n");
		printf("> ");

		int answer = 0;
		int cnt = 0;
		scanf("%d", &answer);
		while (!(answer >= 1 && answer <= 4)) {
			++cnt;
			printf("Ha-ha, very funny, try again\n");
			if (cnt > 10) {
				printf("I'll keep you here forever...\n");
			}
			printf("> ");
			scanf("%d", &answer);
		}
		printf("\n");

		switch (answer) {
			case 1: {
				run_guess();
				break;
			}
			case 2: {
				getchar();
				run_define();
				break;
			}
			case 3: {
				getchar();
				run_difference();
				break;
			}
			case 4: {
				DecisionTree first, second;
				first.load("db1.db");
				second.load("db2.db");

				first.merge(second);

				first.save("db_out.db");
				printf(".doned.\n");
				break;
			}
			default: {
				printf("I'm feeling strange...\n");
				break;
			}
		}

		return 0;
	}

//=============================================================================

	void dump(FILE *file_ptr = stdout) const {
		dump(root, 0, 0, MAX_STATEMENT_LEN, file_ptr);
	}

	void merge(const DecisionTree &tree) {
		root = merge_node(root, tree.get_root());
	}
};

#endif // DECISION_TREE