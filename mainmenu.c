#include <gtk/gtk.h>
#include "header.h"

/*
Main menu window launcher
*/
void show_mainmenu(Windows *windows_obj)
{
	gtk_widget_hide(windows_obj->passwordwindow->window);
	gtk_widget_show(windows_obj->mainmenuwindow->window);
}

/*
We call init_mainmenu_window() after password entry 
to show main menu window with references to Glade file. 
*/
gboolean init_mainmenu_window(Windows *windows_obj)
{
	GtkBuilder              *builder;
	GError                  *err=NULL;

	/* use GtkBuilder to build our interface from the XML file */
	builder = gtk_builder_new ();
	if (gtk_builder_add_from_file (builder, MAINMENU_GLADE_FILE, &err) == 0)
	{
		error_message (err->message);
		g_error_free (err);
		return FALSE;
	}

	/* get the widgets which will be referenced in callbacks */
	windows_obj->mainmenuwindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "mm_window"));
	windows_obj->mainmenuwindow->new_trans_button = GTK_WIDGET (gtk_builder_get_object (builder, "mm_new_trans_button"));
	windows_obj->mainmenuwindow->history_button = GTK_WIDGET (gtk_builder_get_object (builder, "mm_history_button"));
	windows_obj->mainmenuwindow->settlement_button = GTK_WIDGET (gtk_builder_get_object (builder, "mm_settlement_button"));
	windows_obj->mainmenuwindow->option_button = GTK_WIDGET (gtk_builder_get_object (builder, "mm_option_button"));
	windows_obj->mainmenuwindow->exit_button = GTK_WIDGET (gtk_builder_get_object (builder, "mm_exit_button"));

	gtk_builder_connect_signals (builder, windows_obj->mainmenuwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}
