#include "header.h"

/*
Function for switching active window
*/

void WindowSwitcher(Bitwise WindowSwitcherFlag)
{
	(f_password_window == TRUE)?gtk_widget_show(passwordwindow->window):gtk_widget_hide(passwordwindow->window);
	
	(f_mainmenu_window == TRUE)?gtk_widget_show(mainmenuwindow->window):gtk_widget_hide(mainmenuwindow->window);
	
	(f_option_window == TRUE)?gtk_widget_show(optionwindow->window):gtk_widget_hide(optionwindow->window);
	
	if(f_newtrans_window == TRUE)
	{
		gchar SESN_text[4];
		int randomnumber;
		randomnumber = random_number_generator(100,999);
		snprintf(SESN_text,  4, "%d", randomnumber);
		gtk_label_set_text((GtkLabel *)newtranswindow->SESN_label, SESN_text);
		
		gtk_widget_show(newtranswindow->window);
		
		//NFC polling goes in here
	}
	else
	{
		gtk_widget_hide(newtranswindow->window);	
	}
	
	if(f_history_window == TRUE)
	{
		parse_log_file();
		gtk_widget_show(historywindow->window);
	}
	else
	{
		gtk_widget_hide(historywindow->window);	
	}

	if(f_settlement_window == TRUE)
	{
		gchar *settlement_balance = "Rp. 50.000";
		gtk_label_set_text((GtkLabel *)settlementwindow->total_label, settlement_balance);
		
		gtk_widget_show(settlementwindow->window);
	}
	else
	{
		gtk_widget_hide(settlementwindow->window);	
	}
}
