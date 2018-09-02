#include "main.h"

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

    if (!strcmp("l", argv[0]))
        a = l = 1;

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
