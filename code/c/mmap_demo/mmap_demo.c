#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

int
main(int argc, char **argv)
{
    // Create a temporary file that can be mapped.
    char template[] = "mmap_demo.XXXXXX";
    int const fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp");
        return EXIT_FAILURE;
    }

    // Resize the file to hold just over three pages.
    if (ftruncate(fd, __PAGESIZE * 3 + 100) == -1) {
        perror("ftruncate");
        return EXIT_FAILURE;
    }

    struct {
        char   *desc;
        size_t  size;
        int     prot;
        int     flags;
        int     fd;
    } mappings[] = {
        {
            .desc = "Anonymous private",
            .size = __PAGESIZE * 10,
            .prot = PROT_READ | PROT_WRITE,
            .flags = MAP_PRIVATE | MAP_ANON,
            .fd = NOFD
        },
        {
            .desc = "Anonymous shared",
            .size = __PAGESIZE * 2,
            .prot = PROT_READ | PROT_WRITE,
            .flags = MAP_SHARED | MAP_ANON,
            .fd = NOFD
        },
        {
            .desc = "File-backed private",
            .size = __PAGESIZE * 4,
            .prot = PROT_READ | PROT_WRITE,
            .flags = MAP_PRIVATE,
            .fd = fd
        },
        {
            .desc = "File-backed shared",
            .size = __PAGESIZE * 4,
            .prot = PROT_READ | PROT_WRITE,
            .flags = MAP_SHARED,
            .fd = fd
        },
        {
            .desc = "Virtual address range only",
            .size = __PAGESIZE * 1000000UL,
            .prot = PROT_NONE,
            .flags = MAP_PRIVATE,
            .fd = NOFD
        }
    };

    // Map memory.
    for (unsigned i = 0; i < sizeof(mappings) / sizeof(mappings[0]); i++) {
        char * const ptr = mmap(0, mappings[i].size, mappings[i].prot,
                                mappings[i].flags, mappings[i].fd, 0);

        if (ptr == MAP_FAILED) {
            fprintf(stderr, "mmap(%s): %s\n", mappings[i].desc,
                    strerror(errno));
            return EXIT_FAILURE;
        }

        printf("%s mapping %p-%p\n", mappings[i].desc, ptr,
               ptr + mappings[i].size - 1);
    }

    pause();
    return EXIT_SUCCESS;
}
