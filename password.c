#include "header.h"

/*
We call init_pwd_window() when our program is starting to load 
password prompt window with references to Glade file. 
*/
gboolean init_pwd_window()
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
	passwordwindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "pwd_window"));
	passwordwindow->text_entry = GTK_WIDGET (gtk_builder_get_object (builder, "pwd_entry"));

	gtk_builder_connect_signals (builder, passwordwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

/* callback for OK button in password prompt window */
void on_pwd_ok_button_clicked ()
{
	read_pwd_entry();
}

/* callback for Cancel button in password prompt window */
void on_pwd_cancel_button_clicked ()
{
	gtk_main_quit();
}

/* callback for pressing enter in password prompt window text box */
void on_pwd_entry_activate ()
{
	read_pwd_entry();
}

void read_pwd_entry()
{
	const gchar *pwd_entry_text;
	uintmax_t ACCN;

	/*declare and clear array for string container*/
	char hashedpassword[(2*SHA256_DIGEST_LENGTH)+1];
	memset(hashedpassword, 0, (2*SHA256_DIGEST_LENGTH)+1);
	char pwd_in_setting[(2*SHA256_DIGEST_LENGTH)+1];
	memset(pwd_in_setting, 0, (2*SHA256_DIGEST_LENGTH)+1);
	gchar ACCNstr[32];
	memset(ACCNstr, 0, 32);
	
	/*read text entry*/
	pwd_entry_text = gtk_entry_get_text(GTK_ENTRY(passwordwindow->text_entry));
	
	if(!strcmp(pwd_entry_text, ""))	//empty password entry
	{
		/*clear text entry*/
		gtk_entry_set_text((GtkEntry *)passwordwindow->text_entry, "");
	}
	else //password entry not empty
	{
		/*get ACCN from config*/
		if(get_INT64_from_config(&ACCN, "application.ACCN") == TRUE)
		{
			/*convert ACCN from INT64 to string*/
			sprintf(ACCNstr, "%ju", ACCN);
			
			/*hash entered password*/
			passwordhashing(hashedpassword, pwd_entry_text, ACCNstr);
			
			/*compare with password in config*/
			if(get_string_from_config(pwd_in_setting, "application.Pwd") == TRUE)
			{
				/*entered password = stored password in config*/
				if(!strcmp(hashedpassword, pwd_in_setting))
				{
#ifdef DEBUG_MODE
					unsigned char aes_key[KEY_LEN_BYTE];
					memset(aes_key,0,KEY_LEN_BYTE);
					getTransKey(aes_key, pwd_entry_text, ACCNstr, FALSE);
					int i = 0;
					printf("AESKEY!: ");
					for(i = 0; i<KEY_LEN_BYTE;i++)
					{
						printf("%02X ",aes_key[i]);
					}
					printf("\n");
					
					unsigned char logKey[32];
					memset(logKey,0,32);
					if(getLogKey(logKey)==FALSE)fprintf(stderr,"error deriving key\n");
					printf("log key:\n");
					for(i=0;i<32;i++)printf("%02X ", logKey[i]);
					printf("\n");

#endif

					parse_log_file_and_write_to_treeview(1,logNum()); //print all log file to treeview for the first time

					/*switch window to main menu*/
					Bitwise WindowSwitcherFlag;
					f_status_window = FALSE;
					f_mainmenu_window = TRUE;
					WindowSwitcher(WindowSwitcherFlag);
				}
				else
				{
					gchar err_msg[50];
					pass_attempt++;
					gtk_entry_set_text((GtkEntry *)passwordwindow->text_entry, "");
					sprintf(err_msg, "incorrect password! %d of 5 attempt", pass_attempt);
					error_message(err_msg);
					if(pass_attempt >= 5)
					{
						pass_attempt = 0;
						if(!remove("config.cfg"))printf("maximum attempt reached, config file deleted!!\n");
						else printf("error deleting config file");
						gtk_main_quit();
					}
				}
				
			}
			else
			{
				error_message("invalid config files!!");
				gtk_main_quit();
			}
		}
		else
		{
			error_message("invalid config filez!!");
			gtk_main_quit();
		}
	}
}

