#include "header.h"

#define COMMAND_LEN 20
#define DATA_SIZE 512

/*
Function for switching active window
*/
void WindowSwitcher(Bitwise WindowSwitcherFlag)
{
	if(config_checking() != 1)
	{
		gtk_main_quit();
	}
		
	/*password window switcher*/
	(f_password_window == TRUE)?gtk_widget_show(passwordwindow->window):gtk_widget_hide(passwordwindow->window);
	
	/*main menu window switcher*/
	(f_mainmenu_window == TRUE)?gtk_widget_show(mainmenuwindow->window):gtk_widget_hide(mainmenuwindow->window);
	
	/*option window switcher*/
	(f_option_window == TRUE)?gtk_widget_show(optionwindow->window):gtk_widget_hide(optionwindow->window);
	
	/*registration window switcher*/
	(f_registration_window == TRUE)?gtk_widget_show(registrationwindow->window):gtk_widget_hide(registrationwindow->window);

	/*new trans chooser switcher*/
	if(f_newtranschooser_window == TRUE)
	{
		memset(&lastTransactionData, 0, sizeof(lastTransactionData));
		gtk_widget_show(newtranschooserwindow->window);
	}
	else
	{
		gtk_widget_hide(newtranschooserwindow->window);
	}
		
	/*new trans window switcher*/
	if(f_newtrans_nfc_window == TRUE)
	{
		memset(&lastTransactionData,0,sizeof(lastTransactionData));
		
		gchar SESN_text[4];
		int randomnumber;
		randomnumber = random_number_generator(100,999);
		snprintf(SESN_text,  4, "%d", randomnumber);
		gtk_label_set_text((GtkLabel *)newtransNFCwindow->SESN_label, SESN_text);
		
		lastTransactionData.SESNint = randomnumber;
		lastTransactionData.SESNbyte[0] = (randomnumber>>8) & 0xFF;
		lastTransactionData.SESNbyte[1] = randomnumber & 0xFF;
		gtk_widget_show(newtransNFCwindow->window);
		
		//NFC polling goes in here
		nfc_poll_child_process(SESN_text);
	}
	else
	{
		gtk_widget_hide(newtransNFCwindow->window);	
	}
		
	/*new trans QR window switcher*/
	if(f_newtrans_qr_window == TRUE)
	{
		memset(&lastTransactionData,0,sizeof(lastTransactionData));
		
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
		get_INT64_from_config(&ACCN, "application.ACCN");
		gchar ACCNstr[32];
		memset(ACCNstr, 0, 32);
		sprintf(ACCNstr, "%ju", ACCN);
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
		memset(merchant_request_packet+17, 0, 4);
		memset(merchant_request_packet+21, 0, 4);
		memcpy(merchant_request_packet+25, lastTransactionData.SESNbyte, 2);
		memset(merchant_request_packet+27,12,12); //PADDING

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

		AES_KEY enc_key;
		AES_set_encrypt_key(aes_key, 256, &enc_key);
		AES_cbc_encrypt(merchantrequestPayloadPlain, merchantrequestPayloadEncrypted, 32, &enc_key, IV, AES_ENCRYPT);

		memcpy(merchant_request_packet+7,merchantrequestPayloadEncrypted,32);

		char merchant_request_str[111];
		buf_ptr = merchant_request_str;
		for (i = 0; i < 55; i++)
		{
			buf_ptr += sprintf((char*)buf_ptr, "%02X", merchant_request_packet[i]);
		}
		*(buf_ptr + 1) = '\0';
		printf("receipt ndef in str: %s\n", merchant_request_str);
	
		char qrencode_command[128];
		memset(qrencode_command,0,128);
		sprintf(qrencode_command,"qrencode -o merch_req.png -s 7 -m 2 '%s'",merchant_request_str);
		
		system(qrencode_command);
		
		gtk_image_set_from_file((GtkImage *)newtransQRwindow->image, "merch_req.png");
		gtk_widget_show(newtransQRwindow->window);
		
		//start zbarcam child process
		qr_zbar_child_process();
	}
	else
	{
		gtk_widget_hide(newtransQRwindow->window);	
	}
	
	/*history window switcher*/
	if(f_history_window == TRUE)
	{
		gtk_widget_show(historywindow->window);
	}
	else
	{
		gtk_widget_hide(historywindow->window);	
	}

	/*settlement window switcher*/
	if(f_settlement_window == TRUE)
	{
		gchar balanceToWrite[32];
		memset(balanceToWrite,0,32);
		
		sprintf(balanceToWrite, "Rp %'d", settlementwindow->settlement_balance);
		gtk_label_set_text((GtkLabel *)settlementwindow->total_label, balanceToWrite);
		
		//~ if(settlementwindow->settlement_balance)
			//~ gtk_widget_set_sensitive(settlementwindow->claim_button, TRUE);

		gtk_widget_show(settlementwindow->window);
	}
	else
	{
		gtk_widget_hide(settlementwindow->window);	
	}

	/*send receipt window switcher*/
	if(f_receipt_nfc_window == TRUE)
	{
		gchar successMsg[255];
		sprintf(successMsg,
				"Transaction Success!\nAmount: Rp. %'lu\nFrom: %ju\n",
				lastTransactionData.AMNTlong,
				lastTransactionData.ACCNlong);
				
		gtk_label_set_text((GtkLabel *)receiptNFCwindow->label, successMsg);
		
		gchar receipt_ndef[111];
		build_receipt_packet(receipt_ndef);
		
		spawn_nfc_receipt_process(receipt_ndef);
		gtk_widget_show(receiptNFCwindow->window);
	}
	else
	{
		gtk_widget_hide(receiptNFCwindow->window);	
	}
}
