#include <gtk/gtk.h>
#include "header.h"

#define DEFINE_VARIABLES
//~ PasswordWindow *passwordwindow;
//~ MainMenuWindow *mainmenuwindow;
	
int main(int argc, char *argv[])
{	
	//~ PasswordWindow *passwordwindow;
	//~ MainMenuWindow *mainmenuwindow;
	
	passwordwindow = g_slice_new(PasswordWindow);
	mainmenuwindow = g_slice_new(MainMenuWindow);
	
	//~ Windows *windows_obj;
	//~ windows_obj = g_slice_new(Windows);
	
	gtk_init(&argc,&argv);

	if(init_mainmenu_window(mainmenuwindow) == FALSE) return 1;
	gtk_widget_hide(mainmenuwindow->window);
		
	if(init_pwd_window(passwordwindow) == FALSE) return 1;
	gtk_widget_show(passwordwindow->window);
	
	gtk_main();

	//~ g_slice_free(Windows, windows_obj);
	g_slice_free(PasswordWindow, passwordwindow);
	g_slice_free(MainMenuWindow, mainmenuwindow);

	return 0;
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
