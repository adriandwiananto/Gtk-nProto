#include "header.h"

static gboolean send_log_to_server();

/*
We call init_settlement_window() when our program is starting to load 
settlement window with references to Glade file. 
*/
gboolean init_settlement_window()
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
	settlementwindow->claim_button = GTK_WIDGET (gtk_builder_get_object (builder, "settlement_claim_button"));
	
	gtk_builder_connect_signals (builder, settlementwindow);
	g_object_unref(G_OBJECT(builder));
	
	gchar *settlement_balance = "Rp 0";
	settlementwindow->settlement_balance = 0;

	gtk_label_set_text((GtkLabel *)settlementwindow->total_label, settlement_balance);

	gtk_widget_set_sensitive(settlementwindow->claim_button, FALSE);
	
	return TRUE;
}

/* Callback for Claim button in settlement window */
void on_settlement_claim_button_clicked()
{
	if(send_log_to_server() == TRUE)
	{
		notification_message("Settlement Success!");
		gtk_widget_hide(settlementwindow->window);
	}
	else
	{
		error_message("Settlement Fail!");
	}
}

/* Callback for Cancel button in settlement window */
void on_settlement_cancel_button_clicked()
{
	/*open main menu window only*/
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}

static gboolean send_log_to_server()
{
	json_object* log_jobj = create_log_as_json_object();

	gchar aesKeyString[65];
	memset(aesKeyString,0,65);
	
	if(send_log_jsonstring_to_server	(aesKeyString, 
									json_object_to_json_string(log_jobj), 
									"http://emoney-server.herokuapp.com/sync.json") == FALSE)
		return FALSE;
	else
		//DELETE LOG
		return TRUE;
}
