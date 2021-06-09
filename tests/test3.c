#include "parseargsf.h"

void func1() {
	printf("func1\n");
}

void func2() {
	printf("func2\n");
}

int main(int argc, char** argv) {
	parseargsf(argc, argv, "-func1() -func2()", func1, func2);
	return 0;
}
