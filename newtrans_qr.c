#include "header.h"

static void kill_qr_zbar_process()
{
	/*check child process availability, if exists kill it*/
	if(kill(qr_zbar_pid, 0) >= 0)
	{
		if(kill(qr_zbar_pid, SIGTERM) >= 0)
		{
			printf("qr zbarcam process killed with SIGTERM\n");
		}
		else
		{
			kill(qr_zbar_pid, SIGKILL);
			printf("qr zbarcam process killed with SIGKILL\n");
		}
	}
	else printf("qr zbarcam child process does not exists\n");
}

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
	kill_qr_zbar_process();
	
	/*open nfc new trans window*/
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}
/* child process watch callback */
static void qr_child_watch( GPid pid, gint status, GString *data )
{
	data = g_string_new(NULL);
	
	/* Close pid */
    g_spawn_close_pid( pid );
    
    g_string_free(data,TRUE);
    
    on_newtrans_qr_cancel_button_clicked();
}

/* io out watch callback */
static gboolean qr_out_watch( GIOChannel *channel, GIOCondition cond, GString *data )
{
	GIOStatus status;
	
	gchar detect_str[10];
	memset(detect_str,0,10);
	
	char notif_message[128];
	
	data = g_string_new(NULL);

    if( cond == G_IO_HUP )
    {
        g_io_channel_unref( channel );
        return( FALSE );
    }

    status = g_io_channel_read_line_string( channel, data, NULL, NULL );
 
    switch(status)
    {
		case G_IO_STATUS_EOF:
			printf("EOF\n");
			break;
			
		case G_IO_STATUS_NORMAL:
			fprintf(stdout,"%s",data->str);
			
			memcpy(detect_str,data->str,8);
			printf("detect_str:%s\n",detect_str);
			if(!strcmp(detect_str,"QR-Code:"))
			{
				kill_qr_zbar_process();
				sprintf(notif_message, "success read QR Code!\nData:%s", data->str+8);
				notification_message(notif_message);
				
				//~ on_newtrans_qr_cancel_button_clicked();
				//~ parse_qr_data(data);
					
				//~ if(write_lastTransaction_log() == TRUE)
				//~ {
					//CREATE PDF HERE!!!
					//~ create_receipt_from_lastTransactionData();
					//~ parse_log_file_and_write_to_treeview(logNum(), logNum());
					
					/*open receipt window and main menu*/
					//~ Bitwise WindowSwitcherFlag;
					//~ f_status_window = FALSE;
					//~ f_mainmenu_window = TRUE;
					//~ f_receipt_nfc_window = TRUE;
					//~ WindowSwitcher(WindowSwitcherFlag);
				//~ }
			}
			
			break;
	
		case G_IO_STATUS_AGAIN: 
			break;
		case G_IO_STATUS_ERROR:
		default:
			printf("Error stdout from child process\n");
			error_message("Error reading from child process");
			break;
	}
		
    g_string_free(data,TRUE);

    return( TRUE );
}

/* io err watch callback */
static gboolean qr_err_watch( GIOChannel *channel, GIOCondition cond, GString *data )
{
    gchar *string;
    gsize  size;
    
	data = g_string_new(NULL);

    if( cond == G_IO_HUP )
    {
        g_io_channel_unref( channel );
        return( FALSE );
    }

    g_io_channel_read_line( channel, &string, &size, NULL, NULL );
    fprintf(stderr,"%s",string);    
    g_free( string );
    g_string_free(data,TRUE);

    return( TRUE );
}

void qr_zbar_child_process()
{
    GPid        pid;
    gchar      *argv[] = { "/usr/bin/zbarcam", NULL };
    gint        out,
                err;
    GIOChannel *out_ch,
               *err_ch;
    gboolean    ret;

	GString *data;
	data = g_string_new(NULL);
	
    /* Spawn child process */
    ret = g_spawn_async_with_pipes( NULL, argv, NULL,
                                    G_SPAWN_DO_NOT_REAP_CHILD, NULL,
                                    data, &pid, NULL, &out, &err, NULL );
    if( ! ret )
    {
        g_error( "SPAWN FAILED" );
        return;
    }
    
	qr_zbar_pid = pid;
	
    /* Add watch function to catch termination of the process. This function
     * will clean any remnants of process. */
    g_child_watch_add( pid, (GChildWatchFunc)qr_child_watch, data );

    /* Create channels that will be used to read data from pipes. */
    out_ch = g_io_channel_unix_new( out );
    err_ch = g_io_channel_unix_new( err );

    /* Add watches to channels */
    g_io_add_watch( out_ch, G_IO_IN | G_IO_HUP, (GIOFunc)qr_out_watch, data );
    g_io_add_watch( err_ch, G_IO_IN | G_IO_HUP, (GIOFunc)qr_err_watch, data );
    
    g_string_free(data,TRUE);
}

