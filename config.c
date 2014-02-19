#include "header.h"
#include <libconfig.h>

int config_checking()
{
	config_t cfg;
	//~ config_setting_t *setting;
	//~ const char *str;

	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, "config.cfg"))
	{
		config_destroy(&cfg);
		return 2;
	}
	else
	{
		//config file for tamper checking goes here (compare hash value of redundant config files)
		int tamper = 0;
		if(tamper)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}
}	

int create_new_config_file(uintmax_t ACCN, const char* password)
{
	printf("ACCN: %ju\n",ACCN);
	printf("password: %s\n",password);
	return 1;
}


