#include "header.h"

/* function for generating random number using /dev/random */
int random_number_generator(int min_number, int max_number)
{
	int randomData = open("/dev/random", O_RDONLY);
	int myRandomInteger = max_number + 1;
	ssize_t result;
	
	while(myRandomInteger > max_number)
	{
		myRandomInteger = 0;
		result = read(randomData, (char*)&myRandomInteger, (sizeof myRandomInteger));
		if (result < 0)
		{
			error_message("Failed to read /dev/random");
		}
				
		myRandomInteger %= max_number;
		if(myRandomInteger < min_number)
		{
			myRandomInteger = max_number + 1;
		}
	}
	close(randomData);
	
	return myRandomInteger;
}
