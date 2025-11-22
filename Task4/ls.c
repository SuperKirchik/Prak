#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

void print_permissions(mode_t mode) {
    printf((S_ISDIR(mode)) ? "d" : "-");
    printf((mode & S_IRUSR) ? "r" : "-");
    printf((mode & S_IWUSR) ? "w" : "-");
    printf((mode & S_IXUSR) ? "x" : "-");
    printf((mode & S_IRGRP) ? "r" : "-");
    printf((mode & S_IWGRP) ? "w" : "-");
    printf((mode & S_IXGRP) ? "x" : "-");
    printf((mode & S_IROTH) ? "r" : "-");
    printf((mode & S_IWOTH) ? "w" : "-");
    printf((mode & S_IXOTH) ? "x" : "-");
}

void list_directory(const char *dirname, int l_flag, int g_flag, int R_flag) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    
    if ((dir = opendir(dirname)) == NULL) {
        perror("ls");
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        // Пропускаем скрытые файлы (кроме тех, что начинаются с .)
        if (entry->d_name[0] == '.' && !l_flag) continue;
        
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, entry->d_name);
        
        if (lstat(fullpath, &file_stat) == -1) {
            perror("lstat");
            continue;
        }
        
        if (l_flag) {
            print_permissions(file_stat.st_mode);
            printf(" %2lu", file_stat.st_nlink);
            
            struct passwd *pw = getpwuid(file_stat.st_uid);
            printf(" %s", pw ? pw->pw_name : "?");
            
            if (!g_flag) {
                struct group *gr = getgrgid(file_stat.st_gid);
                printf(" %s", gr ? gr->gr_name : "?");
            }
            
            printf(" %8ld", file_stat.st_size);
            
            char timebuf[80];
            struct tm *timeinfo = localtime(&file_stat.st_mtime);
            strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", timeinfo);
            printf(" %s", timebuf);
            
            printf(" %s", entry->d_name);
            
            // Показываем символические ссылки
            if (S_ISLNK(file_stat.st_mode)) {
                char linkbuf[1024];
                ssize_t len = readlink(fullpath, linkbuf, sizeof(linkbuf)-1);
                if (len != -1) {
                    linkbuf[len] = '\0';
                    printf(" -> %s", linkbuf);
                }
            }
            
            printf("\n");
        } else {
            printf("%s\n", entry->d_name);
        }
        
        // Рекурсивный обход для -R
        if (R_flag && S_ISDIR(file_stat.st_mode) && 
            strcmp(entry->d_name, ".") != 0 && 
            strcmp(entry->d_name, "..") != 0) {
            printf("\n%s:\n", fullpath);
            list_directory(fullpath, l_flag, g_flag, R_flag);
        }
    }
    
    closedir(dir);
}

int main(int argc, char *argv[]) {
    int l_flag = 0, g_flag = 0, R_flag = 0;
    char *dirname = ".";
    
    // Обработка флагов
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strchr(argv[i], 'l')) l_flag = 1;
            if (strchr(argv[i], 'g')) g_flag = 1;
            if (strchr(argv[i], 'R')) R_flag = 1;
        } else {
            dirname = argv[i];
        }
    }
    
    list_directory(dirname, l_flag, g_flag, R_flag);
    return 0;
}