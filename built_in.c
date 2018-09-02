#include "main.h"

int pwd() {
    char *cwd = (char *) malloc(250 * sizeof(char));
    getcwd(cwd, 250);
    printf("%s\n", cwd);
    free(cwd);
    return 1;
}

int cd(char **argv) {
    char *curdir = (char *) malloc(250 * sizeof(char));
    getcwd(curdir, 250); 
    if (argv[1] != NULL) {
        if (!strcmp(argv[1], "~"))
            chdir(HOME);
        else {
            if (!strcmp(curdir, HOME) && !strcmp(argv[1], "~"))
                return 1;
            if (chdir(argv[1]))
                perror("ush");
        }
    } else {
        chdir(HOME);
    }
    free(curdir);
    return 1;
}

int echo(char **argv) {
    for (int i = 1; argv[i] != NULL; i++)
        printf("%s ", argv[i]);
    printf("\n");
    return 1;
}

int execute(char **argv) {
    if (argv[0] == NULL) {
        return 1;
    }
    
    if (!strcmp("pwd", argv[0])) {
        return pwd();
    } else if (!strcmp("cd", argv[0])) {
        return cd(argv);
    } else if (!strcmp("echo", argv[0])) {
        return echo(argv);
    } else if (!strcmp("pinfo", argv[0])) {
        return pinfo(argv);
    } else if (!strcmp("ls", argv[0]) || !strcmp("l", argv[0])) {
        return ls(argv);
    } else if (!strcmp("clock", argv[0]) && !strcmp("-t", argv[1])) {
        return clock_builtin(argv);
    } else if (!strcmp("exit", argv[0])) {
        return 0;
    }

    return proc_launch(argv);
}
