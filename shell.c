// 1, 2, 5, 4, 6 done (total 6)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

#define W_DELIM " \t\n\r\a"
#define MGN "\x1B[35m"
#define RESET "\x1B[0m"

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

    int sz = 100, i;
    char *token, **ret = (char **) malloc(sz * sizeof(char *));

    if (!ret) {
        printf("ush: Allocation Error\n");
        return ret;
    }

    token = strtok(input, delim);
    for (i = 0; token != NULL; i++) {
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
    ret[i] = NULL;
    return ret;
}


/* --------------- Processes --------------- */
void sigint_handler(int sig) {
    pid_t pid = wait(NULL);
    if (pid != -1) {
        printf("\n[%d] exited with status %d.\n", pid, WEXITSTATUS(pid));
    }
}


int proc_launch(char **argv) {

    int bg = 0;
    for (int i = 0; argv[i] != NULL; i++) {
        int len = strlen(argv[i]);
        if (argv[i][len - 1] == '&') {
            bg = 1;
            argv[i][len - 1] = '\0';
            if (argv[i][len - 2] == ' ')
                argv[i][len - 2] = '\0';
        }
    }
    
    signal(SIGCHLD, sigint_handler);
    pid_t pid = fork(), wpid;
    int state;

    if (pid == -1)
        perror("ush");
    else if (!pid) {
        if (execvp(argv[0], argv) == -1)
            perror("ush");
        exit(EXIT_FAILURE);
    } else {
        if (!bg) {
            wpid = waitpid(pid, &state, WUNTRACED);
            while (1) {
                if (!WIFEXITED(state) && !WIFSIGNALED(state)) {
                    wpid = waitpid(pid, &state, WUNTRACED);
                    continue;
                }
                break;
            }
        } else {
            printf("\n[%d] started\n\n", pid);
            waitpid(pid, &state, WNOHANG);
        }
    }

    return 1;
}
/* ----------------------------------------------------- */



/* ----------------- Built in done -------------------*/
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
/* ----------------------------------------------------*/


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
    } else if (!strcmp("exit", argv[0])) {
        return 0;
    }

    return proc_launch(argv);
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

        printf(MGN "%s@%s:%s> " RESET, user, host, process_dir(curdir));
        
        input = get_input();
        commands = tokenize_input(input, ";");

        for (int i = 0; commands[i] != NULL && state; i++) {
            argv = tokenize_input(commands[i], W_DELIM);
            state = execute(argv);
            free(argv);
        }

        free(input);
        free(commands);

    } while (state);
    
    return 0;
}
