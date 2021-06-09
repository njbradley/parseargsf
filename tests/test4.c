#include "parseargsf.h"
#include <time.h>

typedef struct date_t {
	int month;
	int day;
	int year;
} date_t;

typedef struct timepoint_t {
	int hour;
	int minute;
	int second;
} timepoint_t;

int main(int argc, char** argv) {
	date_t date;
	timepoint_t timepoint;
	int start = clock();
	parseargsf(argc, argv,
		"date!:%i/%i/%i "
		"time:%i:%i:%i ",
		&date.month, &date.day, &date.year,
		&timepoint.hour, &timepoint.minute, &timepoint.second
	);
	
	int totaldays = date.year * 365 + date.month * 30 + date.day;
	int totalseconds = timepoint.hour * 3600 + timepoint.minute * 60 + timepoint.second;
	
	printf("%i %i\n", totaldays, totalseconds);
	return 0;
}
