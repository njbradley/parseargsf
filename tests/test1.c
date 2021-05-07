#include "../parseargsf.h"

int main(int argc, char** argv) {
	int x, y;
	parseargsf(argc, argv, "x!:%i y!:%i", &x, &y);
	printf("%i\n", x * y);
	return 0;
}
