#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

#define MAX_PROCESSES 1024

// Function to print information about processes in the given window
void print_processes(WINDOW *win) {
    DIR *dir;
    struct dirent *entry;
    FILE *fp;
    char path[PATH_MAX];
    char line[256];
    int pid;
    char process_name[256];

    // Clear the window
    wclear(win);
    mvwprintw(win, 0, 0, "Press 'R' to refresh \n");
    // Open the /proc directory
    if ((dir = opendir("/proc")) == NULL) {
        perror("Unable to open /proc");
        exit(EXIT_FAILURE);
    }

    // Iterate over entries in /proc
    while ((entry = readdir(dir)) != NULL) {
        // Check if the entry name begins with a digit (PID is expected)
        if (isdigit(entry->d_name[0])) {
            // Convert the entry name to PID
            pid = atoi(entry->d_name);

            // Construct the path to the cmdline file for the process
            snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
            
            // Open the cmdline file
            fp = fopen(path, "r");
            if (fp != NULL) {
                // Read the command line from the file
                if (fgets(line, sizeof(line), fp) != NULL) {
                    // Extract process name from cmdline
                    strncpy(process_name, line, sizeof(process_name) - 1);
                    process_name[sizeof(process_name) - 1] = '\0';

                    // Print PID and process name in the window with a newline character
                    wprintw(win, "PID: %d, Name: %s\n", pid, process_name);
                }
                fclose(fp);
            }
        }
    }

    // Close the /proc directory
    closedir(dir);

    // Refresh the window to display changes
    wrefresh(win);
}


// Main function
int main() {
    WINDOW *mainwin;
    int ch;

    // Initialize ncurses
    if ((mainwin = initscr()) == NULL) {
        fprintf(stderr, "Error initializing ncurses.\n");
        exit(EXIT_FAILURE);
    }

    // Enable keypad
    keypad(mainwin, TRUE);
    timeout(0);

    // Main loop
    
    while ((ch = getch()) != 'q') {
        // Check for 'r' key pressed - manually refresh the process list
        if (ch == 'r') {
            print_processes(mainwin);
        }

        // Other processing, if needed...

        // Sleep for a short time to prevent high CPU usage
        usleep(10000);
    }

    // Clean up and exit ncurses
    delwin(mainwin);
    endwin();
    refresh();

    return 0;
}
