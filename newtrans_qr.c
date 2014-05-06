#include "header.h"

static gboolean parse_qr_data(GString *nfcdata);

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
	newtransQRwindow->continue_button = GTK_WIDGET (gtk_builder_get_object (builder, "newtrans_qr_continue_button"));

	gtk_builder_connect_signals (builder, newtransQRwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

/* Callback for cancel button on new trans qr window */
void on_newtrans_qr_cancel_button_clicked()
{
	printf("cancel!\n");
	kill_qr_zbar_process();
	
	/*open nfc new trans window*/
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}

/* Callback for continue button on new trans qr window */
void on_newtrans_qr_continue_button_clicked()
{
	gtk_widget_set_sensitive(newtransQRwindow->continue_button, FALSE);
	qr_zbar_child_process();
}

/* child process watch callback */
static void qr_child_watch( GPid pid, gint status, GString *data )
{
	data = g_string_new(NULL);
	
	gtk_widget_set_sensitive(newtransQRwindow->continue_button, TRUE);

	/* Close pid */
    g_spawn_close_pid( pid );
    
    g_string_free(data,TRUE);
}

/* io out watch callback */
static gboolean qr_out_watch( GIOChannel *channel, GIOCondition cond, GString *data )
{
	GIOStatus status;
	
	gchar detect_str[10];
	memset(detect_str,0,10);
	
	//~ char notif_message[128];
	
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
			//~ fprintf(stdout,"%s",data->str);
			
			memcpy(detect_str,data->str,8);
			if(!strcmp(detect_str,"QR-Code:"))
			{
				//~ sprintf(notif_message, "success read QR Code!\nData:%s", data->str+8);
				//~ notification_message(notif_message);
				
				if(parse_qr_data(data) == TRUE)
				{
					if(write_lastTransaction_log() == TRUE)
					{
						kill_qr_zbar_process();

						//~ //CREATE PDF HERE!!!
						create_receipt_from_lastTransactionData();
						parse_log_file_and_write_to_treeview(logNum(), logNum());
						
						/*open receipt window and main menu*/
						Bitwise WindowSwitcherFlag;
						f_status_window = FALSE;
						f_mainmenu_window = TRUE;
						f_receipt_qr_window = TRUE;
						WindowSwitcher(WindowSwitcherFlag);
					}
				}
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

void create_merch_req_png()
{
	int i = 0;
		
	gchar SESN_text[4];
	int randomnumber;
	randomnumber = random_number_generator(100,999);
	snprintf(SESN_text,  4, "%d", randomnumber);
	
	lastTransactionData.SESNint = randomnumber;
	lastTransactionData.SESNbyte[0] = (randomnumber>>8) & 0xFF;
	lastTransactionData.SESNbyte[1] = randomnumber & 0xFF;
	
	unsigned int timestamp;
	unsigned char timestamp_array[4];
	timestamp = (unsigned)time(NULL);
	for(i=3; i>=0; i--)
	{
		if(i<3)timestamp >>= 8;
		timestamp_array[i] = timestamp & 0xFF;
	}
	
	unsigned char ACCN_array[6];
	uintmax_t ACCN;
	gchar ACCNstr[32];
	ACCN = get_ACCN(ACCNstr);
	
	for(i=5; i>=0; i--)
	{
		if(i<5)ACCN >>= 8;
		ACCN_array[i] = ACCN & 0xFF;
	}
	
	unsigned char merchant_request_packet[55];
	
	merchant_request_packet[0] = 55; // length 55
	merchant_request_packet[1] = 1; //offline
	merchant_request_packet[2] = 1; //merchant
	memcpy(merchant_request_packet+3, lastTransactionData.SESNbyte, 2);
	memset(merchant_request_packet+5,0,2);
	memcpy(merchant_request_packet+7, ACCN_array, 6);
	memcpy(merchant_request_packet+13, timestamp_array, 4);
	//memset(merchant_request_packet+17, 0, 4);
	memcpy(merchant_request_packet+17, lastTransactionData.AMNTbyte, 4);
	memset(merchant_request_packet+21, 0, 4);
	memcpy(merchant_request_packet+25, lastTransactionData.SESNbyte, 2);
	memset(merchant_request_packet+27,12,12); //PADDING

	print_array_inHex("qr packet plain:",merchant_request_packet, 39);
	
	gchar* buf_ptr;

	unsigned char transKey[32];
	memset(transKey,0,32);
	
	unsigned char merchantrequestPayloadPlain[32];
	memcpy(merchantrequestPayloadPlain, merchant_request_packet+7, 32);
	
	unsigned char merchantrequestPayloadEncrypted[32];
	memset(merchantrequestPayloadEncrypted,0,32);

	unsigned char aes_key[32];
	memset(aes_key,0,32);
	const gchar *passwordStr;
	passwordStr = gtk_entry_get_text(GTK_ENTRY(passwordwindow->text_entry));
	getTransKey(aes_key, passwordStr, ACCNstr, FALSE);
	
	unsigned char IV[16]; //, iv_dec[AES_BLOCK_SIZE];
	RAND_bytes(IV, 16);
	memcpy(merchant_request_packet+39,IV,16);

	aes256cbc(merchantrequestPayloadEncrypted, merchantrequestPayloadPlain, aes_key, IV, "ENCRYPT");

	memcpy(merchant_request_packet+7,merchantrequestPayloadEncrypted,32);

	char merchant_request_str[111];
	buf_ptr = merchant_request_str;
	for (i = 0; i < 55; i++)
	{
		buf_ptr += sprintf((char*)buf_ptr, "%02X", merchant_request_packet[i]);
	}
	*(buf_ptr + 1) = '\0';
	printf("merchant request in str: %s\n", merchant_request_str);

	char qrencode_command[256];
	memset(qrencode_command,0,256);
	sprintf(qrencode_command,"qrencode -o merch_req.png -s 7 -m 2 '%s'",merchant_request_str);
	
	system(qrencode_command);
}

/* parse nfc data from child process */
static gboolean parse_qr_data(GString *nfcdata)
{
	gsize data_len;
	/* subtract 5 from prefix "QR-Code:"
	 * subtract 1 from postfix newline
	 */
	data_len = (nfcdata->len)-9;

	gchar data_only[262];
	memset(data_only,0,262);
	
	memcpy(data_only,(nfcdata->str)+8,data_len);
	
	unsigned char ReceivedData[data_len/2];
	hexstrToBinArr(ReceivedData, data_only, data_len/2);

	print_array_inHex("Received Data:", ReceivedData, data_len/2);
			
	unsigned char payload[data_len/2];
	memcpy(payload,ReceivedData, data_len/2);
	
	return parse_transaction_frame(payload);
}
