#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/neutrino.h>
#include <sys/rpi_gpio.h>

enum {
    EVENT_TIMER,
    EVENT_BUTTON_1,
    EVENT_BUTTON_2
};

static int chid;
static int coid;
static timer_t timer_id;

static bool
init_channel(void)
{
    // Create a local channel.
    chid = ChannelCreate(_NTO_CHF_PRIVATE);
    if (chid == -1) {
        perror("ChannelCreate");
        return false;
    }

    // Connect to the channel.
    coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) {
        perror("ConnectAttach");
        return false;
    }

    return true;
}

static bool
init_timer(void)
{
    // Define and register a pulse event.
    struct sigevent timer_event;
    SIGEV_PULSE_INIT(&timer_event, coid, -1, _PULSE_CODE_MINAVAIL,
                     EVENT_TIMER);
    if (MsgRegisterEvent(&timer_event, coid) == -1) {
        perror("MsgRegisterEvent");
        return false;
    }

    // Create a timer and associate it with the first pulse event.
    if (timer_create(CLOCK_MONOTONIC, &timer_event, &timer_id) == -1) {
        perror("timer_create");
        return false;
    }

    return true;
}

static bool
init_gpio(int fd, int gpio, int event)
{
    // Configure as an input.
    rpi_gpio_msg_t msg = {
        .hdr.type = _IO_MSG,
        .hdr.subtype = RPI_GPIO_SET_SELECT,
        .hdr.mgrid = RPI_GPIO_IOMGR,
        .gpio = gpio,
        .value = RPI_GPIO_FUNC_IN
    };

    if (MsgSend(fd, &msg, sizeof(msg), NULL, 0) == -1) {
        perror("MsgSend(input)");
        return false;
    }

    // Configure pull up.
    msg.hdr.subtype = RPI_GPIO_PUD;
    msg.value = RPI_GPIO_PUD_UP;
    if (MsgSend(fd, &msg, sizeof(msg), NULL, 0) == -1) {
        perror("MsgSend(pud 16)");
        return false;
    }

    // Notify on a rising edge.
    rpi_gpio_event_t event_msg = {
        .hdr.type = _IO_MSG,
        .hdr.subtype = RPI_GPIO_ADD_EVENT,
        .hdr.mgrid = RPI_GPIO_IOMGR,
        .gpio = gpio,
        .detect = RPI_EVENT_EDGE_RISING,
    };

    SIGEV_PULSE_INIT(&event_msg.event, coid, -1, _PULSE_CODE_MINAVAIL,
                     event);
    if (MsgRegisterEvent(&event_msg.event, fd) == -1) {
        perror("MsgRegisterEvent");
        return false;
    }

    if (MsgSend(fd, &event_msg, sizeof(event_msg), NULL, 0) == -1) {
        perror("MsgSend(event)");
        return false;
    }

    return true;
}

int
main(int argc, char **argv)
{
    if (!init_channel()) {
        return EXIT_FAILURE;
    }

    if (!init_timer()) {
        return EXIT_FAILURE;
    }

    int fd = open("/dev/gpio/msg", O_RDWR);
    if (fd == -1) {
        perror("open");
        return false;
    }

    if (!init_gpio(fd, 16, EVENT_BUTTON_1)) {
        return EXIT_FAILURE;
    }

    if (!init_gpio(fd, 20, EVENT_BUTTON_2)) {
        return EXIT_FAILURE;
    }

    struct itimerspec ts = { .it_value.tv_sec = 5 };
    timer_settime(timer_id, 0, &ts, NULL);

    for (;;) {
        struct _pulse pulse;
        if (MsgReceivePulse(chid, &pulse, sizeof(pulse), NULL) == -1) {
            perror("MsgReceivePulse()");
            return EXIT_FAILURE;
        }

        if (pulse.code != _PULSE_CODE_MINAVAIL) {
            fprintf(stderr, "Unexpected pulse code %d\n", pulse.code);
            return EXIT_FAILURE;
        }

        switch (pulse.value.sival_int) {
        case EVENT_TIMER:
            printf("Press a button already!\n");
            break;

        case EVENT_BUTTON_1:
            printf("Thank you for pressing button 1!\n");
            break;

        case EVENT_BUTTON_2:
            printf("Thank you for pressing button 2!\n");
            break;
        }

        timer_settime(timer_id, 0, &ts, NULL);
    }

    return EXIT_SUCCESS;
}
