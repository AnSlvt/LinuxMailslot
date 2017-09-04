#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/sysmacros.h>
#include <string.h>


#define PATH_NAME "../mail"


int main(int argc, char const *argv[])
{
    int ret, fd;
    dev_t dev;
    char *buff = "ciao";
    char read_value[5];

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
    ret = write(fd, buff, 5);
    ret = write(fd, buff, 5);
    ret = write(fd, buff, 5);

    ret = read(fd, read_value, 5);

    printf("Wrote %s get %s\n", buff, read_value);

    getchar();

    close(fd);

    return 0;
}