#include "main.h"

void exit_handler(int signum) {
    exit(EXIT_SUCCESS);
}

int main() {
    
    char *input, *EXIT = "exit", **argv, **commands;
    char user[100], host[200];

    char *curdir = (char *) malloc(250 * sizeof(char));
    memset(bg_proc, -1, sizeof(bg_proc));

    char curhome[250];
    int len = readlink("/proc/self/exe", curhome, 250);

    if (len < 0)
        getcwd(HOME, 250);
    else {
        int i;
        for (i = strlen(curhome) - 1; curhome[i] != '/'; i--)
            curhome[i] = '\0';
        curhome[i] = '\0';
        
        strcpy(HOME, curhome);

        char temp[20];
        strcpy(temp, "cd ~");
        argv = tokenize_input(temp, W_DELIM);
        execute(argv);
        free(argv);
    }


    int state = 1;
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
