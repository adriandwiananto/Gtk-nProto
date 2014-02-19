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
	//~ printf("ACCN: %ju\n",ACCN);
	//~ printf("password: %s\n",password);
	
	config_t cfg;
	config_setting_t *root, *group, *setting;
	static const char *output_file = "config.cfg";
	
	/*libconfig init*/
	config_init(&cfg);
	root = config_root_setting(&cfg);
	
	/*create application group as root group*/
	group = config_setting_add(root, "application", CONFIG_TYPE_GROUP);
	
	/*create ACCN setting with INT64 type in application group*/
	setting = config_setting_add(group, "ACCN", CONFIG_TYPE_INT64);
	config_setting_set_int64(setting, ACCN);
	
	/*create Password setting with string type in application group*/
	setting = config_setting_add(group, "Pwd", CONFIG_TYPE_STRING);
	config_setting_set_string(setting, password);
	
	
	/* Write out the new configuration. */
	if(! config_write_file(&cfg, output_file))
	{
		fprintf(stderr, "Error while writing file.\n");
		config_destroy(&cfg);
		return 0;
	}

	fprintf(stderr, "New configuration successfully written to: %s\n", output_file);

	config_destroy(&cfg);
	return 1;
}


