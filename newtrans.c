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
	newtransNFCwindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "new_trans_nfc_window"));
	newtransNFCwindow->SESN_label = GTK_WIDGET (gtk_builder_get_object (builder, "new_trans_nfc_SESN_label"));

	gtk_builder_connect_signals (builder, newtransNFCwindow);
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
			printf("nfc poll process killed with SIGTERM\n");
		}
		else
		{
			kill(nfc_poll_pid, SIGKILL);
			printf("nfc poll process killed with SIGKILL\n");
		}
	}
	else printf("nfc poll child process does not exists\n");
}

/* callback for destroy and delete-event new trans window */
void on_new_trans_nfc_destroy_delete_event ()
{
	/*check child process availability, if exists kill it*/
	kill_nfc_poll_process();
	gtk_widget_hide(newtransNFCwindow->window);
}

/* callback for Cancel button in new trans window */
void on_new_trans_nfc_cancel_button_clicked ()
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
void hexstrToBinArr(unsigned char* dest, gchar* source, gsize destlength)
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
static gboolean parse_nfc_data(GString *nfcdata)
{
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

	print_array_inHex("Received Data:", ReceivedData, data_len/2);
	
	//TODO:
	//remove ndef header, take only emoney frame
	//read emoney frame header
	//if header valid, decrypt payload using IV sent
	//write to log
	
	/* Parse NDEF data */
	NdefHeader ReceivedNDEF;
	
	if(ReceivedData[0] & 0x80) ReceivedNDEF.msgBegin = TRUE;
	else ReceivedNDEF.msgBegin = FALSE;
	
	if(ReceivedData[0] & 0x40) ReceivedNDEF.msgEnd = TRUE;
	else ReceivedNDEF.msgEnd = FALSE;
	
	if(ReceivedData[0] & 0x20) ReceivedNDEF.chunkFlag = TRUE;
	else ReceivedNDEF.chunkFlag = FALSE;
	
	if(ReceivedData[0] & 0x10) ReceivedNDEF.shortRec = TRUE;
	else ReceivedNDEF.shortRec = FALSE;
	
	if(ReceivedData[0] & 0x08) ReceivedNDEF.IDLen = TRUE;
	else ReceivedNDEF.IDLen = FALSE;
	
	//RFC2046 Media-type
	if(ReceivedData[0] & 0x02) ReceivedNDEF.TNF = 0x02;
	
	if((ReceivedNDEF.TNF == 0x02) && ReceivedNDEF.shortRec && !ReceivedNDEF.chunkFlag)
	{
		int index = 1;
		ReceivedNDEF.typeLen = ReceivedData[index];
		index++;
		
		ReceivedNDEF.payloadLen = ReceivedData[index];
		index++;
		
		if(ReceivedNDEF.typeLen > 0)
		{
			unsigned char type[ReceivedNDEF.typeLen];
			memcpy(type,ReceivedData+index,ReceivedNDEF.typeLen);
			index += ReceivedNDEF.typeLen;
			
			unsigned char payload[ReceivedNDEF.payloadLen];
			memcpy(payload,ReceivedData+index, ReceivedNDEF.payloadLen);
			index += ReceivedNDEF.payloadLen;
			
			return parse_transaction_frame(payload);
		}
		else return FALSE;
	}
	else return FALSE;
}

gboolean parse_transaction_frame(unsigned char *payload)
{
	unsigned char PT = payload[1];
	
	unsigned char encryptedPayload[32];
	memset(encryptedPayload,0,32);
	memcpy(encryptedPayload,payload+7,32);
	
	unsigned char transIV[16];
	memset(transIV,0,16);
	memcpy(transIV,payload+39,16);
	
	unsigned char decryptedPayload[32];
	memset(decryptedPayload,0,32);
	
	if(decrypt_transaction_frame(decryptedPayload, encryptedPayload, transIV))
	{
		if(*(decryptedPayload+18) != lastTransactionData.SESNbyte[0])return FALSE;
		if(*(decryptedPayload+19) != lastTransactionData.SESNbyte[1])return FALSE;
		
		int padding;
		for(padding = 0; padding < 0x0c; padding++)
		{
			if(*(decryptedPayload+20+padding) != 0x0c)return FALSE;
		}
		
		/* DO NOT USE IV VALUE AGAIN! 
		 * AFTER DECRYPT USING OpenSSL, IV VALUE CHANGED!! 
		 */
		lastTransactionData.PT = PT;
		memcpy(&lastTransactionData.ACCNbyte, decryptedPayload, 6);
		memcpy(&lastTransactionData.TSbyte, decryptedPayload+6, 4);
		memcpy(&lastTransactionData.AMNTbyte, decryptedPayload+10, 4);
		memcpy(&lastTransactionData.LATSbyte, decryptedPayload+14, 4);
		
		lastTransactionData.ACCNlong = 0;
		int i=0;
		for(i=0; i<6; i++)
		{
			if(i)lastTransactionData.ACCNlong <<= 8;
			lastTransactionData.ACCNlong |= lastTransactionData.ACCNbyte[i];
		}
		lastTransactionData.ACCNlong &= 0xFFFFFFFFFFFF;
		
		/* gcc in 32-bit Linux always shift in 32 bit!
		 * thus shifting > 32 bit always give wrong result
		 * http://stackoverflow.com/a/11855612/3095632
		 */
		//~ lastTransactionData.ACCNlong = 	(lastTransactionData.ACCNbyte[0]<<40) | (lastTransactionData.ACCNbyte[1]<<32) | 
										//~ (lastTransactionData.ACCNbyte[2]<<24) | (lastTransactionData.ACCNbyte[3]<<16) | 
										//~ (lastTransactionData.ACCNbyte[4]<<8) | lastTransactionData.ACCNbyte[5];
		
		lastTransactionData.AMNTlong = 	(lastTransactionData.AMNTbyte[0]<<24) |	(lastTransactionData.AMNTbyte[1]<<16) | 
										(lastTransactionData.AMNTbyte[2]<<8) | lastTransactionData.AMNTbyte[3];
		
		lastTransactionData.TSlong =	(lastTransactionData.TSbyte[0]<<24) |	(lastTransactionData.TSbyte[1]<<16) | 
										(lastTransactionData.TSbyte[2]<<8) | lastTransactionData.TSbyte[3];
						
		lastTransactionData.LATSlong =	(lastTransactionData.LATSbyte[0]<<24) |	(lastTransactionData.LATSbyte[1]<<16) | 
										(lastTransactionData.LATSbyte[2]<<8) | lastTransactionData.LATSbyte[3];
						
#ifdef DEBUG_MODE
		unsigned char FL = *payload;
		unsigned char FF = *(payload+2);
		unsigned int SESNheader = (payload[3]<<8) | payload[4];

		printf("\nFL: %02X\n",FL);
		printf("PT: %02X\n",lastTransactionData.PT);
		printf("FF: %02X\n",FF);
		printf("SESN in header: %d\n",SESNheader);

		print_array_inHex("decrypted payload:", decryptedPayload, 32);
		print_array_inHex("IV:", payload+39, 16);
		print_array_inHex("ACCN byte:", lastTransactionData.ACCNbyte, 6);

		printf(	"\nACCN: %ju | AMNT: %lu | TS: %lu | LATS: %lu\n", 
				lastTransactionData.ACCNlong, 
				lastTransactionData.AMNTlong, 
				lastTransactionData.TSlong,
				lastTransactionData.LATSlong);
#endif

		return TRUE;
	}
	else
	{
		printf("error decryption!\n");
		return FALSE;
	}
}

/* child process watch callback */
static void cb_child_watch( GPid pid, gint status, GString *data )
{
	data = g_string_new(NULL);
	
	//~ gtk_widget_hide(newtransNFCwindow->window);

	if (WIFEXITED(status))
	{
		switch(WEXITSTATUS(status))
		{
			case 0:
				break;
			case 1:
				error_message("Reader error! Reconnect reader!");
				break;
			case 2:
				break;
			case 3:
				error_message("Transaction failed! Retry tapping your phone again. (error:3)");
				break;
			case 4:
				error_message("Transaction failed! Retry tapping your phone again. (error:4)");
				break;
			case 5:
				error_message("Reader initialization FATAL error!");
				break;
			case 6:
				error_message("Wrong SESN input!");
				break;
			case 7:
				error_message("FATAL error on customer's side!! Wrong transaction key!");
				break;
			default:
				error_message("Transaction failed! error:99");
				break;
		}
	}
	
	/* Close pid */
    g_spawn_close_pid( pid );
    
    g_string_free(data,TRUE);
    
    on_new_trans_nfc_cancel_button_clicked();
}

/* io out watch callback */
static gboolean cb_out_watch( GIOChannel *channel, GIOCondition cond, GString *data )
{
	GIOStatus status;
	
	gchar detect_str[8];
	memset(detect_str,0,8);
	
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
			
			memcpy(detect_str,data->str,5);
			if(!strcmp(detect_str,"DATA:"))
			{
				if(parse_nfc_data(data) == TRUE)
				{
					if(write_lastTransaction_log() == TRUE)
					{
						//CREATE PDF HERE!!!
						create_receipt_from_lastTransactionData();
						parse_log_file_and_write_to_treeview(logNum(), logNum());
						
						/*open receipt window and main menu*/
						Bitwise WindowSwitcherFlag;
						f_status_window = FALSE;
						f_mainmenu_window = TRUE;
						f_receipt_nfc_window = TRUE;
						WindowSwitcher(WindowSwitcherFlag);
					}
				}
			}
			
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
	const gchar *passwordStr;
	passwordStr = gtk_entry_get_text(GTK_ENTRY(passwordwindow->text_entry));
	
    GPid        pid;
    //~ gchar      *argv[] = { "../../latihan/popen/./helloworld", NULL };
    gchar      *argv[] = { "./picc_emulation_write", (gchar*) passwordStr, SESN, NULL };
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
