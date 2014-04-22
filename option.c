#include "header.h"

static gboolean change_password_and_all_encryption(const gchar* old_pwd_entry, const gchar* new_pwd_entry);

/*
We call init_option_window() when our program is starting to load 
option window with references to Glade file. 
*/
gboolean init_option_window()
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
	optionwindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "option_window"));
	optionwindow->old_entry = GTK_WIDGET (gtk_builder_get_object (builder, "option_old_entry"));
	optionwindow->new_entry = GTK_WIDGET (gtk_builder_get_object (builder, "option_new_entry"));
	optionwindow->confirm_entry = GTK_WIDGET (gtk_builder_get_object (builder, "option_confirm_entry"));

	gtk_builder_connect_signals (builder, optionwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

/* Callback for Ok button in option window */
void on_option_ok_button_clicked()
{
	const gchar *new_pwd_entry, *confirm_pwd_entry, *old_pwd_entry, *old_pwd_in_passwordwindow;

	/*read text entry*/
	old_pwd_in_passwordwindow = gtk_entry_get_text(GTK_ENTRY(passwordwindow->text_entry));
	old_pwd_entry = gtk_entry_get_text(GTK_ENTRY(optionwindow->old_entry));
	new_pwd_entry = gtk_entry_get_text(GTK_ENTRY(optionwindow->new_entry));
	confirm_pwd_entry = gtk_entry_get_text(GTK_ENTRY(optionwindow->confirm_entry));

	/*make sure text entry is not empty*/
	if(strcmp(new_pwd_entry,"") && strcmp(confirm_pwd_entry,"") && strcmp(old_pwd_entry, ""))
	{
		/*make sure entered old password same with current password*/
		if(!strcmp(old_pwd_entry, old_pwd_in_passwordwindow))
		{
			/*make sure new password and confirmed password is same*/
			if(!strcmp(new_pwd_entry, confirm_pwd_entry))
			{
				if(change_password_and_all_encryption(old_pwd_entry,new_pwd_entry) == FALSE)
				{
					error_message("failed to change password");
					gtk_main_quit();
				}
				else
				{
					notification_message("Password Changed! Restart application!");
					gtk_main_quit();
				}
			}
			else
				error_message("new password and confirm password field must be same");
		}
		else
			error_message("wrong password!");
	}
	else
		error_message("Field can not empty");
}

/* Callback for Cancel button in option window */
void on_option_cancel_button_clicked()
{
	/*open main menu window only*/
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}

static gboolean change_password_and_all_encryption(const gchar* old_pwd_entry, const gchar* new_pwd_entry)
{
	int i=0;
	char hashed[(SHA256_DIGEST_LENGTH*2)+1];
	
	//~ uintmax_t ACCN;
	gchar ACCNstr[32];
	//~ ACCN = get_ACCN(ACCNstr);
	get_ACCN(ACCNstr);
	
	/*=================*/
	/*hash new password*/
	/*=================*/
	passwordhashing(hashed, new_pwd_entry, ACCNstr);
	
	/*==================*/
	/*change key wrapper*/
	/*==================*/
	unsigned char aes_key[KEY_LEN_BYTE];
	memset(aes_key,0,KEY_LEN_BYTE);
	getTransKey(aes_key, old_pwd_entry, ACCNstr, FALSE);
	
	unsigned char KeyEncryptionKey[KEY_LEN_BYTE];
	unsigned char wrapped_key[KEY_LEN_BYTE+8];
	
	/* derive key from password + ACCN */
	if(derive_key(KeyEncryptionKey, new_pwd_entry, ACCNstr, 10000) == FALSE)
	{
		error_message("KEK derivation error");
		return FALSE;
	}
#ifdef DEBUG_MODE
	print_array_inHex("derived key:", KeyEncryptionKey, KEY_LEN_BYTE);
#endif

	/* wrap key using KEK */
	if(wrap_aes_key(wrapped_key, KeyEncryptionKey, aes_key) == FALSE)
	{
		error_message("error wrapping key");
		return FALSE;
	}
#ifdef DEBUG_MODE
	print_array_inHex("wrapped key:", wrapped_key, KEY_LEN_BYTE+8);
#endif

	/*====================*/
	/*change encrypted log*/
	/*====================*/
	int logged_transaction = logNum();
	if(logged_transaction)
	{
		int logLen;
		unsigned char oldLogKey[32];
		memset(oldLogKey,0,32);
		unsigned char newLogKey[32];
		memset(newLogKey,0,32);
		
		unsigned char logToWrite[logged_transaction][48];
		memset(logToWrite,0,sizeof(logToWrite));

		if(derive_key(oldLogKey, old_pwd_entry, ACCNstr, 9000)==FALSE)
		{
			error_message("error deriving old log key");
			return FALSE;
		}
		if(derive_key(newLogKey, new_pwd_entry, ACCNstr, 9000)==FALSE)
		{
			error_message("error deriving new log key");
			return FALSE;
		}
		
		for(i=1;i<=logged_transaction;i++)
		{
			unsigned char fromDB[96];
			memset(fromDB,0,96);
			logLen = read_log_blob(fromDB,i);
			if(logLen == 96)
			{
				unsigned char fromDBbyte[48];
				memset(fromDBbyte,0,48);
				
				hexstrToBinArr(fromDBbyte,(gchar*)fromDB,48);
				
				unsigned char logDecrypted[32];
				memset(logDecrypted,0,32);

				unsigned char IV[16];
				memset(IV,0,16);
				memcpy(IV, fromDBbyte+32, 16);	
				
				aes256cbc(logDecrypted, fromDBbyte, oldLogKey, IV, "DECRYPT");
				
				unsigned char logEncrypted[32];
				memset(logEncrypted,0,32);
				
				unsigned char newIV[16]; //, iv_dec[AES_BLOCK_SIZE];
				RAND_bytes(newIV, 16);
				memcpy((logToWrite[i-1])+32,newIV,16);

				aes256cbc(logEncrypted, logDecrypted, newLogKey, newIV, "ENCRYPT");
				
				memcpy(logToWrite[i-1],logEncrypted,32);
#ifdef DEBUG_MODE
				print_array_inHex("old log:", fromDBbyte, 48);
				print_array_inHex("new log:", logToWrite[i-1], 48);
#endif
			}
			else return FALSE;
		}
		
		/*=================*/
		/* Write log to DB */
		/*=================*/
		for(i=0;i<logged_transaction;i++)
		{
			int j = 0;
			char logToWriteInStr[97];
			memset(logToWriteInStr,0,97); //NULL in index 96
			for(j = 0; j<48; j++)
			{
				sprintf(&logToWriteInStr[j*2], "%02X", logToWrite[i][j]);
			}
#ifdef DEBUG_MODE			
			printf("new log %d: %s\n",i+1,logToWriteInStr);
#endif
			update_encrypted_log(logToWriteInStr, i+1);
		}
	}
	
	/*=======================================*/
	/* Write trans key to config (in Base64) */
	/*=======================================*/
	char *wrapped_base64 = base64(wrapped_key, KEY_LEN_BYTE+8);
	printf("wrapped key to write: %s\n\n",wrapped_base64);
	write_string_to_config(wrapped_base64,"security.transaction");
	
	/*=================================*/
	/* Write hashed password to config */
	/*=================================*/
	write_string_to_config(hashed,"application.Pwd");
	
	return TRUE;
}
