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
	settlementwindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "settlement_window"));
	settlementwindow->total_label = GTK_WIDGET (gtk_builder_get_object (builder, "settlement_balance_label"));

	gtk_builder_connect_signals (builder, settlementwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

/* Callback for Request button in registration window */
void on_registration_request_button_clicked(GtkButton *button)
{
	
}

/* Callback for Cancel button in registration window */
void on_registration_cancel_button(GtkButton *button)
{
	gtk_main_quit();
}
