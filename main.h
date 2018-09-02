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

char HOME[250];
int bg_proc[100];

char *get_input();
char *process_dir(char *curdir);
char **tokenize_input(char *input, char *delim);
int proc_launch(char **argv);
int pwd();
int cd(char **argv);
int echo(char **argv);
int ls(char **argv);
int pinfo(char **argv);
int clock_builtin(char **argv);
int execute(char **argv);
