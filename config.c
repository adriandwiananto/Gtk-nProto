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

gboolean create_new_config_file(uintmax_t ACCN, const char* password, char* HWID)
{
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
	
	/*create HWID setting with string type in application group*/
	setting = config_setting_add(group, "HWID", CONFIG_TYPE_STRING);
	config_setting_set_string(setting, HWID);
	
	/*create LATS setting with INT64 type in application group*/
	setting = config_setting_add(group, "LATS", CONFIG_TYPE_INT64);
	config_setting_set_int64(setting, 0);
	
	/*create application group as root group*/
	group = config_setting_add(root, "security", CONFIG_TYPE_GROUP);
	
	/*create ACCN setting with INT64 type in application group*/
	setting = config_setting_add(group, "transaction", CONFIG_TYPE_STRING);
	config_setting_set_string(setting, "EMPTY");
	
	/* Write out the new configuration. */
	if(! config_write_file(&cfg, output_file))
	{
		fprintf(stderr, "Error while writing file.\n");
		config_destroy(&cfg);
		return FALSE;
	}

	fprintf(stdout, "New configuration successfully written to: %s\n", output_file);

	config_destroy(&cfg);
	return TRUE;
}

gboolean get_INT64_from_config(uintmax_t *value, const char *path)
{
	config_t cfg;

	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, "config.cfg"))
	{
		config_destroy(&cfg);
		return FALSE;	//return error
	}

	/* Get ACCN. */
	if(config_lookup_int64(&cfg, path, (long long int *)value))
	{
		config_destroy(&cfg);
		return TRUE;
	}
	else
	{
		fprintf(stderr, "No mentioned setting in configuration file.\n");
		config_destroy(&cfg);
		return FALSE;
	}
}

gboolean get_string_from_config(char *value, const char *path)
{
	const char *str_in_config;
	config_t cfg;

	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, "config.cfg"))
	{
		config_destroy(&cfg);
		return FALSE;	//return error
	}

	/* Get pwd. */
	if(config_lookup_string(&cfg, path, &str_in_config))
	{
		memcpy(value, str_in_config, strlen(str_in_config));
		config_destroy(&cfg);
		return TRUE;
	}
	else
	{
		fprintf(stderr, "No mentioned setting in configuration file.\n");
		config_destroy(&cfg);
		return FALSE;
	}
}

gboolean write_string_to_config(char *value, const char *path)
{
	static const char *output_file = "config.cfg";
	config_t cfg;
	config_setting_t *setting;
	
	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, "config.cfg"))
	{
		config_destroy(&cfg);
		return FALSE;	//return error
	}

	setting = config_lookup(&cfg, path);
	
	/* write string */
	if(config_setting_set_string(setting, value))
	{
		/* Write out the new configuration. */
		if(! config_write_file(&cfg, output_file))
		{
			fprintf(stderr, "Error while writing file.\n");
			config_destroy(&cfg);
			return FALSE;
		}

		fprintf(stdout, "New configuration successfully written to: %s\n", output_file);
			config_destroy(&cfg);
		return FALSE;
	}
	else
	{
		fprintf(stderr, "No mentioned setting in configuration file.\n");
		config_destroy(&cfg);
		return FALSE;
	}
}

gboolean write_int64_to_config(uintmax_t value, const char *path)
{
	static const char *output_file = "config.cfg";
	config_t cfg;
	config_setting_t *setting;
	
	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, "config.cfg"))
	{
		config_destroy(&cfg);
		return FALSE;	//return error
	}

	setting = config_lookup(&cfg, path);
	
	/* write string */
	if(config_setting_set_int64(setting, value))
	{
		/* Write out the new configuration. */
		if(! config_write_file(&cfg, output_file))
		{
			fprintf(stderr, "Error while writing file.\n");
			config_destroy(&cfg);
			return FALSE;
		}

		fprintf(stdout, "New configuration successfully written to: %s\n", output_file);
			config_destroy(&cfg);
		return FALSE;
	}
	else
	{
		fprintf(stderr, "No mentioned setting in configuration file.\n");
		config_destroy(&cfg);
		return FALSE;
	}
}
