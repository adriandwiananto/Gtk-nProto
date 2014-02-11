#include "header.h"

/*
We call init_history_window() when our program is starting to load 
history window with references to Glade file. 
*/
gboolean init_history_window()
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
	historywindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "history_window"));
	historywindow->history_textview = GTK_WIDGET (gtk_builder_get_object (builder, "history_textview"));

	gtk_builder_connect_signals (builder, historywindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

/* callback for Save As button in history window */
void on_history_saveas_button_clicked (GtkButton *button)
{
	
}

/* callback for Close button in history window */
void on_history_close_button_clicked (GtkButton *button)
{
	WindowSwitcher(	FALSE,	//password window
					TRUE,	//gboolean f_mainmenu_window, 
					FALSE,	//new trans window
					FALSE,	//history window
					FALSE,	//settlement window
					FALSE);	//option window
}


