// 1, 2, 5, 4, 6, 3 done (total 6)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

#define W_DELIM " \t\n\r\a"
#define MGN "\x1B[35m"
#define GREEN "\x1b[32m"
#define BLU "\x1b[34m"
#define RESET "\x1B[0m"

char HOME[250], bg_proc_name[100][100];
int bg_proc[100], bg_cnt = 0;
int b_clock = 1;

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


/* ---------------------- Processes ------------------- */
void sigint_handler(int sig) {
    int state;
    pid_t pid = waitpid(-1, &state, WNOHANG);
	
    if (pid != -1) {
		if(!WIFSIGNALED(state)) {
			for (int i = 0; i < bg_cnt; i++)
				if (pid == bg_proc[i]) {
					printf("\n[%d]  + %d done\t%s\n", i + 1, pid, bg_proc_name[i]);

                    bg_proc[i] = -1;
                    bg_cnt--;
					break;
				}
		}
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
            bg_proc[bg_cnt] = pid;
            strcpy(bg_proc_name[bg_cnt++], argv[0]);

            printf("[%d] %d\n", bg_cnt, pid);
            signal(SIGCHLD, sigint_handler);
            bg = 0;
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


void ls_utility(struct stat info, struct dirent *name) {
    struct passwd *pw = getpwuid(info.st_uid);
    
    char date[150];
    strftime(date, 100, "%h %d %H:%M", localtime(&(info.st_ctime)));
    
    struct group  *gr = getgrgid(info.st_gid);
    
    int d = S_ISDIR(info.st_mode);

    printf( (S_ISDIR(info.st_mode)) ? "d" : "-");
    printf( (info.st_mode & S_IRUSR) ? "r" : "-");
    printf( (info.st_mode & S_IWUSR) ? "w" : "-");
    printf( (info.st_mode & S_IXUSR) ? "x" : "-");
    printf( (info.st_mode & S_IRGRP) ? "r" : "-");
    printf( (info.st_mode & S_IWGRP) ? "w" : "-");
    printf( (info.st_mode & S_IXGRP) ? "x" : "-");
    printf( (info.st_mode & S_IROTH) ? "r" : "-");
    printf( (info.st_mode & S_IWOTH) ? "w" : "-");
    printf( (info.st_mode & S_IXOTH) ? "x" : "-");
    
    printf(" %d", (int)info.st_nlink);
    
    printf(" %s", pw->pw_name);
    printf(" %s", gr->gr_name);
    
    printf(" %8d", (int)info.st_size);
    printf(" %s", date);
    
    if (d) printf(BLU);
    printf(" %s", name->d_name);
    printf(RESET);

    printf("\n");
}


int ls(char **argv) {
    int a = 0, l = 0, dir_count = 0;

    DIR *dir;
    struct dirent *name;
    struct stat info;

    for (int i = 1; argv[i] != NULL; i++) {
        if (!strcmp(argv[i], "-a"))
            a = 1;
        else if (!strcmp(argv[i], "-l"))
            l = 1;
        else if (!strcmp(argv[i], "-al") || !strcmp(argv[i], "-la")) {
            a = 1;
            l = 1;
        }
        else if (strcmp(argv[i], " "))
            dir_count++;
    }

    int single = 0;
    if (!dir_count) {
        dir_count = 1;
        single = 1;
    }

    char dir_names[dir_count * 8][100];

    for (int i = 1, j = 0; argv[i] != NULL; i++)
        if (strcmp(argv[i], "-al") && strcmp(argv[i], "-la") && strcmp(argv[i], "-a") && strcmp(argv[i], "-l"))
            strcpy(dir_names[j++], argv[i]);

    if (single)
        strcpy(dir_names[0], ".");

    int i = 0;
    while (i < dir_count) {

        dir = opendir(dir_names[i]);
        
        if (dir == NULL) {
            fprintf(stderr, "%s: No such file or directory\n", dir_names[i++]);
            continue;
        }

        printf(GREEN "%s:\n" RESET, dir_names[i]);

        for (; (name = readdir(dir)) != NULL; ) {
            
            char buf[500];
            sprintf(buf, "%s/%s", dir_names[i], name->d_name);
            stat(buf, &info);
            
            if (!l && a) {
                if (S_ISDIR(info.st_mode)) printf(BLU);
                printf("%s\n" RESET, name->d_name);
            } else if (a && l) {
                ls_utility(info, name);
            } else if (!a && !l) {
                if(name->d_name[0] != '.') {
                    if (S_ISDIR(info.st_mode)) printf(BLU);
                    printf("%s\n" RESET, name->d_name);
                }
            } else {
                if (name->d_name[0] != '.')
                    ls_utility(info, name);
            }
        }
        i++;
    }
    closedir(dir);

    return 1;
}
/* ----------------------------------------------------*/


/* ---------------- Clock and consequences ------------------*/
void clock_handler(int signum) {
    b_clock = 0;
}

void exit_handler(int signum) {
    exit(EXIT_SUCCESS);
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
/* --------------------------------------------------------*/


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
    } else if (!strcmp("ls", argv[0])) {
        return ls(argv);
    } else if (!strcmp("clock", argv[0]) && !strcmp("-t", argv[1])) {
        return clock_builtin(argv);
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
    memset(bg_proc, -1, sizeof(bg_proc));

    int state;
    do {

        signal(SIGINT, exit_handler);

        cuserid(user);
        gethostname(host, 200);
        getcwd(curdir, 250);

        printf(MGN "<%s@%s:%s> " RESET, user, host, process_dir(curdir));
        
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
