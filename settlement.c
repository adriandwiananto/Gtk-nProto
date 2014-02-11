#include "header.h"

/*
We call init_settlement_window() when our program is starting to load 
settlement window with references to Glade file. 
*/
gboolean init_settlement_window()
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
	settlementwindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "settlement_window"));
	settlementwindow->total_label = GTK_WIDGET (gtk_builder_get_object (builder, "settlement_balance_label"));

	gtk_builder_connect_signals (builder, settlementwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

/* Callback for Claim button in settlement window */
void on_settlement_claim_button_clicked(GtkButton *button)
{
	
}

/* Callback for Cancel button in settlement window */
void on_settlement_cancel_button_clicked(GtkButton *button)
{
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}

