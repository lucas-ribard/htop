#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <ctype.h> 
#include <string.h>
#include <unistd.h> 
#include <linux/limits.h>
#include <linux/limits.h>

#define MAX_PROCESSES 1024

void print_processes(WINDOW *win) {
    DIR *dir;
    struct dirent *entry;
    FILE *fp;
    char path[PATH_MAX];
    char line[256];
    int pid;
    char process_name[256];

    wclear(win);

    if ((dir = opendir("/proc")) == NULL) {
        perror("Unable to open /proc");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (isdigit(entry->d_name[0])) {
            pid = atoi(entry->d_name);
            snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);

            fp = fopen(path, "r");
            if (fp != NULL) {
                if (fgets(line, sizeof(line), fp) != NULL) {
                    // Extract process name from cmdline
                    strncpy(process_name, line, sizeof(process_name) - 1);
                    process_name[sizeof(process_name) - 1] = '\0';

                    // Print PID and process name
                    wprintw(win, "PID: %d, Name: %s", pid, process_name);
                }
                fclose(fp);
            }
        }
    }

    closedir(dir);

    wrefresh(win);
}

int main() {
    WINDOW *mainwin;
    int ch;

    // Initialize ncurses
    if ((mainwin = initscr()) == NULL) {
        fprintf(stderr, "Error initializing ncurses.\n");
        exit(EXIT_FAILURE);
    }

    // Enable keypad input
    keypad(mainwin, TRUE);
    timeout(0); // non-blocking input

    // Main loop
    while ((ch = getch()) != 'q') {
        if (ch == 'r') {
            // 'r' key pressed - manually refresh the process list
            print_processes(mainwin);
        }

        // Other processing, if needed...

        // Sleep for a short time to prevent high CPU usage
        usleep(10000);
    }

    // Clean up and exit
    delwin(mainwin);
    endwin();
    refresh();

    return 0;
}
