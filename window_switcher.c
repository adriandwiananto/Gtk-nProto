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
		
		create_merch_req_png();
		
		gtk_image_set_from_file((GtkImage *)newtransQRwindow->image, "merch_req.png");
		gtk_widget_show(newtransQRwindow->window);
		
		//start zbarcam child process
		//~ qr_zbar_child_process();
	}
	else
	{
		if(remove("merch_req.png") == 0)
			printf("merch_req.png deleted!\n");
			
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

	/*send receipt qr window switcher*/
	if(f_receipt_qr_window == TRUE)
	{
		gchar receipt_qr[111];
		build_receipt_packet(receipt_qr);
		
		char qrencode_command[256];
		memset(qrencode_command,0,256);
		sprintf(qrencode_command,"qrencode -o merch_receipt.png -s 7 -m 2 '%s'",receipt_qr);
		system(qrencode_command);
		
		gtk_image_set_from_file((GtkImage *)receiptQRwindow->image, "merch_receipt.png");
		gtk_widget_show(receiptQRwindow->window);
	}
	else
	{
		if(remove("merch_receipt.png") == 0)
			printf("merch_receipt.png deleted!\n");

		gtk_widget_hide(receiptQRwindow->window);	
	}
}
