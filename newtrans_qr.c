#include "header.h"

/*
We call init_newtrans_chooser_window() when our program is starting to load 
new trans chooser window with references to Glade file. 
*/
gboolean init_newtrans_qr_window()
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
	newtransQRwindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "newtrans_qr_window"));
	newtransQRwindow->image = GTK_WIDGET (gtk_builder_get_object (builder, "newtrans_qr_image"));

	gtk_builder_connect_signals (builder, newtransQRwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

/* Callback for nfc button on new trans chooser window */
void on_newtrans_qr_cancel_button_clicked()
{
	/*open nfc new trans window*/
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}

