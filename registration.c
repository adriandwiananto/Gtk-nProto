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
				if(!create_new_config_file(ACCN, (const char *)hashed))
				{
					error_message("Error creating config file");
				}
				else
				{
					notification_message("Registration Success! Restart the application");
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
