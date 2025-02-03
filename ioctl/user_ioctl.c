#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/mychardev"
#define IOCTL_MAGIC 'x'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC, 1, int)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC, 2, int)

int main() {
    int fd, value, ret;

    // Open the device file
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    // Set a value using ioctl
    value = 42;
    ret = ioctl(fd, IOCTL_SET_VALUE, &value);
    if (ret < 0) {
        perror("Failed to set value");
    } else {
        printf("Successfully set value: %d\n", value);
    }

    // Get the value using ioctl
    ret = ioctl(fd, IOCTL_GET_VALUE, &value);
    if (ret < 0) {
        perror("Failed to get value");
    } else {
        printf("Retrieved value: %d\n", value);
    }

    // Close the device file
    close(fd);
    return EXIT_SUCCESS;
}
