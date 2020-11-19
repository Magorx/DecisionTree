#include "decision_tree.h"

int main() {
	DecisionTree tree;
	tree.load("akinator.db");
	tree.save("akinator_runtime_copy.db");
	tree.dump();

	tree.run_interaction();
	tree.save("akinator.db");

	return 0;
}
