#include "main.h"

int pinfo(char **argv) {

    char proc_path[400], info_path[500], sym_path[400], exe_path[500];

    strcpy(proc_path, "/proc/");
    strcat(proc_path, (argv[1]) ? argv[1] : "self");

    strcpy(info_path, proc_path);
    strcat(info_path, "/stat");
    
    int fd = open(info_path, O_RDONLY);
    if (fd == -1) {
        perror("ush");
        return 1;
    }

    char all[2048], **out;
    read(fd, all, 2048);
    out = tokenize_input(all, W_DELIM);
    close(fd);

    printf("pid -- %s\n", out[0]);
    printf("Process Status -- %s\n", out[2]);
    printf("Virtual Memory -- %s\n", out[22]);

    strcpy(sym_path, proc_path);
    strcat(sym_path, "/exe");

    int len = readlink(sym_path, exe_path, 500);
    if (len < 0) {
        strcpy(exe_path, "Link Broken");
    }
    exe_path[len] = '\0';
    printf("Executable Path -- %s\n", process_dir(exe_path));

    free(out);
    return 1;
}
