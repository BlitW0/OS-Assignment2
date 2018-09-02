#include "main.h"

int bg_cnt = 0;
char bg_proc_name[100][100];

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
