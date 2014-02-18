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

/* Callback for Request button in registration window */
void on_registration_request_button_clicked(GtkButton *button)
{
	const gchar *new_pwd_entry, *confirm_pwd_entry, *new_ACCN_entry;

	/*read text entry*/
	new_pwd_entry = gtk_entry_get_text(GTK_ENTRY(registrationwindow->new_entry));
	confirm_pwd_entry = gtk_entry_get_text(GTK_ENTRY(registrationwindow->confirm_entry));

	if(strcmp(new_pwd_entry,""))
	{
		if(!strcmp(new_pwd_entry, confirm_pwd_entry))
		{
			new_ACCN_entry = gtk_entry_get_text(GTK_ENTRY(registrationwindow->ACCN_entry));
			//~ create_new_config_file();
			notification_message("Registration Success! Restart the application");
			gtk_main_quit();
		}

	}
	else
	{
		gtk_entry_set_text((GtkEntry *)registrationwindow->new_entry, "");
		gtk_entry_set_text((GtkEntry *)registrationwindow->confirm_entry, "");
	}
}

/* Callback for Cancel button in registration window */
void on_registration_cancel_button_clicked(GtkButton *button)
{
	gtk_main_quit();
}
