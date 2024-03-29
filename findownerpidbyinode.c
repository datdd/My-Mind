#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

static void find_owner_pid_by_inode_number(int inonum)
{
    const char *root = getenv("PROC_ROOT") ? : "/proc/";
    struct dirent *d;
    char name[1024];
    int nameoff;
    DIR *dir;

    strncpy(name, root, sizeof(name));

    if (strlen(name) == 0 || name[strlen(name)-1] != '/')
        strcat(name, "/");

    nameoff = strlen(name);

    dir = opendir(name);
    if (!dir)
        return;

    while ((d = readdir(dir)) != NULL)
    {
        struct dirent *d1;
        char process[16];
        char *p;
        int pid, pos;
        DIR *dir1;
        char crap;

        if (sscanf(d->d_name, "%d%c", &pid, &crap) != 1)
            continue;

        snprintf(name + nameoff, sizeof(name) - nameoff, "%d/fd/", pid);
        pos = strlen(name);
        if ((dir1 = opendir(name)) == NULL) {
            continue;
        }

        process[0] = '\0';
        p = process;

        while ((d1 = readdir(dir1)) != NULL)
        {
            const char *pattern = "socket:[";
            unsigned int ino;
            char lnk[64];
            char buf[1024];
            int fd;
            FILE *fp = NULL;
            ssize_t link_len;

            if (sscanf(d1->d_name, "%d%c", &fd, &crap) != 1)
                continue;

            snprintf(name+pos, sizeof(name) - pos, "%d", fd);

            link_len = readlink(name, lnk, sizeof(lnk)-1);
            if (link_len == -1)
                continue;
            lnk[link_len] = '\0';

            if (strncmp(lnk, pattern, strlen(pattern)))
                continue;

            sscanf(lnk, "socket:[%u]", &ino);

            if (ino == inonum)
            {
                snprintf(name + nameoff, sizeof(name) - nameoff, "%d/cmdline", pid);
                if ((fp = fopen(name, "r")) != NULL)
                {
                    memset(buf, 0, sizeof buf);
                    if (fgets(buf, sizeof buf, fp) != NULL)
                    {
                        printf("Found pid=%d, cmdline=%s\n", pid, buf);
                    }
                    fclose(fp);
                }
                else
                {
                    printf("Found pid=%d\n", pid);
                }
                
                break;
            }
        }
        closedir(dir1);
    }
    closedir(dir);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "%s: Invalid argument!\n", argv[0]);
        fprintf(stderr, "Usage: %s [inode]\n", argv[0]);
        return 0;
    }

    find_owner_pid_by_inode_number(atoi(argv[1]));

    return 0;
}
