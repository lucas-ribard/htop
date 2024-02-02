#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

#define MAX_PROCESSES 1024
#define DEFAULT_COLOR 1
#define HIGHLIGHT_COLOR 2
#define HIGHLIGHT_COLOR_SECONDARY 3

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

    // Initialize color pairs
    start_color();
    // Define the DEFAULT_COLOR constant
    
    // Initialize the color pair
                //state      //text color //background color
    init_pair(DEFAULT_COLOR, COLOR_WHITE, COLOR_BLACK);
    init_pair(HIGHLIGHT_COLOR, COLOR_YELLOW, COLOR_BLACK);
    init_pair(HIGHLIGHT_COLOR_SECONDARY, COLOR_CYAN, COLOR_BLACK);

    // Set highlight color secondary ( cyan )
    attron(COLOR_PAIR(HIGHLIGHT_COLOR_SECONDARY));

    mvwprintw(win, 0, 0, "Press 'R' to refresh \n");

    // Reset to default color
    attroff(COLOR_PAIR(HIGHLIGHT_COLOR));
    
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

                    // Print process id (PID) and process name in the window with a newline character
                    wprintw(win, "PID : %d      Name : %s\n", pid, process_name);

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
    // Set default color ( White )
    attron(COLOR_PAIR(DEFAULT_COLOR));

    // Construct the path to the stat file for the process
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    fp = fopen(path, "r");
    if (fp != NULL) {
        // Read the contents of the stat file
        if (fgets(line, sizeof(line), fp) != NULL) {
            // Parse the contents
            unsigned long long total_time, total_system_time;
            unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
            unsigned long utime, stime, rss;

            // Parse the process times from /proc/[pid]/stat
            sscanf(line, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %lu", &utime, &stime, &rss);

            total_time = utime + stime;

            // Parse the system times from /proc/stat
            FILE *fp = fopen("/proc/stat", "r");
            fscanf(fp, "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);
            fclose(fp);

            total_system_time = user + nice + system + idle + iowait + irq + softirq + steal;
            

             // Calculate the CPU usage
            double cpu_usage = 100.0 * total_time / total_system_time;

            // Set highlight color ( Yellow )
            attron(COLOR_PAIR(HIGHLIGHT_COLOR));
            

            // calculate and print CPU and RAM usage for the process
            wprintw(win, "  CPU Usage : %.2f%%\n", cpu_usage);

            // Reset to default color
            attroff(COLOR_PAIR(HIGHLIGHT_COLOR));
           
           
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
