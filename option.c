#include "header.h"

/*
We call init_option_window() when our program is starting to load 
option window with references to Glade file. 
*/
gboolean init_option_window()
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
	optionwindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "option_window"));
	optionwindow->old_entry = GTK_WIDGET (gtk_builder_get_object (builder, "option_old_entry"));
	optionwindow->new_entry = GTK_WIDGET (gtk_builder_get_object (builder, "option_new_entry"));
	optionwindow->confirm_entry = GTK_WIDGET (gtk_builder_get_object (builder, "option_confirm_entry"));

	gtk_builder_connect_signals (builder, optionwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

/* Callback for Ok button in option window */
void on_option_ok_button_clicked()
{
	
}

/* Callback for Cancel button in option window */
void on_option_cancel_button_clicked()
{
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}
