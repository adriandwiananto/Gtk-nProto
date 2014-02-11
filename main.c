#include "header.h"

#define DECLARE_VARIABLES
	
int main(int argc, char *argv[])
{	
	passwordwindow = g_slice_new(PasswordWindow);
	mainmenuwindow = g_slice_new(MainMenuWindow);
	newtranswindow = g_slice_new(NewTransWindow);
	historywindow = g_slice_new(HistoryWindow);
	
	gtk_init(&argc,&argv);

	if(init_history_window() == FALSE) return 1;
	if(init_newtrans_window() == FALSE) return 1;
	if(init_mainmenu_window() == FALSE) return 1;
	if(init_pwd_window() == FALSE) return 1;
	
	WindowSwitcher(	TRUE,	//password window
					FALSE,	//gboolean f_mainmenu_window, 
					FALSE,	//new trans window
					FALSE,	//history window
					FALSE,	//settlement window
					FALSE);	//option window
	
	gtk_main();

	g_slice_free(PasswordWindow, passwordwindow);
	g_slice_free(MainMenuWindow, mainmenuwindow);
	g_slice_free(NewTransWindow, newtranswindow);
	g_slice_free(HistoryWindow, historywindow);
	
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
        g_warning ("%s",message);
        
        /* create an error message dialog and display modally to the user */
        dialog = gtk_message_dialog_new (NULL, 
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_ERROR,
                                         GTK_BUTTONS_OK,
                                         "%s", message);
        
        gtk_window_set_title (GTK_WINDOW (dialog), "Error!");
        gtk_dialog_run (GTK_DIALOG (dialog));      
        gtk_widget_destroy (dialog);         
}
