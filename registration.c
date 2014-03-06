#include "header.h"

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
				/*hash password and use ACCN as salt*/
				passwordhashing(hashed, confirm_pwd_entry, new_ACCN_entry);
				printf("hashed: %s\n", hashed);

				/*create new config file (with error checking)*/
				if(create_new_config_file(ACCN, (const char *)hashed) == FALSE)
				{
					error_message("Error creating config file");
				}
				else
				{
					notification_message("Registration Success! Restart the application");
					
					/* put registration ACCN to server
					 * get key from server
					 */
					 
					/* starts of create rand dummy key */
					/* THIS IS NOT PERMANENT */
					/* FINAL CODE WILL GET KEY FROM SERVER */
					unsigned char aes_key[KEY_LEN_BYTE];
					RAND_bytes(aes_key, KEY_LEN_BYTE); 
					
					printf("aes key: ");
					int i=0;
					for(i=0;i<KEY_LEN_BYTE;i++)printf("%.02X ",aes_key[i]);
					printf("\n\n");
					/* ends of create rand dummy key */
					
					unsigned char KeyEncryptionKey[KEY_LEN_BYTE];
					unsigned char wrapped_key[KEY_LEN_BYTE+8];
					
					/* derive key from password + ACCN */
					if(derive_key(KeyEncryptionKey, new_pwd_entry, new_ACCN_entry, 10000) == FALSE)
						error_message("KEK derivation error");
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
						error_message("error wrapping key");
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

					gtk_main_quit();
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
