#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/neutrino.h>
#include <sys/iomsg.h>

int
main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s PATH\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Open the file.
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    // Construct a read request message.
    struct _io_read msg = {
        .type = _IO_READ,
        .nbytes = 100
    };

    // Buffer to fill with reply.
    char buf[100];

    ssize_t nread = MsgSend(fd, &msg, sizeof(msg), buf, sizeof(buf));
    if (nread == -1) {
        perror("MsgSend");
        return EXIT_FAILURE;
    }

    printf("Read %zd bytes\n", nread);
    return EXIT_SUCCESS;
}
