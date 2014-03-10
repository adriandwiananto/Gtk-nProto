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

void parse_log_file()
{
	GtkTreeIter iter;
	
	//~ gint lognum = 1;
	//~ FILE *logfile;
	//~ char *line = NULL;
	//~ size_t len = 0;
	//~ ssize_t read;
	
	//~ /*open file with filename "log"*/
	//~ logfile = fopen("log","r");
	//~ if(logfile == NULL)error_message("fail to open log file");
	//~ 
	//~ /*read every line of the file until EOF (-1)*/
	//~ while((read = getline(&line, &len, logfile)) != -1)
	//~ {
		//~ /*write line to gtk tree*/
		//~ write_to_history_tree(read, line, lognum, &iter);
		//~ lognum++;
	//~ }
	
	
	
	/* check log row in db
	 * pull first row
	 * decrypt
	 * write to treeview
	 * pull next row
	 * etc
	 */
	
	int logRow = logNum();
	int logLen;
	int i = 0;
	unsigned char fromDB[128];
	memset(fromDB,0,128);
	unsigned char fromDBbyte[64];
	memset(fromDBbyte,0,64);

	unsigned char logDecrypted[32];
	memset(logDecrypted,0,32);
	
	unsigned char IV[16];
	memset(IV,0,16);

	unsigned char logKey[32];
	memset(logKey,0,32);

	uintmax_t ACCN;
	get_INT64_from_config(&ACCN, "application.ACCN");
	const gchar *password;
	password = gtk_entry_get_text(GTK_ENTRY(passwordwindow->text_entry));

	gchar ACCNstr[32];
	memset(ACCNstr, 0, 32);
	sprintf(ACCNstr, "%ju", ACCN);
	
	printf("pwd:%s,ACCN:%s\n",password,ACCNstr);
	if(derive_key(logKey, password, ACCNstr, 9000)==FALSE)fprintf(stderr,"error deriving key\n");
	
	for(i = 0; i < logRow; i++)
	{
		logLen = read_log_blob(fromDB,i+1);
		if(logLen)
		{
#ifdef DEBUG_MODE
			printf("from DB: %s, length: %d\n",fromDB,logLen);
#endif
			hexstrToBinArr(fromDBbyte,(gchar*)fromDB,logLen/2);
			
			int z=0;
			printf("byte array:\n");
			for(z=0;z<logLen/2;z++)printf("%02X ",fromDBbyte[z]);
			printf("\n");
			
			memcpy(IV, fromDBbyte+32, 16);	

			printf("IV:\n");
			for(z=0;z<16;z++)printf("%02X ", IV[z]);
			printf("\n");
			printf("log key:\n");
			for(z=0;z<32;z++)printf("%02X ", logKey[z]);
			printf("\n");
			
			AES_KEY dec_key;
			AES_set_decrypt_key(logKey, 256, &dec_key);
			AES_cbc_encrypt(fromDBbyte, logDecrypted, 32, &dec_key, IV, AES_DECRYPT);

			printf("log decrypted:\n");
			for(z=0;z<32;z++)printf("%02X ", logDecrypted[z]);
			printf("\n");
			
			unsigned int TS = (logDecrypted[24]<<24) | (logDecrypted[25]<<16) | (logDecrypted[26]<<8) | (logDecrypted[27]);
			time_t rawtime = TS;
			struct tm timeinfo;
			char timebuffer [80];

			(void) localtime_r(&rawtime, &timeinfo);
			strftime (timebuffer,80,"%d/%m/%Y %H:%M",&timeinfo);
			printf ("timestamp:%s\n",timebuffer);
			
			unsigned int amount = (logDecrypted[20]<<24) | (logDecrypted[21]<<16) | (logDecrypted[22]<<8) | (logDecrypted[23]);
			uintmax_t senderACCN;
			
			for(z=0; z<6; z++)
			{
				if(i)senderACCN <<= 8;
				senderACCN |= logDecrypted[8+i];
			}
			
			unsigned int lognum = (logDecrypted[0]<<16) | (logDecrypted[1]<<8) | (logDecrypted[2]);

			printf("lognum:%d,senderACCN:%ju, Amount:%d\n", lognum,senderACCN, amount);
			char ACCNstring[20];
			memset(ACCNstring, 0, 20);
			sprintf(ACCNstring, "%ju", senderACCN);
			char AmountString[10];
			memset(AmountString, 0, 10);
			sprintf(AmountString, "%d", amount);
			printf("lognum:%d,senderACCN:%s, Amount:%s\n", lognum,ACCNstring, AmountString);

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
