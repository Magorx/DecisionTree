#ifndef DECISION_TREE
#define DECISION_TREE

#include "general/warnings.h"

#include "general/c/strings_and_files.h"
#include "general/cpp/stringview.hpp"
#include "general/cpp/vector.hpp"

const char SYMB_OPEN_NODE  = '[';
const char SYMB_CLOSE_NODE = ']';
const char SYMB_QUOTE      = '"';

const int MAX_STATEMENT_LEN = 50;
const double OVERLOAD_COEF = 0.05;

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
	StringView           *statement;
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
	DecisionTreeNode           (const DecisionTreeNode& other) = delete;
	DecisionTreeNode& operator=(const DecisionTreeNode& other) = delete;

	DecisionTreeNode ():
	statement(nullptr),
	node_true(nullptr),
	node_false(nullptr)
	{} 
	
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

	void ctor(StringView *statement_, DecisionTreeNode  *node_true_  = nullptr, DecisionTreeNode *node_false_ = nullptr) {
		statement  = statement_;
		node_true  = node_true_;
		node_false = node_false_;
	}

	static DecisionTreeNode *NEW(StringView *statement_, DecisionTreeNode  *node_true_  = nullptr, DecisionTreeNode *node_false_ = nullptr) {
		DecisionTreeNode *cake = (DecisionTreeNode*) calloc(1, sizeof(DecisionTreeNode));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(statement_, node_true_, node_false_);
		return cake;
	}

	~DecisionTreeNode() {};

	void dtor(bool recursive = false) {
		StringView::DELETE(statement);
		statement = nullptr;

		if (recursive) {
			if (node_true)  node_true-> dtor(recursive);
			if (node_false) node_false->dtor(recursive);
		}
	}

	static void DELETE(DecisionTreeNode *node, bool recursive = false) {
		if (!node) {
			return;
		}

		node->dtor();
		if (recursive) {
			if (node->node_true)  DELETE(node->node_true,  recursive);
			if (node->node_false) DELETE(node->node_false, recursive);
		}

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

	const StringView &get_statement () const {
		return *statement;
	}

	int state(bool festival_verbosity = false) {
		if (is_defenition()) {
			return state_definition(festival_verbosity);
		} else {
			return state_question(festival_verbosity);
		}
	}

	int state_definition(bool festival_verbosity = false) {
		printf("Yout object is ");
		statement->print();
		printf("!\n");
		printf("Am I right?\n");

		if (festival_verbosity) {
			char format[200] = "echo Your object is %s! Am I right? | festival --tts";
			statement->generate_length_format(format);

			char generated_say_command[200];
			sprintf(generated_say_command, format, statement->get_buffer());

			system(generated_say_command);
		}

		return GUESS + get_and_process_answer();
	}

	int state_question(bool festival_verbosity = false) {
		printf("Is it true that your object ");
		statement->print();
		printf("?\n");

		if (festival_verbosity) {
			char format[200] = "echo Is it true that your object %s? | festival --tts";
			statement->generate_length_format(format);

			char generated_say_command[200];
			sprintf(generated_say_command, format, statement->get_buffer());

			system(generated_say_command);
		}

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
	File *db_file;
	bool festival_verbosity;
	int cur_node_cnt;
	int new_node_cnt;
//=============================================================================
// meth =======================================================================

	#define FESTIVAL_SAY(string) if (festival_verbosity) system("echo " string " | festival --tts");;

	DecisionTreeNode *load_node(File *file) {
		unsigned char *c = file->cc;
		Char_get_next_symb(&c);
		if (*c != SYMB_QUOTE) {
			printf("[ERR]<DeTreeNode>: invalid file being loaded\n");
			return nullptr;
		}

		++c;
		StringView *node_statement = StringView::NEW();
		c += node_statement->read((char*) c, false, '"');

		++c;
		Char_get_next_symb(&c);
		file->cc = c;

		++cur_node_cnt;
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

	bool node_find_definition_way(const StringView &definition, const DecisionTreeNode *cur_node, Vector<char> *buffer) {
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

	Vector<char> *find_definition_way(const StringView &definition) {
		Vector<char> *buffer = Vector<char>::NEW();
		node_find_definition_way(definition, root, buffer);
		return buffer;
	}

	int print_definition(const StringView &definition) {
		Vector<char> *way = find_definition_way(definition);
		if (way->size() == 0) {
			printf("What the hell even is it?\n");
			festival_no_such_object();
			return 0;
		}

		definition.print();
		printf(" ");
		print_definition_by_way(*way);

		Vector<char>::DELETE(way);
		return 0;
	}

	void print_prefixed_statement(const StringView &statement, const bool truth) const {
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
		FESTIVAL_SAY("Well, okay...");
		FESTIVAL_SAY("What is your object?");

		char *c_definition = (char*) calloc(MAX_STATEMENT_LEN, sizeof(char));
		scanf ("%[^\n]%*c", c_definition);
		StringView *definition = StringView::NEW(c_definition, true);
		DecisionTreeNode *new_defenition_node = DecisionTreeNode::NEW(definition);

		printf("\nAnd how is [");
		new_defenition_node->dump();
		printf("] different from [");
		cur_node->dump();
		printf("]? It... /*continue the phrase*/\n> ");
		FESTIVAL_SAY("And what??");

		char *c_question = (char*) calloc(MAX_STATEMENT_LEN, sizeof(char));
		scanf ("%[^\n]%*c", c_question);
		StringView *question = StringView::NEW(c_question, true);
		DecisionTreeNode *new_question_node = DecisionTreeNode::NEW(question);
		new_question_node->set_true (new_defenition_node);
		new_question_node->set_false(cur_node);

		if (prev_ans) {
			prev_node->set_true(new_question_node);
		} else {
			prev_node->set_false(new_question_node);
		}

		printf("\nI'll remember!\n");
		FESTIVAL_SAY("I won't remember");

		++new_node_cnt;
		return 0;
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

	void graphviz_dump_node_name(const DecisionTreeNode *node, FILE *file) {
		fprintf(file, "\"");
		node->get_statement().print(file);
		fprintf(file, "\"");
	}

	void graphviz_dump_node(const DecisionTreeNode *node, FILE *file) {
		graphviz_dump_node_name(node, file);
		fprintf(file, "[label=");
		graphviz_dump_node_name(node, file);

		if (node == root) {
			fprintf(file, "fillcolor=\"#000000\" style=filled color=gold penwidth=2 fontcolor=white");
		} else if (node->is_defenition()) {
			fprintf(file, "fillcolor=\"#CCCCFF\" style=filled color=red   penwidth=2");
		} else {
			fprintf(file, "fillcolor=\"#CCFFCC\" style=filled color=green penwidth=2");
		}

		fprintf(file, "]\n");
	}

	void graphviz_dump(const DecisionTreeNode *node, FILE *file) {
		if (!node) {return;}

		graphviz_dump_node(node, file);

		if (node->get_node_true()) {
			graphviz_dump_node_name(node, file);
			fprintf(file, "->");
			graphviz_dump_node_name(node->get_node_true(), file);
			fprintf(file, "[color=green label=\"yes\" penwidth=2]\n");
		}
		if (node->get_node_false()) {
			graphviz_dump_node_name(node, file);
			fprintf(file, "->");
			graphviz_dump_node_name(node->get_node_false(), file);
			fprintf(file, "[color=red label=\"no\" penwidth=2]\n");
		}

		graphviz_dump(node->get_node_true (), file);
		graphviz_dump(node->get_node_false(), file);
	}

//=============================================================================
public:
	DecisionTree           (const DecisionTree& other) = delete;
	DecisionTree& operator=(const DecisionTree& other) = delete;

	DecisionTree():
	root(nullptr),
	db_file(nullptr),
	festival_verbosity(false),
	cur_node_cnt(0),
	new_node_cnt(0)
	{};

	void ctor() {
		root = nullptr;
		db_file = nullptr;
		festival_verbosity = false;
		cur_node_cnt  = 0;
		new_node_cnt = 0;
	}

	~DecisionTree() {};

	void dtor() {
		if (root) {
			DecisionTreeNode::DELETE(root, true);
		}
		root = nullptr;

		if (db_file) {
			File_destruct(db_file);
			free(db_file);
		}
		db_file = nullptr;
	}

//=============================================================================
// Disk work ==================================================================
//=============================================================================

	int load(const char *file_name) {
		if (file_name == nullptr) {
			printf("[ERR]<detree>: [file_name](nullptr)\n");
			return -1;
		}

		File *file = (File*) calloc(1, sizeof(File));
		if (File_construct(file, file_name) < 0) {
			printf("[ERR]<detree>: [file_name](%s) unexistance\n", file_name);
		}

		root = load_node(file);
		db_file = file;

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

	void reload(const char *reload_db_name = "rld.db", bool force = false) {
		if (!new_node_cnt && !force) {
			return;
		}

		save(reload_db_name);
		dtor();
		ctor();

		load(reload_db_name);
	}

	bool check_and_fix_overburdance(const char *reload_db_name = "rld.db") {
		if ((double) cur_node_cnt / (double) new_node_cnt > OVERLOAD_COEF) {
			reload(reload_db_name);
			return true;
		} else {
			return false;
		}
	}

	DecisionTreeNode *get_root() const {
		return root;
	}

//=============================================================================
// Festival ===================================================================
//=============================================================================

	void festival_read_it_yourself() {
		if (festival_verbosity) printf("Try to read it outloud by yourself...\n\n");
		FESTIVAL_SAY("Try to read it outloud by yourself...");
	}

	void festival_no_such_object() {
		FESTIVAL_SAY("What the hell even is it?");
	}

//=============================================================================
// Run_modes ==================================================================
//=============================================================================

	int run_guess(bool fest_verbosity = false) {
		DecisionTreeNode *cur_node = root;

		int answer = QUESTION;
		int prev_ans = QUESTION;
		DecisionTreeNode *prev_node = nullptr;

		while (true) {
			prev_ans = answer;
			answer   = cur_node->state(fest_verbosity);
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
			if (fest_verbosity) system("echo Accurate as always! | festival --tts");
		} else {
			run_new_node_generation(cur_node, prev_node, prev_ans);
		}

		return 0;
	}

	int run_define() {
		printf("What object do you want me to define?\n> ");
		FESTIVAL_SAY("What object do you want me to define?");
		char str[MAX_STATEMENT_LEN];
		scanf("%[^\n]%*c", str);
		StringView defenition;
		defenition.ctor(str);

		festival_read_it_yourself();
		printf("\n");
		print_definition(defenition);
		printf("\n");

		defenition.dtor();

		return 0;
	}

	int run_difference() {
		printf("What object do you want me to compare?\n> ");
		FESTIVAL_SAY("What object do you want me to compare?");
		char c_first[MAX_STATEMENT_LEN];
		scanf("%[^\n]%*c", c_first);
		StringView first;
		first.ctor(c_first);

		printf("What should I compare it with?\n> ");
		FESTIVAL_SAY("What should I compare it with?");
		char c_second[MAX_STATEMENT_LEN];
		scanf("%[^\n]%*c", c_second);
		StringView second;
		second.ctor(c_second);

		printf("\n");

		if (first == second) {
			printf("They are just the same, pathetic human...\n");
			FESTIVAL_SAY("They are just the same, pathetic human...");
			return 0;
		}

		Vector<char> *way_first  = find_definition_way(first );
		Vector<char> *way_second = find_definition_way(second);

		if (!way_first->size()) {
			printf("What the hell even is it?\n");
			festival_no_such_object();
			return 0;
		}

		if (!way_second->size()) {
			printf("What the hell even is it?\n");
			festival_no_such_object();
			return 0;
		}

		int common_part = 0;
		for (; common_part < (int) way_first ->size()  && 
			   common_part < (int) way_second->size() &&
			   (*way_first)[common_part] == (*way_second)[common_part]; ++common_part);
		if (common_part == 0) {
			printf("They are so different...\n");
			FESTIVAL_SAY("They are so different...");
		} else {
			festival_read_it_yourself();
			first.print();
			printf(" ");
			print_definition_by_way(*way_first, 0, common_part);
			printf(" and so is ");
			second.print();

			printf("\nBut\n");
			FESTIVAL_SAY("But...");
		}

		if (common_part == 0) {
			festival_read_it_yourself();
		}

		first.print();
		printf(" ");
		print_definition_by_way(*way_first, common_part);

		printf("\nWhile\n");

		second.print();
		printf(" ");
		print_definition_by_way(*way_second, common_part);
		printf("\n");

		first.dtor();
		second.dtor();
		Vector<char>::DELETE(way_first);
		Vector<char>::DELETE(way_second);

		return 0;
	}

	void print_interface() {
		printf("/^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\\ \n");
		printf("+-------------------+-----------+---------------------+  \n");
		printf("|                 <~| DeTreeser |~>                   |  \n");
		printf("+-------------------+-----------+---------------------+  \n");
		printf("| [q] - exit                                          |  \n");
		printf("| [1] - play a guessing game                          |  \n");
		printf("| [2] - get a definition of an object                 |  \n");
		printf("| [3] - get a difference between two objects          |  \n");
		printf("| [g] - make a pretty dump                            |  \n");
		printf("| [v] - change verbosity for festival, currently: %s  |  \n", festival_verbosity ? "on" : "of");
		printf("| [m] - merge 'db1.db' with 'db2.db' into 'db_out.db' |  \n");
		printf("+-----------------------------------------------------+  \n");
		printf("\\_____________________________________________________/ \n");
	}

	int run_interaction() {
		while (true) {
			check_and_fix_overburdance();
			print_interface();
			printf("> ");
			char answer;
			scanf("%c", &answer);

			switch (answer) {
				case 'q': {
					return 0;
				}
				case '1': {
					run_guess(festival_verbosity);
					break;
				}
				case '2': {
					getchar();
					run_define();
					break;
				}
				case '3': {
					getchar();
					run_difference();
					break;
				}
				case 'g': {
					graphviz_dump("akidump", "svg");
					break;
				}
				case 'v': {
					festival_verbosity ^= 1;
					break;
				}
				case 'm': {
					DecisionTree first, second;
					first.load("db1.db");
					second.load("db2.db");

					first.merge(second);

					first.save("db_out.db");
					printf(".doned.\n");
					break;
				}
				default: {
					printf("Make a simple choice, human!\n");
					break;
				}
			}

			printf("\n");
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

	int graphviz_dump(const char *file_name, const char *format = "svg") {
		if (file_name == nullptr) {
			printf("[ERR]<detree>: [file_name](nullptr)\n");
			return -1;
		}

		FILE *file = fopen(file_name, "w");
		if (!file) {
			printf("[ERR]<detree>: [file_name](%s) can't be opened\n", file_name);
			return -1;
		}

		fprintf(file, "digraph list {rankdir=\"UD\";\n");

		graphviz_dump(root, file);
		
		fprintf(file, "}\n");
		fclose(file);

		char generate_picture_command[100];
		sprintf(generate_picture_command, "dot %s -T%s -o%s.svg", file_name, format, file_name);

		char view_picture_command[100];
		sprintf(view_picture_command, "eog %s.%s", file_name, format);
		
		system(generate_picture_command);
		system(view_picture_command);

		return 0;
	}

	#undef FESTIVAL_SAY
};

#endif // DECISION_TREE