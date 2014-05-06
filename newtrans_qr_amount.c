#include "header.h"

/*
We call init_newtrans_chooser_window() when our program is starting to load 
new trans chooser window with references to Glade file. 
*/
gboolean init_amount_qr_window()
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
	amountQRwindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "amount_qr_window"));
	amountQRwindow->amount_entry = GTK_WIDGET (gtk_builder_get_object (builder, "amount_qr_entry"));

	gtk_builder_connect_signals (builder, amountQRwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

/* Callback for cancel button on new trans qr window */
void on_amount_qr_request_button_clicked()
{
	const gchar* amount_qr = gtk_entry_get_text(GTK_ENTRY(amountQRwindow->amount_entry));
	uintmax_t amount_qr_long = strtoumax(amount_qr, NULL, 10);
	
	if(amount_qr_long <= 0 || amount_qr_long > 1000000)
	{
		error_message("valid amount 1 ~ 1.000.000");
		return;
	}
	
	lastTransactionData.AMNTlong = amount_qr_long;
	
	int i;
	for(i=3; i>=0; i--)
	{
		if(i<3)lastTransactionData.AMNTlong >>= 8;
		lastTransactionData.AMNTbyte[i] = lastTransactionData.AMNTlong & 0xFF;
	}
	
	print_array_inHex("amount byte:", lastTransactionData.AMNTbyte, 4);
	
	/*open nfc new trans window*/
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_newtrans_qr_window = TRUE;
	f_mainmenu_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}

/* Callback for continue button on new trans qr window */
void on_amount_qr_cancel_button_clicked()
{
	/*open nfc new trans window*/
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);	
}

void on_amount_qr_entry_insert_text(GtkEditable *buffer, gchar *new_text, gint new_text_length, gint *position, gpointer data)
{
	int i;
	guint sigid;

	/* Only allow 0-9 to be written to the entry */
	for (i = 0; i < new_text_length; i++) {
		if (new_text[i] < '0' || new_text[i] > '9') {
			sigid = g_signal_lookup("insert-text",
						G_OBJECT_TYPE(buffer));
			g_signal_stop_emission(buffer, sigid, 0);
			return;
		}
	}
}
