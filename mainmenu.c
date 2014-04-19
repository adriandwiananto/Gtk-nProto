#include "header.h"
#include <dirent.h>

static pid_t proc_find(const char* name);

/*
We call init_mainmenu_window() when our program is starting to load 
main menu window with references to Glade file. 
*/
gboolean init_mainmenu_window()
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
	mainmenuwindow->window = GTK_WIDGET (gtk_builder_get_object (builder, "mm_window"));

	gtk_builder_connect_signals (builder, mainmenuwindow);
	g_object_unref(G_OBJECT(builder));
	
	return TRUE;
}

/* callback for New Trans button in main menu window */
void on_mm_new_trans_button_clicked ()
{
	pid_t pid = proc_find("./picc_emulation_write");
	if(pid == -1)
	{
		/*open new trans window*/
		Bitwise WindowSwitcherFlag;
		f_status_window = FALSE;
		f_mainmenu_window = TRUE;
		f_newtranschooser_window = TRUE;
		WindowSwitcher(WindowSwitcherFlag);
	}
	else
	{
		printf("unfinished process pid: %d\n",pid);
		kill(pid, SIGTERM);
		notification_message("Please wait a moment before \nstarting new transaction");
	}
}

/* callback for History button in main menu window */
void on_mm_history_button_clicked ()
{
	/*open history window*/
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	f_history_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}

/* callback for Settlement button in main menu window */
void on_mm_settlement_button_clicked ()
{
	/*open settlement window*/
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	f_settlement_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}

/* callback for Option button in main menu window */
void on_mm_option_button_clicked ()
{
	/*open option window*/
	Bitwise WindowSwitcherFlag;
	f_status_window = FALSE;
	f_mainmenu_window = TRUE;
	f_option_window = TRUE;
	WindowSwitcher(WindowSwitcherFlag);
}

/* callback for Exit button in main menu window */
void on_mm_exit_button_clicked ()
{
	/*exit application*/
	gtk_main_quit();
}

static pid_t proc_find(const char* name) 
{
    DIR* dir;
    struct dirent* ent;
    char* endptr;
    char buf[512];

    if (!(dir = opendir("/proc"))) {
        perror("can't open /proc");
        return -1;
    }

    while((ent = readdir(dir)) != NULL) {
        /* if endptr is not a null character, the directory is not
         * entirely numeric, so ignore it */
        long lpid = strtol(ent->d_name, &endptr, 10);
        if (*endptr != '\0') {
            continue;
        }

        /* try to open the cmdline file */
        snprintf(buf, sizeof(buf), "/proc/%ld/cmdline", lpid);
        FILE* fp = fopen(buf, "r");

        if (fp) {
            if (fgets(buf, sizeof(buf), fp) != NULL) {
                /* check the first token in the file, the program name */
                char* first = strtok(buf, " ");
                if (!strcmp(first, name)) {
                    fclose(fp);
                    closedir(dir);
                    return (pid_t)lpid;
                }
            }
            fclose(fp);
        }

    }

    closedir(dir);
    return -1;
}
