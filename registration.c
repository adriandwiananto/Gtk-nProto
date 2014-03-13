#include "header.h"

/* abort registration, delete config and log file */
static void abort_registration()
{
	if(remove("config.cfg") == 0)
		printf("config.cfg deleted!\n");
	else
		printf("config.cfg not exists\n");
		
	if(remove("log.db") == 0)
		printf("log.db deleted!\n");
	else
		printf("log.db not exists\n");
	
	error_message("Registration failed! Please retry registration process");
	exit(1);
}

static gboolean send_regData_get_aesKey(unsigned char* aesKey, const gchar* ACCN, char* HWID)
{
	json_object *jobj = create_registration_json(ACCN,HWID);
	gchar aesKeyString[65];
	memset(aesKeyString,0,65);
	
	printf("json object in string: %s\n",json_object_to_json_string(jobj));
	
	if(send_jsonstring_to_server	(aesKeyString, 
									json_object_to_json_string(jobj), 
									"http://emoney-server.herokuapp.com/sync.json") == FALSE)
		return FALSE;
	
	hexstrToBinArr(aesKey, aesKeyString, 32);
	
#ifdef DEBUG_MODE
	printf("aeskey in string: %s\n", aesKeyString);

	int i;
	printf("aes key array from json response:\n");
	for(i=0;i<32;i++)
	{
		printf("%02X ", aesKey[i]);
	}
	printf("\n");
#endif

	return TRUE;
}

/*
We call init_registration_window() when our program is starting to load 
settlement window with references to Glade file. 
*/
gboolean init_registration_window()
{
	GtkBuilder              *builder;
	GError                  *err=NULL;

	/* use GtkBuilder to build our interface from the XML file */
	builder = gtk_builder_new ();
	if (gtk_builder_add_from_file (builder, UI_GLADE_FILE, &err) == 0)
	{
		error_message (err->message);
		g_error_free (err);
		return FALSE;
	}

	/* get the widgets which will be referenced in callbacks */
	registrationwindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "registration_window"));
	registrationwindow->ACCN_entry = GTK_WIDGET (gtk_builder_get_object (builder, "registration_ACCN_entry"));
	registrationwindow->new_entry = GTK_WIDGET (gtk_builder_get_object (builder, "registration_new_entry"));
	registrationwindow->confirm_entry = GTK_WIDGET (gtk_builder_get_object (builder, "registration_confirm_entry"));

	gtk_builder_connect_signals (builder, registrationwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

void on_registration_ACCN_entry_insert_text(GtkEditable *buffer, gchar *new_text, gint new_text_length, gint *position, gpointer data)
{
	int i;
	guint sigid;

	/* Only allow 0-9 to be written to the entry */
	for (i = 0; i < new_text_length; i++) {
		if (new_text[i] < '0' || new_text[i] > '9') {
			sigid = g_signal_lookup("insert-text",
						G_OBJECT_TYPE(buffer));
			g_signal_stop_emission(buffer, sigid, 0);
			return;
		}
	}
}

/* Callback for Request button in registration window */
void on_registration_request_button_clicked()
{
	const gchar *new_pwd_entry, *confirm_pwd_entry, *new_ACCN_entry;
	
	/* hashed password+salt written in hex as string
	 * array size must be (2*hash_length)+1
	 * every byte is represented with 2 character hence 2*hash_length
	 * +1 is for null
	 */ 
	char hashed[(SHA256_DIGEST_LENGTH*2)+1];
	
	/*read text entry*/
	new_pwd_entry = gtk_entry_get_text(GTK_ENTRY(registrationwindow->new_entry));
	confirm_pwd_entry = gtk_entry_get_text(GTK_ENTRY(registrationwindow->confirm_entry));
	new_ACCN_entry = gtk_entry_get_text(GTK_ENTRY(registrationwindow->ACCN_entry));
	
	/*make sure text entry is not empty*/
	if(strcmp(new_pwd_entry,"") && strcmp(confirm_pwd_entry,"") && strcmp(new_ACCN_entry, ""))
	{
		/*make sure new password and confirmed password is same*/
		if(!strcmp(new_pwd_entry, confirm_pwd_entry))
		{
			/*convert ACCN type from string to long int (64 byte)*/
			uintmax_t ACCN;
			ACCN = strtoumax(new_ACCN_entry, NULL, 10);
			
			/*make sure ACCN value is not greater than maximum of 6 bytes value*/
			if (ACCN >= 0xFFFFFFFFFFFF)
			{
				error_message("Account ID value error");
				gtk_entry_set_text((GtkEntry *)registrationwindow->new_entry, "");
				gtk_entry_set_text((GtkEntry *)registrationwindow->confirm_entry, "");
				gtk_entry_set_text((GtkEntry *)registrationwindow->ACCN_entry, "");
			}
			else /*user input valid data to all text entry*/
			{
				char HWID[16];
				memset(HWID,0,16);
				if(get_USB_reader_HWID(HWID) == TRUE)
				{
					/*hash password and use ACCN as salt*/
					passwordhashing(hashed, confirm_pwd_entry, new_ACCN_entry);
					printf("hashed: %s\n", hashed);

					unsigned char aes_key[KEY_LEN_BYTE];
					memset(aes_key,0,KEY_LEN_BYTE);

					if(send_regData_get_aesKey(aes_key, new_ACCN_entry, HWID) == FALSE)
						abort_registration();
						
					/*create new config file (with error checking)*/
					if(create_new_config_file(ACCN, (const char *)hashed, HWID) == FALSE)
					{
						error_message("Error creating config file");
						abort_registration();
					}

#ifdef DEBUG_MODE					
					printf("aes key: ");
					int i=0;
					for(i=0;i<KEY_LEN_BYTE;i++)printf("%.02X ",aes_key[i]);
					printf("\n\n");
#endif

					unsigned char KeyEncryptionKey[KEY_LEN_BYTE];
					unsigned char wrapped_key[KEY_LEN_BYTE+8];
					
					/* derive key from password + ACCN */
					if(derive_key(KeyEncryptionKey, new_pwd_entry, new_ACCN_entry, 10000) == FALSE)
					{
						error_message("KEK derivation error");
						abort_registration();
					}
#ifdef DEBUG_MODE
					else
					{
						printf("derived key: ");
						int i=0;
						for(i=0;i<KEY_LEN_BYTE;i++)printf("%.02X ",KeyEncryptionKey[i]);
						printf("\n\n");
					}
#endif

					/* wrap key using KEK */
					if(wrap_aes_key(wrapped_key, KeyEncryptionKey, aes_key) == FALSE)
					{
						error_message("error wrapping key");
						abort_registration();
					}
#ifdef DEBUG_MODE
					else
					{
						printf("wrapped key: ");
						int i=0;
						for(i=0;i<KEY_LEN_BYTE+8;i++)printf("%.02X ",wrapped_key[i]);
						printf("\n\n");
					}
#endif

					/* convert wrapped key to base 64 and write to config */
					char *wrapped_base64 = base64(wrapped_key, KEY_LEN_BYTE+8);
					printf("wrapped key to write: %s\n\n",wrapped_base64);
					write_string_to_config(wrapped_base64,"security.transaction");
					
#ifdef DEBUG_MODE					
					get_string_from_config(wrapped_base64,"security.transaction");
					unsigned char *wrapped_unbase64 = (unsigned char *) unbase64((unsigned char *)wrapped_base64, strlen(wrapped_base64)+1);

					printf("wrapped unbase64 key: ");
					for(i=0;i<KEY_LEN_BYTE+8;i++)printf("%.02X ",*(wrapped_unbase64+i));
					printf("\n\n");
					
					/* unwrap key using KEK */
					if(unwrap_aes_key(aes_key, KeyEncryptionKey, (unsigned char *)wrapped_unbase64) == FALSE)
						error_message("error unwrapping key");
					else
					{
						printf("wrapped key: ");
						int i=0;
						for(i=0;i<KEY_LEN_BYTE;i++)printf("%.02X ",aes_key[i]);
						printf("\n\n");
					}
#endif

					if(createDB_and_table() == FALSE)abort_registration();

					notification_message("Registration Success! Restart the application");

					gtk_main_quit();
				}
				else
				{
					error_message("Connect USB reader!");
				}
			}
		}

	}
	else /*if one of the text entry is empty, clear both password entry*/
	{
		gtk_entry_set_text((GtkEntry *)registrationwindow->new_entry, "");
		gtk_entry_set_text((GtkEntry *)registrationwindow->confirm_entry, "");
	}
}

/* Callback for Cancel button in registration window */
void on_registration_cancel_button_clicked()
{
	gtk_main_quit();
}

gboolean get_USB_reader_HWID (char* hwid)
{
	FILE *p;
	char popenData[32];
	memset(popenData,0,32);
	char detectSTR[6];
	memset(detectSTR,0,6);

	p = popen("./picc_emulation_write hwid", "r");

	if(!p) 
	{
		fprintf(stderr, "Error opening pipe.\n");
		return FALSE;
	}

	while(!feof(p)) 
	{
		fgets(popenData, 32 , p);
		memcpy(detectSTR,popenData,5);
		if(!strcmp(detectSTR,"DATA:"))
			//strlen(data)-6 is 5 for DATA: and 1 for \n
			memcpy(hwid,popenData+5,(strlen(popenData)-6));
	}

	if(!strcmp(hwid,""))
		return FALSE;
#ifdef DEBUG_MODE
	printf("HWID:%s\n",hwid);
#endif

	if (pclose(p) == -1) 
	{
		fprintf(stderr,"Error!\n");
		return FALSE;
	}
	
	return TRUE;
}
