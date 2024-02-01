#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

#define MAX_PROCESSES 1024

// Function prototype for print_process_info
void print_process_info(WINDOW *win, int pid);

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

                    // Display CPU and RAM usage for the process
                    print_process_info(win, pid);
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

// Function to print CPU and RAM usage for a given process
void print_process_info(WINDOW *win, int pid) {
    FILE *fp;
    char path[PATH_MAX];
    char line[256];

    // Construct the path to the stat file for the process
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    fp = fopen(path, "r");
    if (fp != NULL) {
        // Read the contents of the stat file
        if (fgets(line, sizeof(line), fp) != NULL) {
            // Parse the contents
            unsigned long utime, stime;
            unsigned long rss;

            sscanf(line, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %lu", &utime, &stime, &rss);

            

            // calculate and print CPU and RAM usage for the process
            unsigned long total_time = utime + stime;
            wprintw(win, "  CPU Usage: %lu ms, RAM Usage: %lu KB\n", total_time, rss);
        }
        fclose(fp);
    }
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

    // Print the screen one to not have a black screen
    print_processes(mainwin);

    // Main loop
    while ((ch = getch()) != 'q') {
        // Check for 'r' key pressed - manually refresh the process list
        if (ch == 'r') {
            print_processes(mainwin);
        }

    }

    // Clean up and exit ncurses
    delwin(mainwin);
    endwin();
    refresh();

    return 0;
}
