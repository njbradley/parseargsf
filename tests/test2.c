#include "../parseargsf.h"

int main(int argc, char** argv) {
	char str1[50], str2[50];
	parseargsf(argc, argv, "x!:%50s y!:%50s", str1, str2);
	printf("%s%s\n", str1, str2);
	return 0;
}
