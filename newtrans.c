#include "header.h"

/*
We call init_newtrans_window() when our program is starting to load 
new trans window with references to Glade file. 
*/
gboolean init_newtrans_window()
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
	newtranswindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "new_trans_window"));
	newtranswindow->SESN_label = GTK_WIDGET (gtk_builder_get_object (builder, "new_trans_SESN_label"));

	gtk_builder_connect_signals (builder, newtranswindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

static void kill_nfc_poll_process()
{
	/*check child process availability, if exists kill it*/
	if(kill(nfc_poll_pid, 0) >= 0)
	{
		if(kill(nfc_poll_pid, SIGTERM) >= 0)
		{
			printf("process killed with SIGTERM\n");
		}
		else
		{
			kill(nfc_poll_pid, SIGKILL);
			printf("process killed with SIGKILL\n");
		}
	}
	else printf("child process does not exists\n");
}

/* callback for destroy and delete-event new trans window */
void on_new_trans_destroy_delete_event ()
{
	/*check child process availability, if exists kill it*/
	kill_nfc_poll_process();
	gtk_widget_hide(newtranswindow->window);
}

/* callback for Cancel button in new trans window */
void on_new_trans_cancel_button_clicked ()
{
	/*check child process availability, if exists kill it*/
	kill_nfc_poll_process();
	
	/*open main menu window only*/
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}

/* parse nfc data from child process */
static void parse_nfc_data(gchar *nfcdata)
{
	gchar dataonly[128];
	memset(dataonly, 0, 128);
	memcpy(dataonly, nfcdata+5, strlen(nfcdata)-5);
	printf("data only: %s\n",dataonly);
}

/* child process watch callback */
static void cb_child_watch( GPid pid, gint status)//, Data *data );
{
    /* Close pid */
    g_spawn_close_pid( pid );
}

/* io out watch callback */
static gboolean cb_out_watch( GIOChannel *channel, GIOCondition cond)//, Data *data );
{
    gchar *string;
    gsize  size;
	GIOStatus status;
	
    if( cond == G_IO_HUP )
    {
        g_io_channel_unref( channel );
        return( FALSE );
    }

    status = g_io_channel_read_line( channel, &string, &size, NULL, NULL );
    
    if(status == G_IO_STATUS_EOF)parse_nfc_data(string);
    //~ gtk_text_buffer_insert_at_cursor( data->out, string, -1 );
	else fprintf(stdout,"%s",string);
	//~ parse_nfc_data(string);
	g_free( string );

    return( TRUE );
}

/* io err watch callback */
static gboolean cb_err_watch( GIOChannel *channel, GIOCondition cond)//, Data *data );
{
    gchar *string;
    gsize  size;

    if( cond == G_IO_HUP )
    {
        g_io_channel_unref( channel );
        return( FALSE );
    }

    g_io_channel_read_line( channel, &string, &size, NULL, NULL );
    //gtk_text_buffer_insert_at_cursor( data->err, string, -1 );
    fprintf(stderr,"%s",string);    
    g_free( string );

    return( TRUE );
}

/* create child process for nfc poll (call other program) */
void nfc_poll_child_process()
{
    GPid        pid;
    //~ gchar      *argv[] = { "../../latihan/popen/./helloworld", NULL };
    gchar      *argv[] = { "./nfcreceive", NULL };
    gint        out,
                err;
    GIOChannel *out_ch,
               *err_ch;
    gboolean    ret;

    /* Spawn child process */
    ret = g_spawn_async_with_pipes( NULL, argv, NULL,
                                    G_SPAWN_DO_NOT_REAP_CHILD, NULL,
                                    NULL, &pid, NULL, &out, &err, NULL );
    if( ! ret )
    {
        g_error( "SPAWN FAILED" );
        return;
    }
	
	nfc_poll_pid = pid;
    /* Add watch function to catch termination of the process. This function
     * will clean any remnants of process. */
    g_child_watch_add( pid, (GChildWatchFunc)cb_child_watch, NULL );

    /* Create channels that will be used to read data from pipes. */
    out_ch = g_io_channel_unix_new( out );
    err_ch = g_io_channel_unix_new( err );

    /* Add watches to channels */
    g_io_add_watch( out_ch, G_IO_IN | G_IO_HUP, (GIOFunc)cb_out_watch, NULL );
    g_io_add_watch( err_ch, G_IO_IN | G_IO_HUP, (GIOFunc)cb_err_watch, NULL );
}
