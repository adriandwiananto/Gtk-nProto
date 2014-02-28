#include <gtk/gtk.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <inttypes.h>
#include <glib.h>
#include <sys/wait.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

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

/*spawn function*/
void nfc_poll_child_process();

/*other function*/
void read_pwd_entry();
void error_message (const gchar *message);
void notification_message (const gchar *message);
void WindowSwitcher(Bitwise WindowSwitcherFlag);
int random_number_generator(int min_number, int max_number);
void parse_log_file();
void write_to_history_tree(ssize_t read, char* line, gint lognum, GtkTreeIter *iter);

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
#endif
