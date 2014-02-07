#include <gtk/gtk.h>
#include "header.h"

/*
We call init_pwd_window() when our program is starting to load 
password prompt window with references to Glade file. 
*/
gboolean init_pwd_window(PasswordWindow *passwordwindow)
{
	GtkBuilder              *builder;
	GError                  *err=NULL;

	/* use GtkBuilder to build our interface from the XML file */
	builder = gtk_builder_new ();
	if (gtk_builder_add_from_file (builder, PASSWORD_GLADE_FILE, &err) == 0)
	{
		error_message (err->message);
		g_error_free (err);
		return FALSE;
	}

	/* get the widgets which will be referenced in callbacks */
	passwordwindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "pwd_window"));
	passwordwindow->text_entry = GTK_WIDGET (gtk_builder_get_object (builder, "pwd_entry"));
	passwordwindow->button1 = GTK_WIDGET (gtk_builder_get_object (builder, "pwd_ok_button"));
	passwordwindow->button2 = GTK_WIDGET (gtk_builder_get_object (builder, "pwd_cancel_button"));

	gtk_builder_connect_signals (builder, passwordwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

/*
We call error_message() any time we want to display an error message to the
user. It will both show an error dialog and log the error to the terminal
window.
*/
void
error_message (const gchar *message)
{
        GtkWidget               *dialog;
        
        /* log to terminal window */
        g_warning (message);
        
        /* create an error message dialog and display modally to the user */
        dialog = gtk_message_dialog_new (NULL, 
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_ERROR,
                                         GTK_BUTTONS_OK,
                                         message);
        
        gtk_window_set_title (GTK_WINDOW (dialog), "Error!");
        gtk_dialog_run (GTK_DIALOG (dialog));      
        gtk_widget_destroy (dialog);         
}

void on_pwd_ok_button_clicked (GtkButton *button, PasswordWindow *passwordwindow)
{
	printf("OK BUTTON CLICKED \n");
	read_entry(passwordwindow);
}

void on_pwd_cancel_button_clicked (GtkButton *button, PasswordWindow *passwordwindow)
{
	printf("CANCEL BUTTON CLICKED \n");	
	gtk_main_quit();
}

void on_pwd_entry_activate (GtkButton *button, PasswordWindow *passwordwindow)
{
	printf("password entered\n");
	read_entry(passwordwindow);
}

void read_entry(PasswordWindow *passwordwindow)
{
	const gchar *pwd_entry_text;
	pwd_entry_text = gtk_entry_get_text(GTK_ENTRY(passwordwindow->text_entry));
	printf("Password: %s\n", pwd_entry_text);
	
	gtk_entry_set_text((GtkEntry *)passwordwindow->text_entry, "");
	
}
