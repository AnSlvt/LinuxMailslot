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
#define SEQUENCE_CMD_BLOCKING 0x000001
#define SEQUENCE_CMD_NONBLOCKING 0x000010
#define SEQUENCE_CMD_CHANGE_MSG_SIZE 0x000011
#define SEQUENCE_CMD_INCREASE_MAX_MSGS 0x000100
#define SEQUENCE_CMD_DECREASE_MAX_MSGS 0x0000101
#define SEQUENCE_CMD_GET_MAX_MSGS 0x000110

#define BLOCKING_CMD IOCTL_MAGIC | SEQUENCE_CMD_BLOCKING | 0U
#define NONBLOCKING_CMD IOCTL_MAGIC | SEQUENCE_CMD_NONBLOCKING | 0U
#define CHANGE_MSG_SIZE_CMD IOCTL_MAGIC | SEQUENCE_CMD_CHANGE_MSG_SIZE | 0U
#define INCREASE_MAX_MSGS_CMD IOCTL_MAGIC | SEQUENCE_CMD_INCREASE_MAX_MSGS | 0U
#define DECREASE_MAX_MSGS_CMD IOCTL_MAGIC | SEQUENCE_CMD_DECREASE_MAX_MSGS | 0U
#define GET_MAX_MSGS_CMD IOCTL_MAGIC | SEQUENCE_CMD_GET_MAX_MSGS | 0U


int main(int argc, char const *argv[])
{
    int ret, fd, i, fails = 0;
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

    int max_msgs = ioctl(fd, GET_MAX_MSGS_CMD, 0);
    printf("There is space for %d messages in the mailslot\n", max_msgs);

    while (1)
    {
        max_msgs = ioctl(fd, GET_MAX_MSGS_CMD, 0);
        printf("There is space for %d messages in the mailslot\n", max_msgs);
        fflush(stdout);
        scanf("%s", buff);
        if (strcmp(buff, "decrease") == 0)
        {
            int n;
            scanf("%d", &n);
            int step_to_decrease = max_msgs - (max_msgs - n);
            for (i = 0; i < step_to_decrease; i++)
            {
                ret = ioctl(fd, DECREASE_MAX_MSGS_CMD, 0);
                if (ret == -1 && errno == EAGAIN) fails++;
            }

            if (fails != 0) printf("The size of the mailslot is decreased only by %d\n", step_to_decrease - fails);
        }
        else if (strcmp(buff, "increase") == 0)
        {
            int n;
            scanf("%d", &n);
            ret = ioctl(fd, GET_MAX_MSGS_CMD, 0);
            ret = ioctl(fd, INCREASE_MAX_MSGS_CMD, ret + n);
            if (ret == -1 && errno == EINVAL) printf("Too much to increase\n");
        }
        else
        {
            ret = write(fd, buff, strlen(buff) + 1);
            if (ret == -1)
            {
                if (errno == E2BIG) printf("Message limit exceeded\n");
                else if (errno == ENOSPC) printf("No space in the mailslot\n");
            }
            else printf("Wrote!\n");
        }
    }
    getchar();

    close(fd);

    return 0;
}
