---
title: Real-Time Programming in C
author: Elad Lahav
chapter: 5
---

# Real-Time Programming in C {#chapter:realtime}

## Building C Programs {#sec:build_c}

Programs written in C need to be compiled and linked before they can be run on
the Raspberry Pi. There are a few methods for doing that when using the QNX
build tools. For the following discussion we will use the ubiquitous "Hello
World" example:

~~~ {#hello.c .c .numberLines}
#include <stdio.h>

int
main()
{
    printf("Hello World!\n");
    return 0;
}
~~~

### Command Line

The compiler and linker front-end for QNX is called **qcc**. Front-end means
that this tool is really a wrapper around the compiler and linker provided by
the GNU project. As **qcc** can generate binaries and libraries for
different targets it needs to be told the combination of target and tools.

```
$ qcc -Vgcc_ntoaarch64le -o hello hello.c
```

This command will build the code in **hello.c** into the executable
**hello**. Once built, the executable can be copied to the Raspberry Pi and
run:

```
$ scp hello qnxuser@qnxpi:
$ ssh -t qnxuser@qnxpi hello
```

or, from a QNX shell (after copying the executable to the Raspberry Pi):

```
qnxuser@qnxpi:~$ ./hello
Hello World!
```

Just as with other compilers, multiple source files can be specified on the
command line to be linked together. Alternatively, the `-c` option can be
used just to compile one source file into an object, and then the objects linked
together.

### Recursive Make

Of course, compiling multiple source files can quickly become tedious and error
prone. The traditional UNIX `make` program can be used to keep track of
multiple source files in a project, simplifying the task of building a complex
executable. You can write your own make file using the standard language for
such files. Alternatively, you can use the QNX recursive make system, a
set of macros that facilitate the building of projects for QNX targets. While
this system's greatest strength lies in its ability to create binaries for
different targets from the same source code, it also simplifies the building of
simple projects with just one target, by choosing the right tools and options.

The recursive make project for our hello world example consists of a top-level
directory, an architecture-level directory and a variant-level directory. As we
are only interested in building the program for the Raspberry Pi, the
architecture level directory is **aarch64** and the variant level is
**le**.[^5.1] We could add a **linux-x86_64** directory at the architecture
level with a **o** directory under it to build the same program for a Linux
host.

```
$ tree hello
hello
|-- Makefile
|-- aarch64
|   |-- Makefile
|   `-- le
|       `-- Makefile
|-- common.mk
`-- hello.c
```

The **common.mk** file includes the files necessary for the recursive make
system, along with definitions for the project as a whole (such as the name of
the executable). A bare-bones file for our example looks like this:

```
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=hello
USEFILE=

include $(MKFILES_ROOT)/qtargets.mk
```

The top-level make files simply tell make to continue down the hierarchy, while
the bottom level file (the one under **hello/aarch64/le**) includes
**common.mk**. It is the names of the directories that tell the recursive
system what is the target for the binaries built under them.

We can now go into any of these directories and type `make` to build the
executables for the targets covered by this level of the hierarchy. As we have
only one target it doesn't matter which one we use.

### VSCode

To use VSCode for QNX C/C++ development you can open an existing folder with
pre-created make files (as in the example above), or you can create a new
project directly in VSCode. To do that, open the command palette (Ctrl-Shift-P)
and type "QNX: Create New QNX Project" (the command will be completed soon
after you start typing). Choose a name, the make system to use and a default
language. A project with a simple `main()` function is initialized and
ready to be built.

Next, make sure that the Raspberry Pi is set as the default target. The list of
targets is available in the QNX pane, which is made visible by clicking on the
QNX plugin button. Once the default target is set you can launch the program on
the Raspberry Pi. From the command palette choose "QNX: Run as QNX
Application". The output should be visible in the debug console below. You can
also use VSCode to debug the program.

## Inter-Process Communication {#sec:ipc}

Inter-Process Communication, or IPC, is a feature of operating systems that
allows two processes, each running in its own private virtual address space, to
interact. Shared memory, shared semaphores, pipes, sockets and even files
accessible to multiple processes, can all be used as forms of IPC.

While there are use cases for inter-process communication on any operating
system, IPC is an essential feature for an operating system based on a
microkernel, such as QNX. No useful work can be done in such an operating
system without communicating with the various services that are implemented by
other processes. Allocating memory, writing to a file, sending a packet on the
network, getting the coordinates from a USB mouse or displaying an image on the
screen are all performed by talking to the various services that implement these
features using IPC.

The most basic form of IPC in the QNX RTOS is synchronous message passing, also
referred to as Send/Receive/Reply. Almost all of the scenarios mentioned above
for the use of IPC in a microkernel operating system are implemented in QNX by
the use of such messages. In fact, some of the other IPC mechanisms are built on
top of synchronous message passing.

With synchronous message passing, one process, known as the *client*, requests a
service from another process, known as the *server*, by sending it a message,
and then waiting for a reply. The server receives the message, handles it, and
then replies to the waiting client. Note that when we say that the client is
waiting, we are only referring to one thread in the client that has sent the
message to the server. Other threads in the client process can continue running
while one thread is waiting for a reply.

The main advantage of synchronous message passing over other forms of IPC is
that it does not queue messages. Consequently, there are no limits on the size
of messages.[^5.2] Also, synchronous message passing is self-throttling, making
it harder for the client to flood the server with IPC requests at a higher rate
than the server can handle. Finally, QNX message passing provides a
scatter/gather feature, in which all the buffers used during the message pass
(client's send and reply buffers, and the server's receive buffer) can be
specified as arrays of sub-buffers, known as I/O vectors (IOV). Each of these
sub-buffers consists of a base address and a length. With IOVs, messages can be
assembled without first copying the components into a single buffer.

Another feature of synchronous message passing, which is absent from most other
forms of IPC, is *priority inheritance*. When a client thread at priority 20
sends a message to a server, then the server thread that receives the message
will have its priority changed to 20, for the duration of the time it handles
the message. For more information on priorities see
[Threads and Priorities](#sec:threads).

Before a client can send a message to a server, it needs to establish a
communication conduit to that server. The server creates one or more
*channels*, on which threads are waiting to receive messages. The use of
channels as the end points of IPC rather than the server process itself allows
the server to have multiple such end points, which it can then use either for
different services (e.g., a file system process can have a separate channel for
each mount point) or for different quality of service (e.g., separating services
to privileged clients from those to non-privileged clients). Once a channel is
created, via the `ChannelCreate()` kernel call, the client can establish
a *connection* to that channel, using the `ConnectAttach()` kernel
call.

The `ConnectAttach()` call takes the server's process ID and the
channel ID, and returns a connection ID for use with calls to
`MsgSend()`. However, the requirement to know the identifiers of the
server process and its channel make it hard to write client code: these
identifiers may be different on different systems, or even across boots of the
same system. Consequently, we need a mechanism by which a server can advertise
itself, and which the client can use to discover it. That mechanism is provided
by paths: a server can associate its channel with a unique path name, which is
registered and handled by the path manager
(see [The File System](system.md#sec:file-system)).
The client can then call `open()` on that path, which performs the following
actions:

1. Sends a message to the path manager to inquire about the path. The path
   manager responds with the process and channel identifiers for the
   corresponding server channel.
2. Calls `ConnectAttach()` using the provided identifiers.

The resulting connection identifier is the QNX implementation of a *file
descriptor*.

The following program demonstrates a client connecting to the GPIO server, and
then using messages to control a GPIO. Rebuild the circuit from
[Basic Output (LED)](python.md#sec:basic_output) to see the program turning an
LED on and off.

~~~ {#led_client.c .c .numberLines}
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
~~~

Line 11 establishes the connection to the GPIO server. Line 18 declares a
message structure with the type expected by this server. The first message,
which is sent on line 26, uses the `RPI_GPIO_SET_SELECT` subtype and
the value `RPI_GPIO_FUNC_OUT` to tell the server to configure GPIO 16
as an output. Lines 32-35 reuse the same message structure to tell the server to
set GPIO 16 to high (note that the other structure fields are unchanged), while
lines 43-45 use the same structure to set the GPIO to low.

It may seem from this example that messages must have a specific structure. The
QNX kernel does not impose any such structure on messages, and treats these as
raw buffers to be copied from client to server and back. It is only the server
that imposes semantics on these raw buffers, and each server can define its own
expected structures for the various types of messages it handles.

As mentioned above, many of the standard library functions in QNX are
implemented as message passes to the appropriate servers. The following example
shows how to read a file directly with a message to the server that provides the
file. When the call to `MsgSend()` returns, the read bytes are in the reply
buffer. Of course, in this example there is no advantage to the direct use of
`MsgSend()`, and code would normally call `read()` instead.

~~~ {#read_file.c .c .numberLines}
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
~~~

## Threads and Priorities {#sec:threads}

### What are Threads?

Every processor has a fixed set of registers, which store numeric values. One of
these registers is the *instruction pointer* (also called *program
  counter*). When a processor executes, it fetches the next instruction from
memory based on the address stored in the instruction pointer, decodes it,
updates registers according to the instruction (e.g., adds the values from two
registers and stores the result in a third register), and then updates the
instruction pointer such that it points to the next instruction. That next
instruction is typically the one that follows the current instruction in memory,
but it can also be in a different memory location if the processor has just
executed a branch instruction.

The values stored in the processor's registers at any given point in time
provide the processor with its *execution context*. We can think of the
processor as executing a stream of instructions that updates this context as it
goes along from one instruction to the next. Many simple micro-controllers only
have just one such stream of execution, typically an infinite loop for the
control of some device. The micro-controller follows that loop and does nothing
else.

Computer systems that are not simple micro-controllers require multiple streams
of execution. An example of the use of a separate execution stream is an
exception: when the processor encounters an instruction such as a trap call, it
jumps to a predefined location and executes the code in that location. It is
expected that, once the exception is handled, the previous stream of execution
resumes from where it left (though potentially the context is modified to
reflect the results of exception handling). In order to resume the previous
execution stream, the exception handler needs first to save, and then to restore,
the execution context of the processor at the time the exception occurred.

A stream of execution that can be suspended and then restored is called a
*thread*. In its most basic form, a thread provides storage for a processor's
execution context. The system (typically the operating system kernel) copies the
execution context from the processor to the thread's storage when a thread is
suspended (such as when a processor jumps to an exception handler), and copies
back the stored context to the processor when the thread is resumed.

The use of threads provides several important features in a computer system:

1. **Time Sharing** Different programs can make use of different threads in
   order to execute concurrently. The operating system alternates the execution
   of threads from all running programs, giving the illusion that the programs
   are running in parallel.[^5.3]
2. **Processor Utilization** A program often needs to wait for some event
   to occur (e.g., a block read from a hard drive, user input, or a sensor
   detecting activity). Threads allow for a different execution stream to proceed
   while one is waiting, preventing the processor from going idle while the
   system has work to do.
3. **Non-Blocking Execution** Closely related to the previous point, a
   single program can make use of threads to turn a blocking operation into a
   non-blocking one. For example, a device that does not respond to a request
   until it is ready can be interacted with using one thread, while another
   thread in the same program continues.
4. **Low Latency** Threads are crucial for a real-time operating system
   to provide timing guarantees on low-latency operations. When an event occurs,
   such as an interrupt raised from an external device, or a timer expiring, the
   current thread can be suspended, and a new one put into execution to handle
   the event. Such replacement of one thread by another in response to an event
   is called *preemption*. Without preemption event handling would have to
   wait until the current thread suspends itself, which may interfere with the
   timely response to the event.

:::::: Note ::::::
The discussion so far has made no reference to processes. Some
textbooks refer to threads as light-weight processes, but that is an
anachronism: old UNIX (and UNIX-like) systems had one stream of execution per
process, and new processes had to be created for each of the use cases described
above. The introduction of multiple such streams per process helped reduce the
overhead in some of these cases, as the system no longer needed to allocate the
full resources for a process just to have another stream of execution within the
same logical program. But such a definition of a thread misses the essential
point of this construct. Even in systems that do not support multiple threads
per process there are still multiple threads, one for each process, and these
threads are distinct entities. When the operating system scheduler puts a stream
into execution, it schedules a thread, not a process.
::::::::::::::::::

### Thread Scheduling

How does the system determine the next thread to execute? That is the task of
the *scheduler*, which is a part of the operating system kernel.[^5.4]
The scheduler uses a scheduling policy to decide which, among all the threads
that are currently ready for execution (i.e., threads that are not waiting for
some event to occur and can proceed with their work), is the thread most
eligible to run next, and load its stored context into the processor. Different
operating systems provide different scheduling policies. Two common policies
are:

* *FIFO* (first-in, first-out), in which the thread that has been
   ready for the longest time executes next, and then runs until it has to wait; and
* *round-robin*, which alternates between threads, allowing each a
   fixed time period to execute (its time slice).

Neither of these policies by itself is a good choice for a complex operating
system. FIFO depends on a thread to relinquish the processor before any other
activity can proceed, which means that other threads may have to wait
indefinitely. Round-robin alleviates the problem only to a degree, but critical
system work still has to wait for all the time slices of all the ready threads
ahead of it.

QNX, like other real-time operating systems, supports *priority scheduling*.
Each thread is associated with a priority value, which is a number between 0
(lowest) and 255 (highest). When a scheduling decision is performed, the
scheduler chooses the thread with the highest priority that is ready to
run. Within each priority level, threads are scheduled according to which has
been ready for the longest time.

Priority 0 is reserved for the idle threads. These are the threads that run when
the processor has nothing else to do. Typically these will execute the
architecture-specific halt instruction, which can reduce the power consumption
of the processor when it does not need to do anything. Priority 255 is reserved
for the inter-processor interrupt (IPI) thread. IPIs are used in a
multi-processor system to communicate between the different processors, and are
required for distributing work. Additionally, one priority level is assigned to
the clock interrupt handler, which handles timers. By default this priority is
254, but can be lowered with a command-line option to the kernel. If lowered,
then the priorities between the clock thread and the IPI thread can be used for
the lowest-latency threads, as long as these do not require software timers.

The assignment of priorities to threads is one of the most important tasks when
designing a complete system. Clearly this cannot be a free-for-all, or all
programs would assign the highest priority to all (or most) of their
threads.[^5.5] The following are some best practices when deciding on priorities:

* The threads with the highest priority in the system are those that require
  the shortest response time to events (i.e., the lowest latency). These are not
  (necessarily) the most *important* threads in the system, and are
  certainly not the most demanding in terms of throughput.
* The higher the priority of a thread, the shorter its execution time should
  be before it blocks waiting for the next event. High-priority threads running
  for long periods of time prevent the processor from doing anything else.
* The above restriction should be enforced by the system. In QNX the
  priority range is divided into a non-privileged range and a privileged range
  (by default 1-63 and 64-254). Only trusted programs should be given the
  ability to use privileged priorities. Also, the use of a software watchdog to
  detect a misbehaving high-priority thread is recommended.

The following example shows a program that creates 11 threads, in addition to
the main thread. Ten of these threads are worker threads, which calculate the
value of $\pi$, while another thread sleeps for one second and then prints the
number of microseconds that have elapsed since the last time it woke up. Running
this example should show that the high-priority thread executes consistently
within one millisecond of the expected time.[^5.6] Now comment out the call to
`pthread_attr_setschedparam()` so that the high-priority thread is reduced to
the default priority, and observe the effect.

~~~ {#threads.c .c .numberLines}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

static void *
worker(void *arg)
{
    double pi = 0.0;
    double denom = 1.0;

    for (unsigned i = 0; i < 100000000; i++) {
        pi += 4.0 / denom;
        pi -= 4.0 / (denom + 2.0);
        denom += 4.0;
    }

    printf("pi=%f\n", pi);
    return NULL;
}

static void *
high_priority(void *arg)
{
    unsigned long last = clock_gettime_mon_ns();
    for (;;) {
        sleep(1);
        unsigned long now = clock_gettime_mon_ns();
        printf("Slept for %luus\n", (now - last) / 1000UL);
        last = now;
    }

    return NULL;
}

int
main(int argc, char **argv)
{
    // Attribute structure for a high-priority thread.
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    struct sched_param sched = { .sched_priority = 63 };
    pthread_attr_setschedparam(&attr, &sched);

    // Create the high-priority thread.
    pthread_t tids[11];
    int rc;
    rc = pthread_create(&tids[0], &attr, high_priority, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(rc));
        return EXIT_FAILURE;
    }

    // Create worker threads.
    for (unsigned i = 1; i < 11; i++) {
        rc = pthread_create(&tids[i], NULL, worker, NULL);
        if (rc != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(rc));
            return EXIT_FAILURE;
        }
    }

    // Wait for workers to finish.
    for (unsigned i = 1; i < 11; i++) {
        pthread_join(tids[i], NULL);
    }

    return EXIT_SUCCESS;
}
~~~

## Timers {#sec:timers}

Keeping track of time is a requirement of many programs: refreshing the screen,
re-transmitting network packets, detecting deadlines and controlling devices
with strict timing restrictions are all examples of such a requirement. Given
that hardware timers are few and demand is high, the operating system needs to
multiplex software timers on top of those provided by the hardware. A program
can create multiple timers and associate each of those with an event to be
emitted whenever that timer expires. The program can then ask the system,
individually for each of the timers it created, to expire the timer at some
point in the future.

The QNX RTOS (as of version 8.0) makes use of a per-processor hardware
timer. When a program requests that one of its software timers expire in the
future, that request is enqueued for the hardware timer associated with the
processor on which the request was made. When the expiration time is reached,
that hardware timer raises an interrupt, which is handled by the clock interrupt
thread on the same processor. That thread then emits the event associated with
the software timer, which notifies the program that the timer has expired.

Timers are created with the `timer_create()` function, which takes the
clock ID and an event to be associated with the timer. The clock ID represents
the underlying clock to which expiration times for the timer correspond. Two
common clocks are `CLOCK_MONOTONIC` and `CLOCK_REALTIME`. Both
of these are tied to the hardware clock, which ticks constantly, but the latter
is affected by changes to the system's notion of the time of day, while the former
is not. Another option for a clock is the runtime of a thread or a process (the
latter is the accumulation of thread runtime for all threads in the
process). This option is less common, and is typically used to monitor thread
execution. The event associated with the timer can be any of the types that are
allowed by the `sigevent` structure, including a pulse, a signal,
posting a semaphore, creating a thread[^5.7] or updating a memory location.

The standard timer resolution in a QNX system is the tick length, which is by
default one millisecond. Software timers that use this resolution are referred
to as *tick-aligned timers*. This means that these timers will expire on the
next logical tick after their real expiration time. Aligning timers on a logical
tick avoids excessive context switches as multiple timers expire in close
succession.

:::::: Note ::::::
As of QNX 8.0, the system is tickless by default, which means that there is no
recurring tick if no timer expires within the next tick period. Standard timers
are still aligned to a *logical* tick, which is a multiple of the number of
clock cycles in a tick period since the system's boot time.
::::::::::::::::::

The system also provides *high-resolution timers*, which expire as soon as
the specified time was reached (subject to the granularity of the hardware
timer). High-resolution timers should be used sparingly[^5.8] and with care, as
frequent expiration of such timers can prevent the system from making forward
progress. For this reason the creation of such timers is a privileged operation.

Once a timer has been created it can be programmed using
`timer_settime()`. The function can be used to set the expiration time
in either absolute or relative terms (both correspond to the clock used to
create the timer), and as either a single-shot or a recurring timer. An example
of using a timer will be given in the next section.

## Event Loops {#sec:event_loops}

Real-time operating systems are often used to run programs for controlling
external devices, such as robots, factory machinery or irrigation systems. All
but the most simple devices provide multiple inputs, to which the
control program needs to respond. One way to handle such multiple inputs is
to have a separate thread for each one. A common alternative is to write the control
program around an event loop: each input provides its own event, which the
program handles in turn. These two approaches can also be combined in various
ways. For example, a control program can have a pool of threads, each running an
event loop.

An event loop consists of three phases:

1. Wait for any event to occur.
2. Decode the event.
3. Handle the event.

In C code, an event loop will have the following structure:

~~~{.c .numberLines}
void
event_loop()
{
    event_t event;

    for (;;) {
        wait_for_event(&event);
        switch (event_type(event)) {
        case EVENT_TYPE_1:
            handle_event_1(event);
            break;

        case EVENT_TYPE_2:
            handle_event_2(event);
            break;

        ...
        }
    }
}
~~~

(This example does not reflect a real API, but simply illustrates the structure
common to various event loop implementations. The switch statement may be
replaced by if-else blocks, or by a table of function pointers, but such choices
do not alter the fundamental structure.)

Various operating systems have come up with different types of API data
structures and functions for implementing event loops. In UNIX and UNIX-like
operating systems the two common interface functions have traditionally been
`select()`, and `poll()`. These functions have been recognized as
deficient for a long time, both in terms of scalability and race conditions.
Systems such as Linux and FreeBSD attempt to overcome their inherent
limitations with less-standard approaches, such as `epoll` and
`kqueue`, respectively.

While `select()` and `poll()` are available from the QNX C library, the inherent
limitations of these functions make them less than ideal choices for
implementing event loops. A much better choice is to build the loop around a
call to `MsgReceivePulse()` and use pulses as the event notification
mechanism. A *pulse* is a fixed-sized structure with a small payload (a one-byte
code and an 8-byte value) that can be emitted in response to various events,
including, but not limited to, an interrupt firing, a timer expiring, a pipe
changing its state from empty to non empty, and a socket being ready to deliver
the next packet. The program that implements the event loop requests to be
notified via pulses by associating a pulse `sigevent` structure with each event
source. We have seen in the previous section that a `sigevent` can be associated
with a timer when that timer is created. Other functions that take such
structures include `InterruptAttachEvent()`, `ionotify()` and
`mq_notify()`. Various servers accept `sigevent` structures embedded in
messages, which allow them to deliver these events when the conditions for
delivery are met.

:::::: Note ::::::
As of version 7.1 of the QNX OS, `sigevent` structures passed to servers
must first be registered with calls to `MsgRegisterEvent()`.
Registration prevents server processes from delivering unexpected (or even
malicious) events to clients. Events used with timers do not have to be
registered, as these are delivered by the microkernel. Nevertheless, it is good
practice to do so, and the example below does.
::::::::::::::::::

Pulses are queued for delivery by priority first and order of arrival
second. Using priorities allows certain events to be handled first, resulting in
lower latency for those events.

For the next example build the circuit depicted in Figure
@fig:two_button_circuit.

![A circuit with two push buttons](images/two_button_circuit.png){#fig:two_button_circuit}

The program sets three event sources, one for each button and a timer that fires
5 seconds after the last event that was received. Each event is assigned a different
pulse value to distinguish it. The timer event is used with a call to
`timer_create()`. The button events are passed to the GPIO resource
manager to be delivered by that server whenever a rising edge is detected on the
respective GPIO.

The pulses are delivered to a local channel. As the channel is only used for
pulse delivery within the process, the channel can be, and should be, declared
as private. This prevents other processes from connecting to this channel and
sending messages or pulses.

The program creates a connection to the channel,
which is then used by the system to deliver pulses. Note that while the
`SIGEV_PULSE_INIT()` macro takes the connection ID used to deliver the
pulse, `MsgRegisterEvent()` takes a connection ID that identifies the
server that is allowed to emit the event. In the case of a timer the event is
coming from the kernel, but in the case of the buttons the events are delivered
by the GPIO resource manager. Any other process trying to deliver this event to
the process will be prevented from doing so.

~~~ {#event_loop.c .c .numberLines}
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
~~~

~~~ {.c .numberLines}
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
        perror("MsgSend(pud)");
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
~~~

~~~ {.c .numberLines}
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
~~~

## Controlling Hardware {#sec:control-hardware}

We have already seen many examples of how to control external devices by
communicating with the GPIO and I2C resource managers. Let us now examine how
these resource managers, as well as other components such as the block device
drivers used by the file system, or the network drivers, interact with the
underlying hardware.

While every peripheral device has a unique way in which it is initialized and
controlled (with manuals that sometimes reach thousands of pages), they all
share common ways for software to interact with the device:
memory-mapped registers, interrupts and DMA buffers. With the exception of some
legacy devices on x86 systems, gone are the days of I/O ports.

The first thing a program implementing a device driver does is to map the
device's control registers. Mapping requires knowing the physical address of the
device and having permissions to do so. For most devices on the Raspberry Pi 4
these addresses are fixed and listed in the datasheet.[^5.9]
Things are more complicated when it comes to PCI devices, where the addresses
can be determined only at boot time through PCI enumeration. Note that the
addresses assigned to the devices do not correspond to parts
of the system's memory (RAM). Device addresses are always distinct from the address
ranges assigned to RAM.

:::: Warning :::::
Mapping physical memory is an extremely dangerous operation. Using the wrong
addresses creates bugs that are both critical and hard to find. Properly configured
systems restrict every process that requires such mappings to just the physical
addresses it needs (via the `PROCMGR_AID_MEM_PHYS` ability). An even
better option is for the system to add named ranges to typed memory at startup,
and then have the device drivers map these.
::::::::::::::::::

In the next exercise we will map the GPIO control registers and use those
directly to change the state of a pin. First, rebuild the simple LED circuit
from [Basic Output (LED)](python.md#sec:basic_output). Compile the following
program.

~~~ {#gpio_map.c .c .numberLines}
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
    gpio_regs = mmap(0, __PAGESIZE,
	                 PROT_READ | PROT_WRITE | PROT_NOCACHE,
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
~~~

Note that you will need to run this program as root, as a normal user does not
have the necessary privileges to map physical memory:

```
qnxuser@qnxpi:~$ su -c ./gpio_map
```

Lines 14-15 map the registers at physical address 0xfe200000 and then assign
these to an array of 32-bit integers called `gpio_regs` (the control
registers are 32-bit and need to be accessed as such). Note the use of
`PROT_NOCACHE`: if the virtual address assigned by the memory manager
for this mapping is marked as cached then reads and writes will not propagate
immediately to the registers.[^5.10] Also note the use of the `MAP_PHYS` flag,
which tells the memory manager that the last argument is actually a physical
address rather than an offset for the underlying object. Finally, `MAP_SHARED`
is crucial here: a private mapping would create a copy of the register contents
in memory rather than back the mapping with the registers themselves.

Lines 22-23 configure GPIO 16 as an output, first by clearing bits 18-20 in the
GPFSEL1 register and then setting these bits of 0b001. Lines 27-33 set GPIO 16
to high by setting bit 16 in GPSET0 or to low by setting bit 16 in GPCLR0. These
registers are described in GPIO chapter of the datasheet.

## Handling Interrupts {#sec:interrupts}

### What is an Interrupt?

An interrupt is an asynchronous notification delivered to the processor outside
of its current stream of execution. Interrupts are typically used by devices
(both internal and external to the processor) to inform the processor, and
through it any software interested in such events, that something has happened
that requires attention. For example, a timer generates an interrupt when its
counter reaches some value. A network device may generate an interrupt when a
new packet arrives, or a serial device when its buffer has room for more data.

The main benefit of interrupts is that they allow software handling devices to
avoid constantly checking whether an event has happened. Such checking, known as
*polling*, can waste processing time if performed too often and the device
does not require any handling, or can cause latency if not performed often
enough as devices are left unattended.

An event that causes the processor to switch away from its current execution
stream is called an *exception*. It is important to differentiate between
synchronous and asynchronous exceptions, with an interrupt being of the latter
type. A synchronous exception occurs as a direct result of the last instruction
executed by the processor. Examples of synchronous exceptions
include kernel calls, page faults, and floating point processing errors.
Interrupts, on the other hand, are generated by the devices irrespective of
current instructions, though of course software influences the generation of
interrupts: a timer device needs to be programmed by software and its interrupt
capability enabled before it can generate an interrupt. There are other types of
asynchronous exceptions, though they are less common, such as bus errors
observed some time after the responsible instruction was executed.

Typically, different devices generate different interrupts, so that when an
interrupt is delivered to the processor, software can determine which device it
came from and allow the relevant device driver to handle the interrupt. Things
are more complicated if the same interrupt is shared by multiple devices.
Nevertheless, modern hardware typically allows for enough interrupts to service
a large number of devices, and even different functions within the same device.

From a hardware point of view, an interrupt involves three components:

1. the device that generates the interrupt (a timer, a serial device, a
   network card, a graphics processor, etc.);
2. one or more interrupt controllers, connected both to the device and to the
   processor;
3. the processor.

In a multi-processor system, the interrupt controller can be configured to
deliver specific interrupts to specific processors, or (depending on the
controller) to a subset of the processors, though a single processor is the
usual case.

The following conditions must be met for an interrupt to be seen by the
processor:

1. the device is configured to generate an interrupt upon meeting some
   condition;
2. the condition is met;
3. the specific interrupt is enabled in the interrupt controller;
4. the processor to which the interrupt is routed allows for interrupts to be
   delivered.

When referring to a specific interrupt being disabled or enabled in the interrupt
controller we say that the interrupt is *masked* or *unmasked*,
respectively. For a processor, we talk about interrupts (in general) being enabled
or disabled.

### Processing an Interrupt

Any software that supports interrupts needs to follow a protocol with the three
hardware components that are involved in interrupt generation. In the following
description the term *device driver* will be used for the software that
manages a specific device, while the term *kernel* will be used for the
software that handles the processor's switch from running the normal instruction
stream to servicing an interrupt. Note, however, that different systems can
compose these components in different ways. In a monolithic kernel, the device
driver is part of the kernel (perhaps loaded as a kernel module). In a
microkernel system, such as QNX, the device driver is typically a stand-alone
user process. In a simple executive there is no real separate kernel, but the
system still needs to provide dedicated code for the processor to execute when
it switches to servicing an interrupt.

As mentioned above, the device driver must first configure the device to
generate an interrupt when various events of interest occur. A device driver may
choose not to configure the device for interrupts for some, or any, of the
events associated with the device, and resort to polling instead.

When an interrupt is generated by the device and the interrupt controller
notices it, the controller pokes the processor. Assuming the processor has
interrupts enabled, it immediately jumps to a pre-defined code address, where it
expects the kernel to have the code for handling interrupts. Typically the
kernel will store the state of the processor that reflects the current thread
executing, so that it can be resumed later.

Next, the kernel queries the interrupt controller to find out which specific
interrupt caused the processor to be poked (in some simple systems, this may
already be known by the address to which the processor jumped, but modern
hardware has too many interrupt sources to provide a different entry point for
each). Once the interrupt source has been determined the kernel can notify the
device driver so that it can take the necessary action.

When the device driver is done with the interrupt, it lets the interrupt
controller know, so that a new interrupt can be generated once the next event
occurs. Note that the specific interrupt is blocked between the time the
processor jumps to the kernel's entry routine and the time the device driver is
done handling it, to prevent a never-ending sequence of jumps to the entry
routine. Such blocking can occur either at the processor level (in which case
all interrupts to that processor are blocked), or at the controller level (which
allows for the masking of a specific interrupt).

### Handling an Interrupt in the QNX RTOS

Most interrupts in a QNX-based system are handled by device drivers that are
stand-alone user-mode processes. Exceptions to these include inter-process
interrupts (IPIs) and a per-processor clock interrupt, which are handled by the
kernel.

Each interrupt to be handled is associated with a thread, referred to as the
*Interrupt Service Thread*, or IST. This is true even for the interrupts
handled by the kernel, and you can see the IPI and clock ISTs for each core
listed by **pidin** when looking at the kernel process.

As mentioned in the previous section, the device driver must configure the
device to generate interrupts before it can start handling them. How the device
is configured is specific to each device and is covered by the hardware
manual. The routing of the device's interrupts to different processors is not up
to the device driver (as it is not up to the device), and is configured by the
**startup** executable that is part of the board-support package for the
specific board. The **startup** program also configures a global source ID
for every interrupt in the system, which the device driver needs to know in
order to attach to the right interrupt. The association of
these source ID values to interrupts is provided in the documentation for the
board-support package.

The device driver may choose between two methods for being notified about
interrupts. The first is to create a thread dedicated to the handling of the
interrupt (an IST). That thread calls `InterruptAttachThread()` with the
source ID of the relevant interrupt, which creates an association between that
interrupt and the thread in the kernel. The thread can now invoke the
`InterruptWait()` call to block until the interrupt is delivered. When
the call returns (without an error) the interrupt is masked in the
controller. It is up to the device driver to unmask the interrupt once it has
handled it and is ready to accept new interrupts. Unmasking can be done with a
call to `InterruptUnmask()`. However, as the thread will typically employ
a loop around `InterruptWait()` that needs to unmask right before the
call, there is a flag `_NTO_INTR_WAIT_FLAGS_UNMASK` to
`InterruptWait()` to cause it to unmask the interrupt and then block,
which saves on the number of kernel calls required to handle an interrupt.

The following code snippet shows the typical structure of an interrupt handling
loop in an IST:

~~~ {.c .numberLines}
int id = InterruptAttachThread(interrupt_number,
                               _NTO_INTR_FLAGS_NO_UNMASK);

for (;;) {
    if (InterruptWait(_NTO_INTR_WAIT_FLAGS_UNMASK, NULL) != -1) {
        // Service interrupt
    } else {
        // Handle errors
    }
}
~~~

Notes:

* The interrupt number passed to `InterruptAttachThread()` is the
  global interrupt source identifier as provided by the BSP documentation. The
  value returned from the call is a process-specific identifier that can then be
  passed to calls to mask, unmask or detach an interrupt.
* Since the loop above uses the shortcut flag for unmasking an interrupt
  before blocking, the call to `InterruptAttachThread()` must not itself
  unmask the interrupt. The kernel keeps track of how many times an interrupt is
  masked and unmasked, and it is an error to unmask too many times.
* Each thread can attach to at most one interrupt. It is possible to attach
  multiple threads to the same interrupt. When the interrupt is delivered, the
  kernel notifies all the threads attached to that interrupt. However, attaching
  multiple threads to the same interrupt is rarely useful, and can be a source
  of trouble, as the controller is only told to unmask the interrupt once all
  threads are done handling it and unmask the interrupt. If one thread does not
  do that, or if it takes a long time to handle the interrupt, the other threads
  will not be able to handle new events from the device.
* The IST is a regular thread. Other than the requirement to unmask the
  interrupt when it is done there are no special restrictions imposed upon
  it. The IST can invoke any kernel call or any library routine. An IST that
  takes a long time to service an interrupt affects only that particular device
  and not the system as a whole.
* The flag `_NTO_INTR_WAIT_FLAGS_FAST` can be used in a call to
  `InterruptWait()` to reduce the overhead of the kernel call. The
  downside of this flag is that it cannot be used in combination with a timeout on
  the blocking call. If a device driver does not need to enforce a timeout on
  the call then this flag provides lower latency.
* The latency of handling a specific interrupt is determined by the priority
  and processor affinity of the IST. As a general rule the IST should have its
  processor affinity set to the same processor to which the interrupt is
  routed. The higher the thread's priority the lower the latency, though a high
  priority also requires shorter work bursts from the thread to reduce the
  impact on the system.

A second method for handling interrupts is by attaching an action represented by
a signal event to the interrupt. The device driver then waits for the event and
processes the interrupt. The actions associated with signal events include
delivering pulses to a channel, emitting a signal, posting a semaphore,
creating a thread and changing a memory value. In practice, however, only pulse
and semaphore events should be associated with interrupts. A call to
`InterruptAttachEvent()` associates an interrupt number with a signal
event.

Earlier we said that each interrupt handled by the system must be associated
with a thread. This is true for the case of attaching an event to an
interrupt. A call to `InterruptAttachEvent()` creates a thread that
itself calls `InterruptAttachThread()` and then runs a loop that
dispatches the requested event whenever `InterruptWait()` returns. It
should be clear that using events with interrupts does not provide any
functionality that using ISTs cannot, and the latter provides lower overhead and
better control over interrupt handling. The sole purpose of the event API is to
allow for easy migration of code from previous versions of the QNX RTOS that did
not provide the thread-attaching API.

### What about ISRs?

People familiar with earlier versions of the QNX RTOS, or with other operating
systems, may wonder at this point about the absence of any mention above of
interrupt service routines (ISRs). An ISR is code for handling an interrupt
that runs in the context of the kernel routine that is jumped to by the
processor when the interrupt is delivered. In a monolithic kernel, an ISR is
simply a function registered by the device driver and invoked by the kernel. In
a microkernel, the ISR may be some form of byte code interpreted by the kernel,
or, as in the case of previous versions of the QNX RTOS, a function pointer in
the user-mode driver process that the kernel invokes after switching to the
address space of the process.

ISRs provide very good interrupt latency, but do so at the expense of severe
limitations, as well as safety and security risks. An ISR must be kept very
short. It cannot invoke any kernel call, which means that synchronization with
other parts of the driver cannot use blocking objects such as mutexes and
semaphores. Any bug or exploit in the ISR can compromise the entire system, as
the code is executed with the full privileges of the kernel. Moreover, any bug
or exploit in the driver can lead to a bug or exploit in the ISR. For all these
reasons ISRs were removed from (or, more accurately, were not designed for) the
QNX kernel as of version 8.0 of the operating system.

### Example

For the next example, rebuild the button and LED circuit described in
[Basic Input (Push Button)](python.md#sec:basic_input). In order to run the
program, you will first need to kill the GPIO resource manager, which itself
attaches to the GPIO interrupt. While it is possible for more than one process
to attach to the same interrupt, the result is often interference, and in fact
the resource manager will reset the hardware's event registers before our
example program sees the interrupt.

```
qnxuser@qnxpi:~$ su -c slay rpi_gpio
qnxuser@qnxpi:~$ su -c ./rpi_interrupt
```

~~~ {#gpio_interrupt.c .c .numberLines}
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
    gpio_regs = mmap(0, __PAGESIZE,
	                 PROT_READ | PROT_WRITE | PROT_NOCACHE,
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
~~~

~~~ {.c .numberLines}
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
~~~

The example attaches interrupt 145, which is the GPIO interrupt on Raspberry Pi
4, to the main thread. While it is common to have a dedicated IST in a program,
this example is simple enough to have the main thread service the
interrupt. After configuring the GPIOs such that the hardware detects both
rising and falling edges on GPIO 20 (the button), the program goes into an
infinite loop, waiting for the interrupt. Once the call to
`InteruptWait()` returns, indicating that interrupt 145 has fired, the
code checks that the reason for the interrupt was indeed a change to GPIO 20. If
so, it reads the current value of the GPIO, and updates the output on GPIO 16
(the LED) accordingly. It is important to reset the event registers before going
back to `InterruptWait()`, to prevent the interrupt from firing again
immediately.


[^5.1]: For simplicity, the OS-level directory was omitted, as all of our code is
     going to target QNX systems.
[^5.2]: Well, there are limits. A single part in a message is limited to the size
     of the virtual address space, which is 512GB, and a message is limited to
     512,000 parts, for a total limit of 256PB. If you run into this limit,
     please contact QNX support. We would love to see your system.
[^5.3]: Note that true parallelism is not possible on a single processor, but to a
     human observer fast-switching concurrency can be non-distinguishable from
     parallelism.
[^5.4]: Putting threads into execution is the one task that even the smallest,
     purest microkernel has to do itself.
[^5.5]: Convincing programmers that the threads in the applications or drivers
	 they work on should not have the highest priority in the system is one of
     the toughest jobs a system architect has to deal with.
[^5.6]: The one millisecond granularity is the result of the standard timer
     resolution. If needed, this example can be modified to use high-resolution
     timers.
[^5.7]: Provided for POSIX compatibility. Strongly discouraged, especially when
     used with timers.
[^5.8]: There are only a few scenarios in which such timers are needed. As one
     safety expert at QNX is fond of saying, even the fastest car travels very
	 little distance in one millisecond.
[^5.9]: <https://datasheets.raspberrypi.com/bcm2711/bcm2711-peripherals.pdf>
[^5.10]: Since the physical address is not part of RAM, the use of `PROT_NOCACHE`
      results in strongly-ordered device memory (or nGnRnE, in AArch64
      terminology), which also ensures that writes are not reordered.
