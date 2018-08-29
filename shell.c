// 1, 2, 5 done

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define W_DELIM " \t\n\r\a"
char HOME[250];

char *get_input() {
    char *input = NULL;
    size_t sz = 0;
    getline(&input, &sz, stdin);
    return input;
}

char *process_dir(char *curdir) {
    char *ret = NULL;
    if (!strcmp(curdir, HOME))
        ret = "~";
    else {
        int h = strlen(HOME), c = strlen(curdir);
        if (c < h)
            ret = curdir;
        else {
            ret = (char *) malloc((c - h + 2) * sizeof(char));
            ret[0] = '~';
            
            int i = 0, j = 1;
            for (; curdir[i] == HOME[i]; i++);
            for (; curdir[i] != '\0'; i++, j++)
                ret[j] = curdir[i];
            ret[j] = '\0';
        }
    }
    return ret;
}

char **tokenize_input(char *input, char *delim) {

    int sz = 100;
    char *token, **ret = (char **) malloc(sz * sizeof(char *));

    if (!ret) {
        printf("ush: Allocation Error\n");
        return ret;
    }

    token = strtok(input, delim);
    for (int i = 0; token != NULL; i++) {
        if (i == sz) {
            sz += 100;
            char **new_ret = realloc(ret, sz);
            if (!new_ret) {
                printf("ush: Allocation Error\n");
                free(ret);
                break;
            }
        }
        ret[i] = token;
        token = strtok(NULL, delim);
    }
    return ret;
}


/* ----------------- Built in done -------------------*/
int pwd() {
    char cwd[250];
    getcwd(cwd, 250);
    printf("%s\n", cwd);
    return 1;
}

int cd(char **argv) {
    char *curdir = (char *) malloc(250 * sizeof(char));
    getcwd(curdir, 250); 
    if (argv[1]) {
        if (!strcmp(argv[1], "~"))
            chdir(HOME);
        else {
            if (!strcmp(curdir, HOME) && !strcmp(argv[1], "~"))
                return 1;
            if (chdir(argv[1]))
                perror("ush");
        }
    }
    return 1;
}

int echo(char **argv) {
    for (int i = 1; argv[i] != NULL; i++)
        printf("%s ", argv[i]);
    printf("\n");
    return 1;
}

int pinfo(char **argv) {
    // /proc/self/stat -> pid(0), status(1), vm(23)
    // **********/exe -> path(0)

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
    return 1;
}
/* ----------------------------------------------------*/


int execute(char **argv) {
    if (argv[0] == NULL) {
        return 1;
    } else if (!strcmp("pwd", argv[0])) {
        return pwd();
    } else if (!strcmp("cd", argv[0])) {
        return cd(argv);
    } else if (!strcmp("echo", argv[0])) {
        return echo(argv);
    } else if (!strcmp("pinfo", argv[0])) {
        return pinfo(argv);
    } else if (!strcmp("exit", argv[0]))
        return 0;
}

int main() {
    
    char *input, *EXIT = "exit", **argv, **commands;
    char user[100], host[200];

    getcwd(HOME, 250);
    char *curdir = (char *) malloc(250 * sizeof(char));

    int state;
    do {

        cuserid(user);
        gethostname(host, 200);
        getcwd(curdir, 250);

        printf("%s@%s:%s> ", user, host, process_dir(curdir));
        
        input = get_input();
        commands = tokenize_input(input, ";");


        for (int i = 0; commands[i] != NULL && state; i++) {
            argv = tokenize_input(commands[i], W_DELIM);
            state = execute(argv);
        }

        free(input);
        free(commands);

    } while (state);
    
    return 0;
}
