#ifndef _NPROTO_
#define _NPROTO_

/* location of UI XML file relative to path in which program is running */
#define PASSWORD_GLADE_FILE "password.glade"
#define MAINMENU_GLADE_FILE "mainmenu.glade"

typedef struct
{
	GtkWidget *window;
	GtkWidget *text_entry;
	GtkWidget *button1;
	GtkWidget *button2;
}PasswordWindow;

typedef struct
{
	GtkWidget *window;
	GtkWidget *new_trans_button;
	GtkWidget *history_button;
	GtkWidget *settlement_button;
	GtkWidget *option_button;
	GtkWidget *exit_button;
}MainMenuWindow;

typedef struct
{
	PasswordWindow *passwordwindow;
	MainMenuWindow *mainmenuwindow;
}Windows;

/*callback*/
//~ void on_pwd_ok_button_clicked (GtkButton *button, Windows *windows_obj);
//~ void on_pwd_cancel_button_clicked (GtkButton *button, Windows *windows_obj);
//~ void on_pwd_entry_activate (GtkButton *button, Windows *windows_obj);
void on_pwd_ok_button_clicked (GtkButton *button, PasswordWindow *passwordwindow);
void on_pwd_cancel_button_clicked (GtkButton *button, PasswordWindow *passwordwindow);
void on_pwd_entry_activate (GtkButton *button, PasswordWindow *passwordwindow);

/*other*/
void read_entry(PasswordWindow *passwordwindow);
void error_message (const gchar *message);
//~ void show_mainmenu(Windows *windows_obj);

/*window init*/
gboolean init_pwd_window(PasswordWindow *passwordwindow);
//~ gboolean init_mainmenu_window(Windows *windows_obj);

#endif
