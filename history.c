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
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}

void parse_log_file()
{
	GtkTreeIter iter;
	
	gint lognum = 1;
	FILE *logfile;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	
	logfile = fopen("log","r");
	if(logfile == NULL)error_message("fail to open log file");
	
	while((read = getline(&line, &len, logfile)) != -1)
	{
		write_to_history_tree(read, line, lognum, &iter);
		lognum++;
	}
	free(line);
	fclose(logfile);
}

void write_to_history_tree(ssize_t read, char* line, gint lognum, GtkTreeIter *iter)
{
	enum
	{
		LOG_NUMBER = 0,
		LOG_CONTENT,
		LOG_LENGTH,
		NUM_COLS
	};
	
	gtk_list_store_append(historywindow->history_store, iter);
	gtk_list_store_set	(	historywindow->history_store, iter, 
							LOG_NUMBER, lognum,
							LOG_CONTENT, line,
							LOG_LENGTH, read,
							-1
						);
}
