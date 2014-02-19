#include "header.h"

/*
We call init_newtrans_window() when our program is starting to load 
new trans window with references to Glade file. 
*/
gboolean init_newtrans_window()
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
	newtranswindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "new_trans_window"));
	newtranswindow->SESN_label = GTK_WIDGET (gtk_builder_get_object (builder, "new_trans_SESN_label"));

	gtk_builder_connect_signals (builder, newtranswindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

/* callback for Cancel button in new trans window */
void on_new_trans_cancel_button_clicked ()
{
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}

