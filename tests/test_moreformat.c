#include "../parseargsf.h"

void runcomps() {
	printf("Soy un metodo que hace computos\n");
}

int main(int argc, char** argv) {
	int x, y;
	parseargsf(argc, argv,
		"-runcomps() #Un metodo que hace computos# ..", &runcomps,
		"x!:%i #Un entero que me das# ..", &x,
		"y!:%i #Otro entero que me das#", &y
	);
	printf("%i %i\n", x, y);
	return 0;
}
