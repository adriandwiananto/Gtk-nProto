#include <gtk/gtk.h>
#include "header.h"

gboolean init_mainmenu_window(MainMenuWindow *mainmenuwindow)
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
	mainmenuwindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "mm_window"));
	//~ mainmenuwindow->new_trans_button = GTK_WIDGET (gtk_builder_get_object (builder, "mm_new_trans_button"));
	//~ mainmenuwindow->history_button = GTK_WIDGET (gtk_builder_get_object (builder, "mm_history_button"));
	//~ mainmenuwindow->settlement_button = GTK_WIDGET (gtk_builder_get_object (builder, "mm_settlement_button"));
	//~ mainmenuwindow->option_button = GTK_WIDGET (gtk_builder_get_object (builder, "mm_option_button"));
	//~ mainmenuwindow->exit_button = GTK_WIDGET (gtk_builder_get_object (builder, "mm_exit_button"));

	gtk_builder_connect_signals (builder, mainmenuwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}
