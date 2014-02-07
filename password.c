#include <gtk/gtk.h>

/* location of UI XML file relative to path in which program is running */
#define BUILDER_XML_FILE "password.glade"

typedef struct
{
	GtkWidget *window;
	GtkWidget *text_entry;
	GtkWidget *button1;
	GtkWidget *button2;
}PasswordWindow;

void on_pwd_ok_button_clicked (GtkButton *button, PasswordWindow *passwordwindow);
void on_pwd_cancel_button_clicked (GtkButton *button, PasswordWindow *passwordwindow);
void on_pwd_entry_activate (GtkButton *button, PasswordWindow *passwordwindow);

void read_entry(PasswordWindow *passwordwindow);

gboolean init_pwd_window(PasswordWindow *passwordwindow);
void error_message (const gchar *message);

int main(int argc, char *argv[])
{
	PasswordWindow *passwordwindow;

	passwordwindow = g_slice_new(PasswordWindow);

	gtk_init(&argc,&argv);

	if(init_pwd_window(passwordwindow) == FALSE) return 1;

	gtk_widget_show(passwordwindow->window);

	gtk_main();

	g_slice_free(PasswordWindow, passwordwindow);

	return 0;
}

/*
We call init_pwd_window() when our program is starting to load our TutorialTextEditor struct
with references to the widgets we need. This is done using GtkBuilder to read
the XML file we created using Glade.
*/
gboolean init_pwd_window(PasswordWindow *passwordwindow)
{
	GtkBuilder              *builder;
	GError                  *err=NULL;

	/* use GtkBuilder to build our interface from the XML file */
	builder = gtk_builder_new ();
	if (gtk_builder_add_from_file (builder, BUILDER_XML_FILE, &err) == 0)
	{
		error_message (err->message);
		g_error_free (err);
		return FALSE;
	}

	/* get the widgets which will be referenced in callbacks */
	passwordwindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "pwd_window"));
	passwordwindow->text_entry = GTK_WIDGET (gtk_builder_get_object (builder, "pwd_entry"));
	passwordwindow->button1 = GTK_WIDGET (gtk_builder_get_object (builder, "pwd_ok_button"));
	passwordwindow->button2 = GTK_WIDGET (gtk_builder_get_object (builder, "pwd_cancel_button"));

	gtk_builder_connect_signals (builder, passwordwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

/*
We call error_message() any time we want to display an error message to the
user. It will both show an error dialog and log the error to the terminal
window.
*/
void
error_message (const gchar *message)
{
        GtkWidget               *dialog;
        
        /* log to terminal window */
        g_warning (message);
        
        /* create an error message dialog and display modally to the user */
        dialog = gtk_message_dialog_new (NULL, 
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_ERROR,
                                         GTK_BUTTONS_OK,
                                         message);
        
        gtk_window_set_title (GTK_WINDOW (dialog), "Error!");
        gtk_dialog_run (GTK_DIALOG (dialog));      
        gtk_widget_destroy (dialog);         
}

void on_pwd_ok_button_clicked (GtkButton *button, PasswordWindow *passwordwindow)
{
	printf("OK BUTTON CLICKED \n");
	read_entry(passwordwindow);
}

void on_pwd_cancel_button_clicked (GtkButton *button, PasswordWindow *passwordwindow)
{
	printf("CANCEL BUTTON CLICKED \n");	
	gtk_main_quit();
}

void on_pwd_entry_activate (GtkButton *button, PasswordWindow *passwordwindow)
{
	printf("password entered\n");
	read_entry(passwordwindow);
}

void read_entry(PasswordWindow *passwordwindow)
{
	const gchar *pwd_entry_text;
	pwd_entry_text = gtk_entry_get_text(GTK_ENTRY(passwordwindow->text_entry));
	printf("Password: %s\n", pwd_entry_text);
	
	gtk_entry_set_text(passwordwindow->text_entry, "");
	
}
