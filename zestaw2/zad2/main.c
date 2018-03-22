//
// Created by Michał Osadnik on 18/03/2018.
//

#include <time.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <limits.h>
#include <ftw.h>


const char format[] = "%Y-%m-%d %H:%M:%S";
int const buff_size = PATH_MAX;
char buffer[buff_size];
time_t gdate;
char *goperant;

double date_compare(time_t date_1, time_t date_2) {
    return difftime(date_1, date_2);
}

void print_info(const char *path, const struct stat *file_stat) {
    printf(" %lld\t", file_stat->st_size);
    printf((S_ISDIR(file_stat->st_mode)) ? "d" : "-");
    printf((file_stat->st_mode & S_IRUSR) ? "r" : "-");
    printf((file_stat->st_mode & S_IWUSR) ? "w" : "-");
    printf((file_stat->st_mode & S_IXUSR) ? "x" : "-");
    printf((file_stat->st_mode & S_IRGRP) ? "r" : "-");
    printf((file_stat->st_mode & S_IWGRP) ? "w" : "-");
    printf((file_stat->st_mode & S_IXGRP) ? "x" : "-");
    printf((file_stat->st_mode & S_IROTH) ? "r" : "-");
    printf((file_stat->st_mode & S_IWOTH) ? "w" : "-");
    printf((file_stat->st_mode & S_IXOTH) ? "x" : "-");
    strftime(buffer, buff_size, format, localtime(&file_stat->st_mtime));
    printf(" %s\t", buffer);
    printf(" %s\t", path);
    printf("\n");
}

static int nftw_display(const char *fpath, const struct stat *file_stat, int typeflag, struct FTW *ftwbuf) {
    struct tm mtime;

    (void) localtime_r(&file_stat->st_mtime, &mtime);

    if (typeflag != FTW_F) {
        return 0;
    }

    int comparison_result = date_compare(gdate, file_stat->st_mtime);
    if (!(
            (comparison_result == 0 && strcmp(goperant, "=") == 0)
            || (comparison_result > 0 && strcmp(goperant, "<") == 0)
            || (comparison_result < 0 && strcmp(goperant, ">") == 0)
    )) {
        return 0;
    }
    print_info(fpath, file_stat);
    return 0;
}


void file_follow(char *path, char *operant, time_t date) {
    if (path == NULL)
        return;
    DIR *dir = opendir(path);


    if (dir == NULL) {
        printf("%s\n", "error!");
        return;
    }

    struct dirent *rdir = readdir(dir);
    struct stat file_stat;

    char new_path[buff_size];

    while (rdir != NULL) {
        strcpy(new_path, path);
        strcat(new_path, "/");
        strcat(new_path, rdir->d_name);


        lstat(new_path, &file_stat); // with symlinks

        if (strcmp(rdir->d_name, ".") == 0 || strcmp(rdir->d_name, "..") == 0) {
            rdir = readdir(dir);
            continue;
        } else {
            if (S_ISREG(file_stat.st_mode)) {
                if (strcmp(operant, "=") == 0 && date_compare(date, file_stat.st_mtime) == 0) {
                    print_info(new_path, &file_stat);
                } else if (strcmp(operant, "<") == 0 && date_compare(date, file_stat.st_mtime) > 0) {
                    print_info(new_path, &file_stat);
                } else if (strcmp(operant, ">") == 0 && date_compare(date, file_stat.st_mtime) < 0) {
                    print_info(new_path, &file_stat);
                } else {
                    printf("%s\n", "Dunno this operator :o");
                    return;
                }
            }


            if (S_ISDIR(file_stat.st_mode)) {
                file_follow(new_path, operant, date);
            }
            rdir = readdir(dir);
        }
    }
    closedir(dir);
}



int main(int argc, char **argv) {

    if (argc < 4) {
        printf("Bad args!");
        return 1;
    }


    char *path = argv[1];
    char *operant = argv[2];
    char *usr_date = argv[3];

    struct tm *timestamp = malloc(sizeof(struct tm));

    strptime(usr_date, format, timestamp);
    time_t date = mktime(timestamp);


    DIR *dir = opendir(realpath(path, NULL));
    if (dir == NULL) {
        printf("couldnt open the directory\n");
        return 1;
    }


    file_follow(realpath(path, NULL), operant, date);

    printf("\n\n\n");
    gdate = date; // global args for nftw
    goperant = operant;
    printf("%s", "\n\n NFTW \n\n");
    nftw(realpath(path, NULL), nftw_display, 10, FTW_PHYS);

    closedir(dir);

    return 0;
}
