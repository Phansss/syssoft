#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//Max string length equals MAX-1:
const int MAX = 3;



int main() {
	
	char first[MAX];
	char second[MAX];
	char name[MAX];
	char str[MAX];
	int year;


	//for-loop variable.
	int i;
	//comparison variable for second and str.
	int vgl;

	
	


	printf("Enter first name, press enter. Then enter second name and press enter again: ");
	scanf("%s %s", first, second);
	printf("Enter value for str: ");
	scanf("%s", str);
	printf("\n\n\n");
	//Check whether the input strings do not exceed the maximum length. 
	if (strlen(first) > MAX || strlen(second) > MAX || strlen(str) > MAX) {
		printf("String lengths may not be longer than %d.", MAX);
		return 0;
	}

	printf("Entered names are %s and %s\n", first, second);
	printf("Entered str is %s\n\n", str);
	
	//convert second name to all upper case chars.
	for (i = 0; i < strlen(second); i++) {
		if (96 < second[i] && second[i] < 123 ) {
			str[i] = second[i] - 32;
		}
		else {
			str[i] = second[i];
		}
		
		
	}

	printf("String 'second' in capitals equals %s\n", str);

	//Apply strcmp on 'second' and 'str'.
	vgl = strcmp(second, str);

	if (vgl > 0) {
		printf ("String '%s' is more than string '%s'.\n", second, str);
	}
	else if (vgl < 0) {
		printf ("String '%s' is more than string '%s'.\n", str, second);
	}
	else if (vgl == 0) {
		printf ("String '%s' equals string '%s'.\n\n", second, str);
	}

	/*printf("NOTE: Comparing two strings directly using boolean operators is not useful in C since the base addresses of the string arrays and not the contents are compared against each other.\n");
	printf("Current value of string 'second': %s. \nCurrent Value of string 'str':  %s. \n", second, str);
	if (second > str) {
		printf ("second > str: %d\n", second > str);
	}
	if (second < str) {
		printf ("second < str: %d\n", second < str);
	}
	if (second == str) {
		printf ("second == str %d\n", second == str);
	}
	*/

	
	//Concatenate 'first' and 'second' into 'name'
	strncpy(name, first, MAX - 1);
	strncat(name, second, MAX - strlen(first) -1 );
	
	printf("Concatenate strings 'first' and 'second': %s\n", name);

	 //Ask for year of birth
	printf("Enter your year of birth: \n");
	scanf("%d", &year);

	//Concatenate 'first', 'second' and 'year'
	for(i=0; i<MAX; i++)
    	memset(&name,'\0', strlen(name));

	snprintf(name, MAX, "%s %s %d",first, second, year);

	printf("Value of name at the end is %s\n", name);
	//check name value by scanf
	sscanf(name,"%s %s %d", first, second, &year);
	printf("First name: %s\nSecond name: %s\nDate of birth: %d\n", first, second, year);

}
