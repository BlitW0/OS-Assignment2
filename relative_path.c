#include "main.h"

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
