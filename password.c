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
	printf("OK BUTTON CLICKED \n");
	read_pwd_entry();
}

/* callback for Cancel button in password prompt window */
void on_pwd_cancel_button_clicked ()
{
	printf("CANCEL BUTTON CLICKED \n");	
	gtk_main_quit();
}

/* callback for pressing enter in password prompt window text box */
void on_pwd_entry_activate ()
{
	printf("password entered\n");
	read_pwd_entry();
}

void read_pwd_entry()
{
	const gchar *pwd_entry_text;
	
	/*read text entry*/
	pwd_entry_text = gtk_entry_get_text(GTK_ENTRY(passwordwindow->text_entry));
	
	if(!strcmp(pwd_entry_text, ""))	//empty password entry
	{
		/*clear text entry*/
		gtk_entry_set_text((GtkEntry *)passwordwindow->text_entry, "");
	}
	else //password entry not empty
	{
		/*password entry handler goes in here*/
		//~ get_config_entry("application.ACCN");
		//TO DO: 	get ACCN from config
		//			convert ACCN from INT64 to string
		//			hash entered password
		//			compare with password in config
		
		/*switch window to main menu*/
		Bitwise WindowSwitcherFlag;
		f_status_window = FALSE;
		f_mainmenu_window = TRUE;
		WindowSwitcher(WindowSwitcherFlag);
	}
}

