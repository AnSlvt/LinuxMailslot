#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/sysmacros.h>
#include <string.h>
#include <sys/ioctl.h>

#define PATH_NAME "../mail"

#define IOCTL_MAGIC 0xF70000
#define SEQUENCE_CMD_BLOCKING 0x000100
#define SEQUENCE_CMD_NONBLOCKING 0x001000

#define BLOCKING_CMD IOCTL_MAGIC | SEQUENCE_CMD_BLOCKING | 0U
#define NONBLOCKING_CMD IOCTL_MAGIC | SEQUENCE_CMD_NONBLOCKING | 0U


int main(int argc, char const *argv[])
{
    int ret, fd, i;
    dev_t dev;
    char *buff;;

    dev = makedev(245, 0);

    ret = mknod(PATH_NAME, S_IRUSR | S_IWUSR | S_IFCHR, dev);
    if (ret == -1 && errno != EEXIST)
    {
        fprintf(stderr, "Something gone wrong, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    ret = open(PATH_NAME, O_RDWR);
    if (ret == -1)
    {
        fprintf(stderr, "open failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    fd = ret;

    while (1)
    {
        int command;
        scanf("%d", &command);
        if (command == 0)
        {
            ret = read(fd, buff, 1);
            printf("ret is %d\n", ret);
            if (ret == 0) printf("Nothing\n");
            else printf("%s\n", buff);
        }
        else break;
    }

    getchar();
    close(fd);

    return 0;
}
