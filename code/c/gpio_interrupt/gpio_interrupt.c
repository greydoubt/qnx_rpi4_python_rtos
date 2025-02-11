#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <aarch64/rpi_gpio.h>

static uint32_t volatile *gpio_regs;

static bool
init_gpios()
{
    // Map the GPIO registers.
    gpio_regs = mmap(0, __PAGESIZE, PROT_READ | PROT_WRITE | PROT_NOCACHE,
                     MAP_SHARED | MAP_PHYS, -1, 0xfe200000);
    if (gpio_regs == MAP_FAILED) {
        perror("mmap");
        return false;
    }

    // Configure GPIO 16 as an output.
    gpio_regs[RPI_GPIO_REG_GPFSEL1] &= ~(7 << 18);
    gpio_regs[RPI_GPIO_REG_GPFSEL1] |= (1 << 18);

    // Configure GPIO 20 as a pull-up input.
    gpio_regs[RPI_GPIO_REG_GPFSEL2] &= ~(7 << 0);
    gpio_regs[RPI_GPIO_REG_GPPUD1] &= ~(3 << 8);
    gpio_regs[RPI_GPIO_REG_GPPUD1] |= (1 << 8);

    // Clear and disable all events.
    gpio_regs[RPI_GPIO_REG_GPEDS0] = 0xffffffff;
    gpio_regs[RPI_GPIO_REG_GPEDS1] = 0xffffffff;
    gpio_regs[RPI_GPIO_REG_GPREN0] = 0;
    gpio_regs[RPI_GPIO_REG_GPREN1] = 0;
    gpio_regs[RPI_GPIO_REG_GPFEN0] = 0;
    gpio_regs[RPI_GPIO_REG_GPFEN1] = 0;
    gpio_regs[RPI_GPIO_REG_GPHEN0] = 0;
    gpio_regs[RPI_GPIO_REG_GPHEN1] = 0;
    gpio_regs[RPI_GPIO_REG_GPLEN0] = 0;
    gpio_regs[RPI_GPIO_REG_GPLEN1] = 0;

    // Detect falling and rising edge events on GPIO 20.
    gpio_regs[RPI_GPIO_REG_GPREN0] |= (1 << 20);
    gpio_regs[RPI_GPIO_REG_GPFEN0] |= (1 << 20);

    return true;
}

int
main(int argc, char **argv)
{
    if (!init_gpios()) {
        return EXIT_FAILURE;
    }

    // Attach to the GPIO interrupt without unmasking.
    int intid = InterruptAttachThread(145, _NTO_INTR_FLAGS_NO_UNMASK);
    if (intid == -1) {
        perror("InterruptAttachThread");
        return EXIT_FAILURE;
    }

    for (;;) {
        // Unmask the interrupt and wait for the next one.
        if (InterruptWait(_NTO_INTR_WAIT_FLAGS_FAST |
                          _NTO_INTR_WAIT_FLAGS_UNMASK, NULL) == -1) {
            perror("InterruptWait");
            return EXIT_FAILURE;
        }

        // Check for an event on GPIO 20.
        if ((gpio_regs[RPI_GPIO_REG_GPEDS0] & (1 << 20)) != 0) {
            if ((gpio_regs[RPI_GPIO_REG_GPLEV0] & (1 << 20)) == 0) {
                // GPIO 20 is low, set GPIO 16 to high.
                gpio_regs[RPI_GPIO_REG_GPSET0] = (1 << 16);
            } else {
                // GPIO 20 is high, set GPIO 16 to high.
                gpio_regs[RPI_GPIO_REG_GPCLR0] = (1 << 16);
            }
        }

        // Clear any detected events before unmasking the interrupt.
        gpio_regs[RPI_GPIO_REG_GPEDS0] = 0xffffffff;
        gpio_regs[RPI_GPIO_REG_GPEDS1] = 0xffffffff;
    }

    return EXIT_SUCCESS;
}
