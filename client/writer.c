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
#define SEQUENCE_CMD_CHANGE_MSG_SIZE 0x001100

#define BLOCKING_CMD IOCTL_MAGIC | SEQUENCE_CMD_BLOCKING | 0U
#define NONBLOCKING_CMD IOCTL_MAGIC | SEQUENCE_CMD_NONBLOCKING | 0U
#define CHANGE_MSG_SIZE_CMD IOCTL_MAGIC | SEQUENCE_CMD_CHANGE_MSG_SIZE | 0U


int main(int argc, char const *argv[])
{
    int ret, fd, i;
    dev_t dev;
    char *buff = malloc(sizeof(256 * sizeof(char)));

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

    ioctl(fd, NONBLOCKING_CMD, 0);
    ioctl(fd, CHANGE_MSG_SIZE_CMD, 5);

    while (1)
    {
        scanf("%s", buff);
        ret = write(fd, buff, strlen(buff) + 1);
        if (ret == 0) printf("Message limit exceeded\n");
        else printf("Wrote!\n");
    }
    getchar();

    close(fd);

    return 0;
}
