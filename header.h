#ifndef _NPROTO_
#define _NPROTO_

/* location of UI XML file relative to path in which program is running */
#define BUILDER_XML_FILE "password.glade"

typedef struct
{
	GtkWidget *window;
	GtkWidget *text_entry;
	GtkWidget *button1;
	GtkWidget *button2;
}PasswordWindow;

/*callback*/
void on_pwd_ok_button_clicked (GtkButton *button, PasswordWindow *passwordwindow);
void on_pwd_cancel_button_clicked (GtkButton *button, PasswordWindow *passwordwindow);
void on_pwd_entry_activate (GtkButton *button, PasswordWindow *passwordwindow);

/*other*/
void read_entry(PasswordWindow *passwordwindow);
void error_message (const gchar *message);

/*window init*/
gboolean init_pwd_window(PasswordWindow *passwordwindow);

#endif
