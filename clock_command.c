#include "main.h"

int b_clock = 1;

void clock_handler(int signum) {
    b_clock = 0;
}

int clock_builtin(char **argv) {

    if (argv[2] == NULL) {
        fprintf(stderr, "clock -t: Invalid syntax\n");
        return 1;
    }

    int t = atoi(argv[2]);
    char *path = "/proc/driver/rtc";

    signal(SIGINT, clock_handler);

    printf("Press CTRL-C to stop:\n\n");
    for (; b_clock && t;) {
        
        int fd = open(path, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "Error getting time\n");
            return 1;
        }

        char all[2048];
        read(fd, all, 2048);
        close(fd);

        char **lines = tokenize_input(all, W_DELIM);
        char **fulldate = tokenize_input(lines[5], "-");
        char m[3], month[5];
        strcpy(m, fulldate[1]);
        
        if (!strcmp(m, "01"))
            strcpy(month, "Jan");
        else if (!strcmp(m, "02"))
            strcpy(month, "Feb");
        else if (!strcmp(m, "03"))
            strcpy(month, "Mar");
        else if (!strcmp(m, "04"))
            strcpy(month, "Apr");
        else if (!strcmp(m, "05"))
            strcpy(month, "May");
        else if (!strcmp(m, "06"))
            strcpy(month, "Jun");
        else if (!strcmp(m, "07"))
            strcpy(month, "Jul");
        else if (!strcmp(m, "08"))
            strcpy(month, "Aug");
        else if (!strcmp(m, "09"))
            strcpy(month, "Sep");
        else if (!strcmp(m, "10"))
            strcpy(month, "Oct");
        else if (!strcmp(m, "11"))
            strcpy(month, "Nov");
        else if (!strcmp(m, "12"))
            strcpy(month, "Dec");

        if (fulldate[2][0] == '0') {
            fulldate[2][0] = fulldate[2][1];
            fulldate[2][1] = '\0';
        }

        printf("%s %s %s, %s\n", fulldate[2], month, fulldate[0], lines[2]);
        
        free(lines);
        free(fulldate);
        sleep(t);
    }

    b_clock = 1;
    return 1;
}