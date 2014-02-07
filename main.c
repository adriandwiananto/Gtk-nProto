#include <gtk/gtk.h>
#include "header.h"

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
