#include "header.h"
#include <setjmp.h>
#include <hpdf.h>
#include <sys/stat.h>

/*
We call init_pwd_window() when our program is starting to load 
password prompt window with references to Glade file. 
*/
gboolean init_receipt_qr_window()
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
	receiptQRwindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "receipt_qr_window"));
	receiptQRwindow->image = GTK_WIDGET (gtk_builder_get_object (builder, "receipt_qr_image"));

	gtk_builder_connect_signals (builder, receiptQRwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

/* callback for Finish button in receipt window */
void on_receipt_qr_finish_button_clicked ()
{
	/*open main menu window only*/
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}
