#include <gtk/gtk.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#ifndef _NPROTO_
#define _NPROTO_

/* location of UI XML file relative to path in which program is running */
#define UI_GLADE_FILE "ui.glade"

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
	GtkWidget *history_textview;
}HistoryWindow;

/*callback function*/
void on_pwd_ok_button_clicked (GtkButton *button);
void on_pwd_cancel_button_clicked (GtkButton *button);
void on_pwd_entry_activate (GtkButton *button);
void on_mm_new_trans_button_clicked (GtkButton *button);
void on_mm_history_button_clicked (GtkButton *button);
void on_mm_settlement_button_clicked (GtkButton *button);
void on_mm_option_button_clicked (GtkButton *button);
void on_mm_exit_button_clicked (GtkButton *button);
void on_new_trans_cancel_button_clicked (GtkButton *button);
void on_history_saveas_button_clicked (GtkButton *button);
void on_history_close_button_clicked (GtkButton *button);

/*window init function*/
gboolean init_pwd_window();
gboolean init_mainmenu_window();
gboolean init_newtrans_window();
gboolean init_history_window();

/*other function*/
void read_pwd_entry();
void error_message (const gchar *message);
void WindowSwitcher(gboolean f_password_window, 
					gboolean f_mainmenu_window, 
					gboolean f_newtrans_window,
					gboolean f_history_window,
					gboolean f_settlement_window,
					gboolean f_option_window);
int random_number_generator(int min_number, int max_number);

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

#endif
