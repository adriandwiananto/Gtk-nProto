#include <gtk/gtk.h>
#include "header.h"

/*
We call init_pwd_window() when our program is starting to load 
password prompt window with references to Glade file. 
*/
//gboolean init_pwd_window(Windows *windows_obj)
gboolean init_pwd_window(PasswordWindow *passwordwindow)
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
	//~ passwordwindow->button1 = GTK_WIDGET (gtk_builder_get_object (builder, "pwd_ok_button"));
	//~ passwordwindow->button2 = GTK_WIDGET (gtk_builder_get_object (builder, "pwd_cancel_button"));

	gtk_builder_connect_signals (builder, passwordwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

void on_pwd_ok_button_clicked (GtkButton *button, PasswordWindow *passwordwindow)
//~ void on_pwd_ok_button_clicked (GtkButton *button, Windows *windows_obj)
{
	printf("OK BUTTON CLICKED \n");
	read_entry(passwordwindow);
}

void on_pwd_cancel_button_clicked (GtkButton *button, PasswordWindow *passwordwindow)
//~ void on_pwd_cancel_button_clicked (GtkButton *button, Windows *windows_obj)
{
	printf("CANCEL BUTTON CLICKED \n");	
	gtk_main_quit();
}

void on_pwd_entry_activate (GtkButton *button, PasswordWindow *passwordwindow)
//~ void on_pwd_entry_activate (GtkButton *button, Windows *windows_obj)
{
	printf("password entered\n");
	read_entry(passwordwindow);
}

//~ void read_entry(Windows *windows_obj)
void read_entry(PasswordWindow *passwordwindow)
{
	const gchar *pwd_entry_text;
	/*read text entry*/
	pwd_entry_text = gtk_entry_get_text(GTK_ENTRY(passwordwindow->text_entry));
	printf("Password: %s\n", pwd_entry_text);
	
	if(!strcmp(pwd_entry_text, ""))	//empty password, need another input attempt
	{
		/*clear text entry*/
		printf("Password EMPTY\n");
		gtk_entry_set_text((GtkEntry *)passwordwindow->text_entry, "");
	}
	else //assuming entered password is correct
	{
		/*password entry handler goes in here*/
		//show_mainmenu(windows_obj);
		printf("Password NOT EMPTY\n");
		//~ gtk_entry_set_text((GtkEntry *)passwordwindow->text_entry, "");
		gtk_widget_hide(passwordwindow->window);
		gtk_widget_show(mainmenuwindow->window);
	}
}

