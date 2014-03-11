#include "header.h"

static void write_to_history_tree(uintmax_t senderACCN, unsigned int amount, char* date, int lognum, GtkTreeIter *iter);

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
	historywindow->history_tree = GTK_WIDGET (gtk_builder_get_object (builder, "history_treeview"));
	historywindow->history_store = GTK_LIST_STORE (gtk_builder_get_object (builder, "historystore"));

	gtk_builder_connect_signals (builder, historywindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

/* callback for Save As button in history window */
void on_history_saveas_button_clicked ()
{
	
}

/* callback for Close button in history window */
void on_history_close_button_clicked ()
{
	/*open main menu window only*/
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}

void parse_log_file_and_write_to_treeview(int startRow, int endRow)
{
	GtkTreeIter iter;
	
	/* check log row in db
	 * pull first row
	 * decrypt
	 * write to treeview
	 * pull next row
	 * etc
	 */
	
	int logLen;
	int i = 0;
	unsigned char fromDB[128];
	memset(fromDB,0,128);

	for(i = startRow; i <= endRow; i++)
	{
		logLen = read_log_blob(fromDB,i);
		if(logLen)
		{
			unsigned int lognum = 0;
			char timebuffer[80];
			memset(timebuffer,0,80);
			uintmax_t senderACCN = 0;
			unsigned int amount = 0;

			convert_DBdata_to_TreeView_Data(fromDB, logLen, &lognum, timebuffer, &senderACCN, &amount);
			settlementwindow->settlement_balance += amount;
			
			printf("lognum:%d,senderACCN:%ju, Amount:%d\n", lognum,senderACCN, amount);

			write_to_history_tree(senderACCN, amount, timebuffer, lognum, &iter);
		}
		memset(fromDB,0,128);
	}
}

static void write_to_history_tree(uintmax_t senderACCN, unsigned int amount, char* date, int lognum, GtkTreeIter *iter)
{
	enum
	{
		LOG_NUMBER = 0,
		LOG_TS,
		LOG_ACCN,
		LOG_AMNT,
		NUM_COLS
	};
	
	gtk_list_store_append(historywindow->history_store, iter);
	gtk_list_store_set	(	historywindow->history_store, iter, 
							LOG_NUMBER, lognum,
							LOG_TS, date,
							LOG_ACCN, senderACCN,
							LOG_AMNT, amount,
							-1
						);
}
