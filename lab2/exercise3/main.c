#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define FREQUENCY 5
#define	MIN -10
#define MAX 35


int main() {
	

	time_t t = time(&t);
	struct tm tm = *localtime(&t);
	srand(time(NULL));
	

	//printf("Temperature = %")

	while(1) {
  	t = time(&t);
  	tm = *localtime(&t);
  	
  	printf("Temperature = %1.2f @ %02d/%02d/%d %02d:%02d:%02d\n", (float)(rand()%((MAX - MIN)*100))/100 + MIN, tm.tm_mday, 
  		tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
  	fflush( stdout );
  	sleep(FREQUENCY);
  	}
}