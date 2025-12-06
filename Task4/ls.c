#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_FILES 1000
#define MAX_LINE 1024

int compare_by_permissions(const void *a, const void *b) {
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;
    
    // Берем первые 10 символов каждой строки
    char perms_a[11] = {0};
    char perms_b[11] = {0};
    
    strncpy(perms_a, str_a, 10);
    strncpy(perms_b, str_b, 10);
    
    // Сравниваем эти 10 символов
    return strcmp(perms_a, perms_b);
}


// Собираем строку вывода для файла
void build_output_line(char *buffer, size_t bufsize, 
                      const char *name, const char *fullpath, 
                      struct stat *info, int l_flag, int g_flag) {
    
    if (!l_flag) {
        // Простой вывод - только имя
        snprintf(buffer, bufsize, "%s", name);
        return;
    }
    
    // Детальный вывод
    char perms[11];
    snprintf(perms, sizeof(perms), "%c%c%c%c%c%c%c%c%c%c",
        (S_ISDIR(info->st_mode)) ? 'd' : '-',
        (info->st_mode & S_IRUSR) ? 'r' : '-',
        (info->st_mode & S_IWUSR) ? 'w' : '-',
        (info->st_mode & S_IXUSR) ? 'x' : '-',
        (info->st_mode & S_IRGRP) ? 'r' : '-',
        (info->st_mode & S_IWGRP) ? 'w' : '-',
        (info->st_mode & S_IXGRP) ? 'x' : '-',
        (info->st_mode & S_IROTH) ? 'r' : '-',
        (info->st_mode & S_IWOTH) ? 'w' : '-',
        (info->st_mode & S_IXOTH) ? 'x' : '-');
    
    char owner[32] = "?";
    struct passwd *pw = getpwuid(info->st_uid);
    if (pw) strncpy(owner, pw->pw_name, sizeof(owner)-1);
    
    char group[32] = "?";
    if (!g_flag) {
        struct group *gr = getgrgid(info->st_gid);
        if (gr) strncpy(group, gr->gr_name, sizeof(group)-1);
    }
    
    char timebuf[80];
    struct tm *timeinfo = localtime(&info->st_mtime);
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", timeinfo);
    
    char linkinfo[256] = "";
    if (S_ISLNK(info->st_mode)) {
        char linkbuf[1024];
        int len = readlink(fullpath, linkbuf, sizeof(linkbuf)-1);
        if (len != -1) {
            linkbuf[len] = '\0';
            snprintf(linkinfo, sizeof(linkinfo), " -> %s", linkbuf);
        }
    }
    
    // Собираем всю строку
    if (g_flag) {
        snprintf(buffer, bufsize, "%s %2lu %s %8ld %s %s%s",
            perms, (unsigned long)info->st_nlink, owner,
            (long)info->st_size, timebuf, name, linkinfo);
    } else {
        snprintf(buffer, bufsize, "%s %2lu %s %s %8ld %s %s%s",
            perms, (unsigned long)info->st_nlink, owner, group,
            (long)info->st_size, timebuf, name, linkinfo);
    }
}

void list_directory(const char *dirname, int l_flag, int g_flag, int R_flag) {
    DIR *dir;
    struct dirent *entry;
    char *output_lines[MAX_FILES];  // Массив строк вывода
    int line_count = 0;
    
    if ((dir = opendir(dirname)) == NULL) {
        perror("ls");
        return;
    }
    
    // 1. СОБИРАЕМ ВСЕ СТРОКИ ВЫВОДА
    while ((entry = readdir(dir)) != NULL && line_count < MAX_FILES) {
        if (entry->d_name[0] == '.' && !l_flag) continue;
        
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, entry->d_name);
        
        struct stat file_stat;
        if (lstat(fullpath, &file_stat) == -1) {
            perror("lstat");
            continue;
        }
        
        // Выделяем память под строку вывода
        output_lines[line_count] = malloc(MAX_LINE);
        if (!output_lines[line_count]) continue;
        
        // Собираем строку вывода
        build_output_line(output_lines[line_count], MAX_LINE,
                         entry->d_name, fullpath, &file_stat, l_flag, g_flag);
        
        line_count++;
        
        // Рекурсия для папок (делаем ДО сортировки текущей папки)
        if (R_flag && S_ISDIR(file_stat.st_mode) && 
            strcmp(entry->d_name, ".") != 0 && 
            strcmp(entry->d_name, "..") != 0) {
            closedir(dir);  // Закрываем текущую папку
            printf("\n%s:\n", fullpath);
            list_directory(fullpath, l_flag, g_flag, R_flag);
            dir = opendir(dirname);  // Снова открываем
            if (!dir) break;
        }
    }
    
    if (dir) closedir(dir);
    
    // 2. СОРТИРУЕМ СТРОКИ ВЫВОДА
    qsort(output_lines, line_count, sizeof(char*), compare_by_permissions);
    
    // 3. ВЫВОДИМ ОТСОРТИРОВАННЫЕ СТРОКИ
    for (int i = 0; i < line_count; i++) {
        printf("%s\n", output_lines[i]);
        free(output_lines[i]);  // Освобождаем память
    }
}


int main(int argc, char *argv[]) {
    int l_flag = 0, g_flag = 0, R_flag = 0;
    char *dirname = ".";
    
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