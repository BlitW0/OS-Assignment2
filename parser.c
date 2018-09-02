#include "main.h"

char *get_input() {
    char *input = NULL;
    size_t sz = 0;
    getline(&input, &sz, stdin);
    return input;
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
