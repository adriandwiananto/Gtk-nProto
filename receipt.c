#include "header.h"
#include <setjmp.h>
#include <hpdf.h>
#include <sys/stat.h>

jmp_buf env;

static void rcp_child_watch( GPid pid, gint status, GString *data );
static gboolean rcp_err_watch( GIOChannel *channel, GIOCondition cond, GString *data );
static gboolean rcp_out_watch( GIOChannel *channel, GIOCondition cond, GString *data );
static void kill_nfc_receipt_process();

/*
We call init_pwd_window() when our program is starting to load 
password prompt window with references to Glade file. 
*/
gboolean init_receipt_window()
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
	receiptwindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "receipt_window"));
	receiptwindow->label = GTK_WIDGET (gtk_builder_get_object (builder, "receipt_receipt_label"));

	gtk_builder_connect_signals (builder, passwordwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

/* callback for OK button in password prompt window */
void on_receipt_finish_button_clicked ()
{
	/*check child process availability, if exists kill it*/
	kill_nfc_receipt_process();
	
	/*open main menu window only*/
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}

static void kill_nfc_receipt_process()
{
	/*check child process availability, if exists kill it*/
	if(kill(nfc_receipt_pid, 0) >= 0)
	{
		if(kill(nfc_receipt_pid, SIGTERM) >= 0)
		{
			printf("process killed with SIGTERM\n");
		}
		else
		{
			kill(nfc_receipt_pid, SIGKILL);
			printf("process killed with SIGKILL\n");
		}
	}
	else printf("child process does not exists\n");
}

/* create child process for sending receipt (call other program) */
void spawn_nfc_receipt_process(gchar* receipt_ndef)
{
    GPid        pid;
    gchar      *argv[] = { "./picc_emulation_read", receipt_ndef, NULL };
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
    
	nfc_receipt_pid = pid;
	
    /* Add watch function to catch termination of the process. This function
     * will clean any remnants of process. */
    g_child_watch_add( pid, (GChildWatchFunc)rcp_child_watch, data );

    /* Create channels that will be used to read data from pipes. */
    out_ch = g_io_channel_unix_new( out );
    err_ch = g_io_channel_unix_new( err );

    /* Add watches to channels */
    g_io_add_watch( out_ch, G_IO_IN | G_IO_HUP, (GIOFunc)rcp_out_watch, data );
    g_io_add_watch( err_ch, G_IO_IN | G_IO_HUP, (GIOFunc)rcp_err_watch, data );
    
    g_string_free(data,TRUE);
}

static void rcp_child_watch( GPid pid, gint status, GString *data )
{
	data = g_string_new(NULL);
	
	gtk_widget_hide(newtranswindow->window);

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
}

/* io out watch callback */
static gboolean rcp_out_watch( GIOChannel *channel, GIOCondition cond, GString *data )
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
			memcpy(detect_str,data->str,5);
			if(!strcmp(detect_str,"DATA:"))
			{
				/*close receipt window and open main menu*/
				Bitwise WindowSwitcherFlag;
				f_status_window = FALSE;
				f_mainmenu_window = TRUE;
				WindowSwitcher(WindowSwitcherFlag);
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
static gboolean rcp_err_watch( GIOChannel *channel, GIOCondition cond, GString *data )
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

gboolean build_receipt_packet(gchar* receipt_ndef_str)
{
	int i=0;
	unsigned char receipt_ndef_array[55];
	uintmax_t ACCN;
	if(get_INT64_from_config(&ACCN, "application.ACCN") == FALSE) return FALSE;
	gchar ACCNstr[32];
	memset(ACCNstr, 0, 32);
	sprintf(ACCNstr, "%ju", ACCN);

	receipt_ndef_array[0] = 55; // length 55
	receipt_ndef_array[1] = 1; //offline
	receipt_ndef_array[2] = 1; //merchant
	memcpy(receipt_ndef_array+3, lastTransactionData.SESNbyte, 2);
	memset(receipt_ndef_array+5,0,2);
	
	for(i=5; i>=0; i--)
	{
		if(i<5)ACCN >>= 8;
		receipt_ndef_array[7+i] = ACCN & 0xFF;
	}
	
	memcpy(receipt_ndef_array+13, lastTransactionData.TSbyte, 4);
	memcpy(receipt_ndef_array+17, lastTransactionData.AMNTbyte, 4);
	memcpy(receipt_ndef_array+21, lastTransactionData.LATSbyte, 4);
	memcpy(receipt_ndef_array+25, lastTransactionData.SESNbyte, 2);
	memset(receipt_ndef_array+27,12,12); //PADDING

	gchar* buf_ptr;

	unsigned char transKey[32];
	memset(transKey,0,32);
	
	unsigned char receiptPayloadPlain[32];
	memcpy(receiptPayloadPlain, receipt_ndef_array+7, 32);
	
	unsigned char receiptPayloadEncrypted[32];
	memset(receiptPayloadEncrypted,0,32);

	unsigned char aes_key[32];
	memset(aes_key,0,32);
	const gchar *passwordStr;
	passwordStr = gtk_entry_get_text(GTK_ENTRY(passwordwindow->text_entry));
	getTransKey(aes_key, passwordStr, ACCNstr, FALSE);
	
	unsigned char IV[16]; //, iv_dec[AES_BLOCK_SIZE];
	RAND_bytes(IV, 16);
	memcpy(receipt_ndef_array+39,IV,16);

	AES_KEY enc_key;
	AES_set_encrypt_key(aes_key, 256, &enc_key);
	AES_cbc_encrypt(receiptPayloadPlain, receiptPayloadEncrypted, 32, &enc_key, IV, AES_ENCRYPT);

	memcpy(receipt_ndef_array+7,receiptPayloadEncrypted,32);

	buf_ptr = receipt_ndef_str;
	for (i = 0; i < 55; i++)
	{
		buf_ptr += sprintf((char*)buf_ptr, "%02X", receipt_ndef_array[i]);
	}
	*(buf_ptr + 1) = '\0';
	printf("receipt ndef in str: %s\n", receipt_ndef_str);
	
	return TRUE;
}

static void error_handler (HPDF_STATUS   error_no,
       HPDF_STATUS   detail_no,
       void         *user_data)
{
	printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
		(HPDF_UINT)detail_no);
	longjmp(env, 1);
}

static gboolean create_receipt_write_toPdf (char* filename, char* timestamp_inString, char* accnM_inString, char* accnP_inString, int amount_inInt)
{
	HPDF_Doc  pdf;
	pdf = HPDF_New (error_handler, NULL);

	if (!pdf) {
		fprintf (stderr, "error: cannot create PdfDoc object\n");
		return FALSE;
	}

	if (setjmp(env)) {
		HPDF_Free (pdf);
		return FALSE;
	}

	HPDF_Font bold_font;
	HPDF_Font normal_font;
	bold_font = HPDF_GetFont (pdf, "Times-Bold", NULL);
	normal_font = HPDF_GetFont (pdf, "Times-Roman", NULL);

	/* Add a new page object. */
	HPDF_Page page;
	page = HPDF_AddPage (pdf);

	HPDF_REAL height;
	HPDF_REAL width;
	height = 210;
	width = 298;
	
	HPDF_Page_SetWidth(page, width);
	HPDF_Page_SetHeight(page, height);

	/* add image to page */
	HPDF_Image image;
	image = HPDF_LoadPngImageFromFile(pdf, "ITB_logo.png");
	HPDF_Page_DrawImage (page, image, 71, 145, 40, 40);

	/* Print the title */
	HPDF_Page_SetFontAndSize (page, bold_font, 30);
	HPDF_Page_BeginText (page);
	HPDF_Page_TextOut (page, 120, 158, "e-Money");
	HPDF_Page_EndText (page);

	char merchant[64];
	char payer[64];
	memset(merchant, 0, 64);
	memset(payer, 0, 64);
	
	strcpy(merchant, "Merchant: ");
	strcat(merchant, accnM_inString);
	strcpy(payer, "Payer: ");
	strcat(payer, accnP_inString);
	
	/* print date and ID */
	HPDF_Page_BeginText (page);
	HPDF_Page_SetFontAndSize (page, normal_font, 12);
	HPDF_Page_TextOut (page, 20, 116, timestamp_inString);
	HPDF_Page_TextOut (page, 20, 99, merchant);
	HPDF_Page_TextOut (page, 20, 82, payer);
	HPDF_Page_EndText (page);

	/* print amount */
	char amount[64];
	memset(amount, 0, 64);
	sprintf(amount, "Rp%'d", amount_inInt);

	HPDF_Page_BeginText(page);
	HPDF_Page_SetFontAndSize (page, bold_font, 36);
	HPDF_REAL amount_width = HPDF_Page_TextWidth (page, amount);
	HPDF_Page_TextOut (page, width-amount_width-23, 34, amount);

	struct stat st;
    char* receipt_dir = "eMoney/";
    if (stat(receipt_dir, &st) != 0)
    {
        printf("Making log directory\n");
        mkdir(receipt_dir, S_IRWXU | S_IRWXG);
    }

	char final_path[256];
	memset(final_path, 0, 256);
	strcpy(final_path, receipt_dir);
	strcat(final_path, filename);    
	
	/* save to file */
	HPDF_SaveToFile (pdf, final_path);

	printf("receipt created! %s\n",final_path);

	/* clean up */
	HPDF_Free (pdf);

	return 0;
}

gboolean create_receipt_from_lastTransactionData()
{
	uintmax_t ACCN;
	gchar accnM_inString[32];
	memset(accnM_inString, 0, 32);
	
	if(get_INT64_from_config(&ACCN, "application.ACCN") == FALSE)
	{
		error_message("error get ACCN from config");
		return FALSE;
	}
	
	/*convert ACCN from INT64 to string*/
	sprintf(accnM_inString, "%ju", ACCN);
	
	gchar accnP_inString[32];
	memset(accnP_inString, 0, 32);
	sprintf(accnP_inString, "%ju", lastTransactionData.ACCNlong);
	
	gchar timestamp_inString[64];
	memset(timestamp_inString, 0, 64);
	
	time_t rawtime = (unsigned int)lastTransactionData.TSlong;
	struct tm *timeinfo;
	timeinfo = localtime(&rawtime);
	strftime (timestamp_inString,64,"%d/%m/%Y %H:%M:%S",timeinfo);
	
	gchar timestamp_for_filename[64];
	memset(timestamp_for_filename, 0, 64);
	strftime (timestamp_for_filename,64,"%m%d%Y%H%M%S",timeinfo);
	
	char filename[240];
	memset(filename, 0, 240);
	strcpy(filename, "receipt-");
	strcat(filename, timestamp_for_filename);
	strcat(filename, ".pdf");
	
	return create_receipt_write_toPdf(filename, timestamp_inString, accnM_inString, accnP_inString, (int)lastTransactionData.AMNTlong);
}
