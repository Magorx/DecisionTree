#include "decision_tree.h"

#include <cstdio>

//=============================================================================
// Statement and Question =====================================================

DecisionStatement::DecisionStatement(String *statement_) {
	statement = statement_;
}

int DecisionStatement::state(char *end) {
	statement->print();
	printf("%s", end);
	return 0;
}

int DecisionStatement::size() {
	return (int) statement->length();
}

void DecisionStatement::dump(FILE *file_ptr) const {
	statement->print(file_ptr);
}

String &DecisionStatement::get_statement() const {
	return *statement;
}


bool DecisionStatement::operator==(const DecisionStatement &other) const {
	return *statement == other.get_statement();
}

int answer_is_yes(char *answer) {
	return (answer[0] == 'y' || answer[0] == 'Y' || answer[0] == '+' || answer[0] == '1');
}

int get_and_process_answer() {
	printf("> ");
	char answer[51];
	scanf("%50s", answer);

	if (answer_is_yes(answer)) {
		return YES;
	} else {
		return NO;
	}
}

int DecisionDefinition::state(char *end) {
	printf("Yout object is ");
	DecisionStatement::state(end);
	printf("!\n");
	printf("Am I right?\n");

	return GUESS + get_and_process_answer();
}

int DecisionQuestion::state(char *end) {
	printf("Is it true that your object ");
	DecisionStatement::state(end);
	printf("?\n");

	return QUESTION + get_and_process_answer();
}

//=============================================================================
// Node =======================================================================

DecisionTreeNode::DecisionTreeNode() {
	statement  = nullptr;
	node_true  = nullptr;
	node_false = nullptr;
}

DecisionTreeNode::DecisionTreeNode(AbstractDecisionStatement *statement_, DecisionTreeNode* node_true_, DecisionTreeNode *node_false_) {
	statement  = statement_ ;
	node_true  = node_true_ ;
	node_false = node_false_;
}

void DecisionTreeNode::set_true(DecisionTreeNode* node) {
	node_true = node;
}

void DecisionTreeNode::set_false(DecisionTreeNode* node) {
	node_false = node;
}

DecisionTreeNode *DecisionTreeNode::get_node_true() const {
	return node_true;
}

DecisionTreeNode *DecisionTreeNode::get_node_false() const {
	return node_false;
}

String &DecisionTreeNode::get_statement() const {
	return (dynamic_cast<const DecisionStatement*>(statement))->get_statement();
}

DecisionTreeNode *DecisionTreeNode::proceed(const int answer) {
	if (answer) {
		return node_true;
	} else {
		return node_false;
	}
}

int DecisionTreeNode::state() {
	return statement->state();
}

void DecisionTreeNode::dump(FILE *file_ptr) const {
	statement->dump(file_ptr);
}

int DecisionTreeNode::statement_length() {
	return statement->size();
}

bool DecisionTreeNode::is_question() const {
	return get_node_true();
}

bool DecisionTreeNode::is_defenition() const {
	return !get_node_true();
}

//=============================================================================
// Tree =======================================================================

DecisionTree::DecisionTree() {
	root = nullptr;
}

DecisionTreeNode *DecisionTree::load_node(File *file) {
	const unsigned char *c = file->cc;
	Char_get_next_symb(&c);
	if (*c != SYMB_QUOTE) {
		printf("Invalid input file\n");
		return nullptr;
	}

	++c;
	String *node_statement = new String();
	c += node_statement->read(c, false, '"');

	++c;
	Char_get_next_symb(&c);
	file->cc = c;

	if (*c == SYMB_OPEN_NODE) {
		DecisionQuestion *question = new DecisionQuestion(node_statement);
		DecisionTreeNode *node = new DecisionTreeNode(question);

		++file->cc;

		node->set_true (load_node(file));
		node->set_false(load_node(file));

		Char_get_next_symb(&file->cc);
		++file->cc;

		return node;
	} else if (*c == SYMB_CLOSE_NODE || *c == SYMB_QUOTE) {
		DecisionDefinition *statement = new DecisionDefinition(node_statement);
		DecisionTreeNode *node = new DecisionTreeNode(statement);

		return node;
	} else {
		printf("Invalid input file\n");
		return nullptr;
	}
}

int DecisionTree::load(const char *file_name) {
	if (file_name == nullptr) {
		printf("nullptr file_name, DB is NOT read\n");
		return -1;
	}

	File file = {};
	if (File_construct(&file, file_name) < 0) {
		printf("File (%s) doesn't exist\n", file_name);
	}

	root = load_node(&file);
	File_destruct(&file);

	return 0;
}

void file_printf_tab(FILE *file_ptr, const int tab) {
	for (int i = 0; i < tab; ++i) {
		fprintf(file_ptr, "    ");
	}
}

int DecisionTree::save_node(const DecisionTreeNode* node, int depth, bool is_true_node, FILE *file_ptr) {
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

int DecisionTree::save(const char *file_name) {
	if (file_name == nullptr) {
		printf("nullptr file_name, DB is NOT saved\n");
		return -1;
	}

	FILE *file = fopen(file_name, "w");
	if (!file) {
		printf("file was not opened, DB is NOT saved\n");
		return -1;
	}

	save_node(root, 0, false, file);

	fclose(file);

	return 0;
}

DecisionTreeNode *DecisionTree::get_root() const {
	return root;
}

void DecisionTree::dump(DecisionTreeNode *node, int depth, int to_format_cnt, int maxlen, FILE *file_ptr) const {
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
	for (int i = 0; i < maxlen - node->statement_length() - 1; ++i) {
		printf("-");
	}
	printf("->|\n");
	dump(node->get_node_false(), depth + 1, to_format_cnt + 1, maxlen, file_ptr);
}

void DecisionTree::dump(FILE *file_ptr) const {
	dump(root, 0, 0, MAX_STATEMENT_LEN, file_ptr);
}

DecisionTreeNode* DecisionTree::merge_node(DecisionTreeNode *first, DecisionTreeNode *second) {
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
			printf("Merge error, aborting...\n");
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
// Tree - interactions ========================================================

int DecisionTree::run_new_node_generation(DecisionTreeNode *cur_node, DecisionTreeNode* prev_node, const int prev_ans) {
	printf("Well, okay...\n");
	printf("What is your object?\n> ");

	char str[MAX_STATEMENT_LEN];
	scanf ("%[^\n]%*c", str);
	String *definition = new String(str);
	DecisionTreeNode *new_defenition_node = new DecisionTreeNode(new DecisionDefinition(definition));

	printf("\nAnd how is [");
	new_defenition_node->dump();
	printf("] different from [");
	cur_node->dump();
	printf("]? It... /*continue the phrase*/\n> ");

	scanf ("%[^\n]%*c", str);
	String *question = new String(str);
	DecisionTreeNode *new_question_node = new DecisionTreeNode(new DecisionQuestion(question));
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

int DecisionTree::run_guess() {
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

bool DecisionTree::node_find_definition_way(const String &definition, const DecisionTreeNode *cur_node, Vector<char> *buffer) {
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

Vector<char> *DecisionTree::find_definition_way(const String &definition) {
	Vector<char> *buffer = new Vector<char>();
	node_find_definition_way(definition, root, buffer);
	return buffer;
}

void DecisionTree::print_prefixed_statement(const String &statement, const bool truth) const {
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

void DecisionTree::print_definition_by_way(const Vector<char> &way, const int min_depth, const int max_depth) const {
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

int DecisionTree::print_definition(const String &definition) {
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

int DecisionTree::run_define() {
	printf("What object do you want me to define?\n> ");
	char str[MAX_STATEMENT_LEN];
	scanf("%[^\n]%*c", str);
	String defenition(str);

	printf("\n");
	print_definition(defenition);
	printf("\n");

	return 0;
}

int DecisionTree::run_difference() {
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

int DecisionTree::run_interaction() {
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
		case 1:
			run_guess();
			break;
		case 2:
			getchar();
			run_define();
			break;
		case 3:
			getchar();
			run_difference();
			break;
		case 4:
			DecisionTree first, second;
			first.load("db1.db");
			second.load("db2.db");

			first.merge(second);

			first.save("db_out.db");
			printf(".doned.\n");
			break;
	}

	return 0;
}

void DecisionTree::merge(const DecisionTree &tree) {
	root = merge_node(root, tree.get_root());
}