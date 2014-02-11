#include "header.h"

/*
Function for switching active window
*/

void WindowSwitcher(gboolean f_password_window, 
					gboolean f_mainmenu_window, 
					gboolean f_newtrans_window,
					gboolean f_history_window,
					gboolean f_settlement_window,
					gboolean f_option_window)
{
	(f_password_window == TRUE)?gtk_widget_show(passwordwindow->window):gtk_widget_hide(passwordwindow->window);
	
	(f_mainmenu_window == TRUE)?gtk_widget_show(mainmenuwindow->window):gtk_widget_hide(mainmenuwindow->window);
	
	if(f_newtrans_window == TRUE)
	{
		gchar SESN_text[4];
		int randomnumber;
		randomnumber = random_number_generator(100,999);
		snprintf(SESN_text,  4, "%d", randomnumber);
		gtk_label_set_text((GtkLabel *)newtranswindow->SESN_label, SESN_text);
		//~ gtk_entry_set_text((GtkEntry *)newtranswindow->text_entry, SESN_text);
		
		gtk_widget_show(newtranswindow->window);
		
		//NFC polling goes in here
	}
	else
	{
		gtk_widget_hide(newtranswindow->window);	
	}
	
	if(f_history_window == TRUE)
	{
		gtk_widget_show(historywindow->window);
	}
	else
	{
		gtk_widget_hide(historywindow->window);	
	}
}
