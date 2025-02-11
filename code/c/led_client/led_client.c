#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/neutrino.h>
#include <sys/rpi_gpio.h>

int
main(int argc, char **argv)
{
    // Connect to the GPIO server.
    int fd = open("/dev/gpio/msg", O_RDWR);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    // Configure GPIO 16 as an output.
    rpi_gpio_msg_t msg = {
        .hdr.type = _IO_MSG,
        .hdr.subtype = RPI_GPIO_SET_SELECT,
        .hdr.mgrid = RPI_GPIO_IOMGR,
        .gpio = 16,
        .value = RPI_GPIO_FUNC_OUT
    };

    if (MsgSend(fd, &msg, sizeof(msg), NULL, 0) == -1) {
        perror("MsgSend(output)");
        return EXIT_FAILURE;
    }

    // Set GPIO 16 to high.
    msg.hdr.subtype = RPI_GPIO_WRITE;
    msg.value = 1;

    if (MsgSend(fd, &msg, sizeof(msg), NULL, 0) == -1) {
        perror("MsgSend(high)");
        return EXIT_FAILURE;
    }

    usleep(500000);

    // Set GPIO 16 to low.
    msg.value = 0;

    if (MsgSend(fd, &msg, sizeof(msg), NULL, 0) == -1) {
        perror("MsgSend(low)");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
