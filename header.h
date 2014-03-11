#include <gtk/gtk.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <inttypes.h>
#include <glib.h>
#include <time.h>
#include <sys/wait.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

//~ #define DEBUG_MODE

#ifndef _NPROTO_
#define _NPROTO_

/* location of UI XML file relative to path in which program is running */
#define UI_GLADE_FILE "ui.glade"

/* crypto definition */
#define AES_MODE (256)
#define KEY_LEN_BYTE AES_MODE/8 

#define f_password_window		WindowSwitcherFlag.bit0
#define f_mainmenu_window		WindowSwitcherFlag.bit1
#define f_newtrans_window		WindowSwitcherFlag.bit2
#define f_history_window		WindowSwitcherFlag.bit3
#define f_settlement_window		WindowSwitcherFlag.bit4
#define f_option_window		 	WindowSwitcherFlag.bit5
#define f_registration_window	WindowSwitcherFlag.bit6
#define f_status_window 		WindowSwitcherFlag.status

typedef struct
{
	unsigned char* KeyEncryptionKey[32];
	unsigned char* LogKey[32];
}CryptoKey;

typedef struct
{
	gboolean msgBegin;
	gboolean msgEnd;
	gboolean chunkFlag;
	gboolean shortRec;
	gboolean IDLen;
	char TNF;
	unsigned char typeLen;
	unsigned char payloadLen;
}NdefHeader;

typedef struct
{
	unsigned char PT;

	unsigned char ACCNbyte[6];
	unsigned long long ACCNlong;

	unsigned char AMNTbyte[4];
	unsigned long AMNTlong;

	unsigned char TSbyte[4];
	unsigned long TSlong;
}transactionData;
	
typedef union
{
	struct 
	{
		unsigned char bit0:1;
		unsigned char bit1:1;
		unsigned char bit2:1;
		unsigned char bit3:1;
		unsigned char bit4:1;
		unsigned char bit5:1;
		unsigned char bit6:1;
		unsigned char bit7:1;
	};
	unsigned char status;
}Bitwise;

typedef struct
{
	GtkWidget *window;
	GtkWidget *text_entry;
}PasswordWindow;

typedef struct
{
	GtkWidget *window;
}MainMenuWindow;

typedef struct
{
	GtkWidget *window;
	GtkWidget *SESN_label;
}NewTransWindow;

typedef struct
{
	GtkWidget *window;
	GtkWidget *history_tree;
	GtkListStore *history_store;
}HistoryWindow;

typedef struct
{
	GtkWidget *window;
	GtkWidget *total_label;
	GtkWidget *claim_button;
	int settlement_balance;
}SettlementWindow;

typedef struct
{
	GtkWidget *window;
	GtkWidget *old_entry;
	GtkWidget *new_entry;
	GtkWidget *confirm_entry;
}OptionWindow;
	
typedef struct
{
	GtkWidget *window;
	GtkWidget *ACCN_entry;
	GtkWidget *new_entry;
	GtkWidget *confirm_entry;
}RegistrationWindow;

/*callback function*/
void on_pwd_ok_button_clicked ();
void on_pwd_cancel_button_clicked ();
void on_pwd_entry_activate ();
void on_mm_new_trans_button_clicked ();
void on_mm_history_button_clicked ();
void on_mm_settlement_button_clicked ();
void on_mm_option_button_clicked ();
void on_mm_exit_button_clicked ();
void on_new_trans_cancel_button_clicked ();
void on_history_saveas_button_clicked ();
void on_history_close_button_clicked ();
void on_settlement_claim_button_clicked ();
void on_settlement_cancel_button_clicked ();
void on_option_ok_button_clicked();
void on_option_cancel_button_clicked();
void on_new_trans_destroy_delete_event ();
void on_new_trans_cancel_button_clicked ();

/*window init function*/
gboolean init_pwd_window();
gboolean init_mainmenu_window();
gboolean init_newtrans_window();
gboolean init_history_window();
gboolean init_settlement_window();
gboolean init_option_window();
gboolean init_registration_window();

/*libconfig function*/
int config_checking();
gboolean create_new_config_file(uintmax_t ACCN, const char *password);
gboolean get_INT64_from_config(uintmax_t *value, const char *path);
gboolean get_string_from_config(char *value, const char *path);
gboolean write_string_to_config(char *value, const char *path);

/*crypto function*/
void passwordhashing(char *hashed, const gchar *password, const gchar *salt);
char *unbase64(unsigned char *input, int length);
char *base64(const unsigned char *input, int length);
gboolean wrap_aes_key(unsigned char *out, unsigned char *wrapper_key, unsigned char *key_to_wrap);
gboolean unwrap_aes_key(unsigned char *out, unsigned char *wrapper_key, unsigned char *key_to_unwrap);
gboolean derive_key(unsigned char *out, const gchar *password, const gchar *salt, unsigned int iteration);
void getTransKey(unsigned char* aes_key, const gchar* password, const gchar* ACCN, gboolean printResult);
gboolean decrypt_transaction_frame(unsigned char* output, unsigned char* input, unsigned char* IV);
gboolean getLogKey(unsigned char* logKey);

/*spawn function*/
void nfc_poll_child_process(gchar *SESN);

/*log function*/
gboolean createDB_and_table();
gboolean write_lastTransaction_log();
gboolean encrypt_lastTransaction_log(unsigned char* logHexInStr, unsigned int logNum);
int read_log_blob(unsigned char *dest, int row);
int logNum();
void convert_DBdata_to_TreeView_Data(unsigned char *DB_BLOB_data, int logLen, unsigned int *lognum, char *timebuffer, uintmax_t *senderACCN, unsigned int*amount);

/*other function*/
void read_pwd_entry();
void error_message (const gchar *message);
void notification_message (const gchar *message);
void WindowSwitcher(Bitwise WindowSwitcherFlag);
int random_number_generator(int min_number, int max_number);
void parse_log_file_and_write_to_treeview(int startRow, int endRow);
void hexstrToBinArr(unsigned char* dest, gchar* source, gsize destlength);

#ifndef DECLARE_VARIABLES
#define EXTERN /* nothing */
#else
#define EXTERN extern /* declare variable */
#endif

/*global window variable*/
EXTERN PasswordWindow *passwordwindow;
EXTERN MainMenuWindow *mainmenuwindow;
EXTERN NewTransWindow *newtranswindow;
EXTERN HistoryWindow *historywindow;
EXTERN SettlementWindow *settlementwindow;
EXTERN OptionWindow *optionwindow;
EXTERN RegistrationWindow *registrationwindow;

/*global variable*/
EXTERN int pass_attempt;
EXTERN GPid nfc_poll_pid;
EXTERN char nfc_data[128];
EXTERN transactionData lastTransactionData;
EXTERN CryptoKey cryptoKey;

#endif
