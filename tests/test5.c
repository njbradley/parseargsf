#include "../parseargsf.h"

int main(int argc, char** argv) {
	int x, y;
	parseargsf(argc, argv, "x!:%%%%%i", &x);
	printf("%i\n", x);
	return 0;
}
