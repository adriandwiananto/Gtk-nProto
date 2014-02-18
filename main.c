#include "header.h"

#define DECLARE_VARIABLES
	
int main(int argc, char *argv[])
{	
	Bitwise WindowSwitcherFlag;
	int config_status = 0; // -1 = config tampered!, 1 = config ok, 2 = config empty
	
	passwordwindow = g_slice_new(PasswordWindow);
	mainmenuwindow = g_slice_new(MainMenuWindow);
	newtranswindow = g_slice_new(NewTransWindow);
	historywindow = g_slice_new(HistoryWindow);
	settlementwindow = g_slice_new(SettlementWindow);
	optionwindow = g_slice_new(OptionWindow);
	registrationwindow = g_slice_new(RegistrationWindow);
	
	config_status = config_checking();
	
	gtk_init(&argc,&argv);

	if(init_registration_window() == FALSE) return 1;
	if(init_option_window() == FALSE) return 1;
	if(init_settlement_window() == FALSE) return 1;
	if(init_history_window() == FALSE) return 1;
	if(init_newtrans_window() == FALSE) return 1;
	if(init_mainmenu_window() == FALSE) return 1;
	if(init_pwd_window() == FALSE) return 1;
	
	switch(config_status)
	{
		case -1:
			error_message("unauthorized config file modification is detected!!");
			break;
		case 1:
			f_status_window = FALSE;	//hide all window
			f_password_window = TRUE;	//show password window
			WindowSwitcher(WindowSwitcherFlag);
			break;
		case 2:
			printf("no config file, registration process called\n");
			f_status_window = FALSE;		//hide all window
			f_registration_window = TRUE;	//show registration window
			WindowSwitcher(WindowSwitcherFlag);
			break;
	}
			
	gtk_main();

	g_slice_free(PasswordWindow, passwordwindow);
	g_slice_free(MainMenuWindow, mainmenuwindow);
	g_slice_free(NewTransWindow, newtranswindow);
	g_slice_free(HistoryWindow, historywindow);
	g_slice_free(SettlementWindow, settlementwindow);
	g_slice_free(OptionWindow, optionwindow);
	g_slice_free(RegistrationWindow, registrationwindow);
	
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

/*
We call notification_message() any time we want to display a notification to the
user.
*/
void
notification_message (const gchar *message)
{
        GtkWidget               *dialog;
        
        /* log to terminal window */
        //~ g_warning ("%s",message);
        
        /* create an error message dialog and display modally to the user */
        dialog = gtk_message_dialog_new (NULL, 
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_INFO,
                                         GTK_BUTTONS_CLOSE,
                                         "%s", message);
        
        gtk_window_set_title (GTK_WINDOW (dialog), "Notification");
        gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy (dialog);
        //~ gtk_main_quit();
}
