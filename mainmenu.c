#include "header.h"

gboolean init_mainmenu_window()
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

	gtk_builder_connect_signals (builder, mainmenuwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

void on_mm_new_trans_button_clicked (GtkButton *button)
{
	WindowSwitcher(FALSE,TRUE,TRUE);
}

void on_mm_history_button_clicked (GtkButton *button)
{

}

void on_mm_settlement_button_clicked (GtkButton *button)
{
	
}

void on_mm_option_button_clicked (GtkButton *button)
{
	
}

void on_mm_exit_button_clicked (GtkButton *button)
{
	gtk_main_quit();
}
