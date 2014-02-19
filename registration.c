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
	
	/*read text entry*/
	new_pwd_entry = gtk_entry_get_text(GTK_ENTRY(registrationwindow->new_entry));
	confirm_pwd_entry = gtk_entry_get_text(GTK_ENTRY(registrationwindow->confirm_entry));
	new_ACCN_entry = gtk_entry_get_text(GTK_ENTRY(registrationwindow->ACCN_entry));
	
	if(strcmp(new_pwd_entry,"") && strcmp(new_ACCN_entry, ""))
	{
		if(!strcmp(new_pwd_entry, confirm_pwd_entry))
		{
			uintmax_t ACCN;
			ACCN = strtoumax(new_ACCN_entry, NULL, 10);
			if (ACCN >= 0xFFFFFFFFFFFF)
			{
				error_message("Account ID value error");
				gtk_entry_set_text((GtkEntry *)registrationwindow->new_entry, "");
				gtk_entry_set_text((GtkEntry *)registrationwindow->confirm_entry, "");
				gtk_entry_set_text((GtkEntry *)registrationwindow->ACCN_entry, "");
			}
			else
			{
				if(!create_new_config_file(ACCN, confirm_pwd_entry))
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
	else
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
