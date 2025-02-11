#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <aarch64/rpi_gpio.h>

static uint32_t volatile *gpio_regs;

int
main(int argc, char **argv)
{
    // Map the GPIO registers.
    gpio_regs = mmap(0, __PAGESIZE, PROT_READ | PROT_WRITE | PROT_NOCACHE,
                     MAP_SHARED | MAP_PHYS, -1, 0xfe200000);
    if (gpio_regs == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }

    // Configure GPIO 16 as an output.
    gpio_regs[RPI_GPIO_REG_GPFSEL1] &= ~(7 << 18);
    gpio_regs[RPI_GPIO_REG_GPFSEL1] |= (1 << 18);

    int led_state = 0;
    for (;;) {
        if (led_state == 0) {
            // Set GPIO 16 to high.
            gpio_regs[RPI_GPIO_REG_GPSET0] = (1 << 16);
        } else {
            // Set GPIO 16 to low.
            gpio_regs[RPI_GPIO_REG_GPCLR0] = (1 << 16);
        }

        led_state = !led_state;
        sleep(1);
    }

    return EXIT_SUCCESS;
}
