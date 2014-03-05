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

/* hex written as string to binary Array */
static void hexstrToBinArr(unsigned char* dest, gchar* source, gsize destlength)
{
	int i;
	
	for (i=0;i<destlength;i++) 
	{
		int value;
		sscanf(source+2*i,"%02x",&value);
		dest[i] = (unsigned char)value;
	}
}

/* parse nfc data from child process */
static void parse_nfc_data(GString *nfcdata)
{
	int i=0;
	gsize data_len;
	/* subtract 5 from prefix "DATA:"
	 * subtract 1 from postfix newline
	 */
	data_len = (nfcdata->len)-6;

	gchar data_only[262];
	memset(data_only,0,262);
	
	memcpy(data_only,(nfcdata->str)+5,data_len);
	
	unsigned char ReceivedData[data_len/2];
	hexstrToBinArr(ReceivedData, data_only, data_len/2);

	printf("Data in Array:\n");
	for (i=0;i<data_len/2;i++) 
	{
		printf("%02X ", ReceivedData[i]);
		if(!((i+1)%8))printf("\n");
	}
	printf("\n");
	
	//TODO:
	//remove ndef header, take only emoney frame
	//read emoney frame header
	//if header valid, decrypt payload using IV sent
	//write to log
}

/* child process watch callback */
static void cb_child_watch( GPid pid, gint status, GString *data )
{
	data = g_string_new(NULL);
	
	if (WIFEXITED(status))
	{
		if(!WEXITSTATUS(status))notification_message("Transaction success!");
		else
		{
			switch(WEXITSTATUS(status))
			{
				case 1:
					error_message("Reader error! Reconnect reader!");
					break;
				case 2:
					break;
				case 3:
					error_message("Transaction failed! error:3");
					break;
				case 4:
					error_message("Transaction failed! error:4");
					break;
				default:
					error_message("Transaction failed! error:99");
					break;
			}
		}
	}
	
	/* Close pid */
    g_spawn_close_pid( pid );
    
    gtk_widget_hide(newtranswindow->window);
	
    g_string_free(data,TRUE);
}

/* io out watch callback */
static gboolean cb_out_watch( GIOChannel *channel, GIOCondition cond, GString *data )
{
	GIOStatus status;
	
	gchar detect_str[5];
	memset(detect_str,0,5);
	
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
			
			memcpy(detect_str,data->str,5);
			if(!strcmp(detect_str,"DATA:"))parse_nfc_data(data);
			
			break;
	
		case G_IO_STATUS_AGAIN: break;
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
static gboolean cb_err_watch( GIOChannel *channel, GIOCondition cond, GString *data )
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

/* create child process for nfc poll (call other program) */
void nfc_poll_child_process(gchar *SESN)
{
    GPid        pid;
    //~ gchar      *argv[] = { "../../latihan/popen/./helloworld", NULL };
    gchar      *argv[] = { "./picc_emulation_write", SESN, NULL };
    gint        out,
                err;
    GIOChannel *out_ch,
               *err_ch;
    gboolean    ret;

	GString *data;
	data = g_string_new(NULL);
	printf("SESN: %s\n",SESN);
    /* Spawn child process */
    ret = g_spawn_async_with_pipes( NULL, argv, NULL,
                                    G_SPAWN_DO_NOT_REAP_CHILD, NULL,
                                    data, &pid, NULL, &out, &err, NULL );
    if( ! ret )
    {
        g_error( "SPAWN FAILED" );
        return;
    }
    
	nfc_poll_pid = pid;
	
    /* Add watch function to catch termination of the process. This function
     * will clean any remnants of process. */
    g_child_watch_add( pid, (GChildWatchFunc)cb_child_watch, data );

    /* Create channels that will be used to read data from pipes. */
    out_ch = g_io_channel_unix_new( out );
    err_ch = g_io_channel_unix_new( err );

    /* Add watches to channels */
    g_io_add_watch( out_ch, G_IO_IN | G_IO_HUP, (GIOFunc)cb_out_watch, data );
    g_io_add_watch( err_ch, G_IO_IN | G_IO_HUP, (GIOFunc)cb_err_watch, data );
    
    g_string_free(data,TRUE);
}
