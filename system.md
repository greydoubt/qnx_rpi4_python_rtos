---
title: Exploring The System
author: Elad Lahav
chapter: 3
---

# Exploring The System {#chapter:system}

## The Shell {#sec:shell}

Once you have logged in with SSH you are presented with the shell prompt:

```
qnxuser@qnxpi:~$
```

The shell is an interactive command interpreter: you type in various commands
and the shell executes these, typically by creating other processes. Anyone who
has ever used a \*NIX system is familiar with the basic shell commands, such as
**echo**, **cat** and **ls**.

The default shell on QNX systems is a variant of the Korn Shell, called
**pdksh**, but our Raspberry Pi image uses the more familiar **bash**
(the Bourne Again Shell), which is the default shell on many modern *NIX systems.

Many of the simple (and some not so simple) shell utilties are now provided by a
single program call **toybox**. This program uses the name by which it is
invoked as indication of which utility it should run. The system uses symbolic
links from various utility names to **toybox**, which allows the user to
use standard shell commands. For example, we can use **ls** to list files:

```
qnxuser@qnxpi:~$ ls /tmp
encoder.py  robot.py
qnxuser@qnxpi:~$ which ls
/proc/boot/ls
qnxuser@qnxpi:~$ ls -l /proc/boot/ls
lrwxrwxrwx  1 root root 6 2023-10-14 15:18 /proc/boot/ls -> toybox
```

The sequence above shows that **ls** is a symbolic link under
**/proc/boot** to **toybox**.

## The File System {#sec:file-system}

### Background

Before exploring the file system on the image there are a few concepts that need
to be understood. A *file system* is a collection of files, typically
organized hierarchically into directories, that can be stored on some medium
(such as a hard drive, an SD card, network storage or even RAM). There are
different formats for file systems, some of the more common ones being FAT and
NTFS from Microsoft and ext4 from Linux. QNX supports a few file system formats,
primarily the native QNX6 power-safe file system.

Every file and every directory in a file system has a name, and a file is
identified uniquely within a file system by its *path*, which is the chain
of directory names leading to that file, separated by slashes, followed by the
file's name. For example, the file **foo**, contained within the directory
**bar**, which is itself contained within the directory **goo**, is
identified by the path **goo/bar/foo** within that file system.

Each file system can be *mounted* at some path in the system, with all paths
starting at the root directory, identified by a single slash. If the file system
in the example above is mounted at **/some**, then the full path of the
aforementioned file is **/some/goo/bar/foo**. The collection of all paths
from all file systems is referred to as the *path space*, and is handled by
the path manager in the QNX kernel.

With the exception of the most trivial ones, all QNX-based systems employ more
than one file system. These different file systems are provided by different
server processes. Any request to access a file queries the path manager for the
server that provides the path to be opened, and then asks that server for the
file.

### File Systems on the Image

The **df** command allows us to see which file systems are included in the
image, and under which path each of those is mounted:

```
qnxuser@qnxpi:~$ df
ifs                       26344     26344         0     100%  /
/dev/hd0t17            26212320   1288784  24923536       5%  /data/
/dev/hd0t17             2097120    600088   1497032      29%  /system/
/dev/hd0t11             2093016     31256   2061760       2%  /boot/
/dev/hd0               30408704  30408704         0     100%
/dev/shmem                    0         0         0     100%  (/dev/shmem)
```

The first entry shows the Image File System (IFS). This is a read-only file
system that is loaded into memory early during boot, and remains resident in
memory. The IFS contains the startup executable, the kernel and various
essential drivers, libraries and utilities. At a minimum, the IFS must contain
enough of these drivers and libraries to allow for the other file systems to be
mounted. Some QNX-based systems stick to this minimum, while other use the IFS
to host many more binaries, both executables and libraries. There are advantages
and disadvantages to each approach. The IFS in this image is mounted at the root
directory, with most of its files under **/proc/boot**. It is possible to
change this mount point.

Next are two QNX6 file systems, mounted at **/system** and **/data**,
respectively. This separation allows for the system partition to be mounted
read-only, potentially with added verification and encryption, while the data
partition remains writable. In this image both partitions are writable, to allow
for executables and libraries to be added later and for configuration files to
be edited.

Under the **/system** mount point you will find directories for executables
(**/system/bin**), libraries (**/system/lib**), configuration files
(**/system/etc**) and more. The data partition holds the users' home
directories (**/data/home**), the temporary directory (**/data/tmp**,
also linked via **/tmp**) and a run-time directory for various services
(**/data/var**).

The FAT file system mounted at **/boot** is used by the Raspberry Pi
firmware to boot the system. It also holds a few configuration files to allow
for easy setup before the system is booted for the first time (see
[Booting The Raspberry Pi](gettingstarted.md#sec:boot-rpi)).

The next entry shows the entire SD card and does not represent a file system.

Finally, **/dev/shmem** is the path under which shared memory objects
appear. The reason it is listed here is that it is possible to write to files
under **/dev/shmem**, which are kept in memory until the system is shut
down (or the file removed). It is not, however, a full file system, and certain
operations (e.g., `mkdir`) on it will fail. Do not treat it as a file
system.

At this point people familiar with previous versions of QNX, with Linux or with
other UNIX-like operating systems, may be wondering what happened to the
traditional paths, such as **/bin** or **/usr/lib**. The answer is
that you can still create a file system on QNX with these paths and mount it at
the root directory. There are two reasons this image employs a different
strategy:

1. The separation of **/system** and **/data** makes it easier to
   protect the former, as mentioned above.
2. This layout avoids union paths, which have both performance and security
   concerns.

Union paths are an interesting feature of the operating system that allows multiple
file systems to provide the same path. For example, both the IFS and one (or
more) of the QNX6 file systems can provide a folder called **etc**. If both
are mounted under the root path then the directory **/etc** is populated as
a mix of the files in the folders of both file systems. Moreover, if both file
systems have a file called **etc/passwd** then the file visible to anyone
trying to access **/etc/passwd** is the one provided by the file system
mounted at the front. This feature can be quite useful in some circumstances,
such as providing updates to a read-only file system by mounting a patch file
system in front of it, but it complicates path resolution, can lead to confusion
and, under malicious circumstances, can fool a program into opening the wrong
file.


## Processes {#sec:processes}

A *process* is a running instance of a program. Each process contains one or
more *threads*, each representing a stream of execution within that program.
Any QNX-based system has multiple processes running at any given point. The
first process in such a system is the process that hosts the kernel, which,
mainly for historical reasons, is called `procnto-smp-instr`. The program
for this process comprises the Neutrino microkernel, as well as a set of basic
services, including the process manager (for creating new processes), the memory
manager for managing virtual memory and the path manager for resolving path
names to the servers that host them.

The microkernel architecture means that most of the functionality provided by a
monolithic kernel in other operating systems is provided by stand-alone
user processes in the QNX RTOS. Such services include file systems, the network
stack, USB, graphics drivers, audio and more. A QNX system does not require all
of these to run. A headless system does not need a graphics process, while a
simple controller may do away with a permanent file system.

Finally, a system will have application processes to implement the functionality
that the end user requires from it. An interactive system can have processes for
the shell and its utilities, editors, compilers, web browsers, media players,
etc. A non-interactive system can have processes for controlling and monitoring
various devices, such as those found in automotive, industrial or medical
systems.

The `pidin` command can be used to list all processes and threads in the
system. The following is an example of the output of this command. Note,
however, that the output was trimmed to remove many of the threads in each
process for the purpose of this example (indicated by "..."):

```
qnxuser@qnxpi:~$ pidin
     pid tid name                         prio STATE       Blocked
       1   1 /proc/boot/procnto-smp-instr   0f READY
       1   2 /proc/boot/procnto-smp-instr   0f READY
       1   3 /proc/boot/procnto-smp-instr   0f RUNNING
       1   4 /proc/boot/procnto-smp-instr   0f RUNNING
       1   5 /proc/boot/procnto-smp-instr 255i INTR
       1   6 /proc/boot/procnto-smp-instr 255i INTR
       ...
       1  20 /proc/boot/procnto-smp-instr  10r RECEIVE     1
   12291   1 proc/boot/pipe                10r SIGWAITINFO
   12291   2 proc/boot/pipe                10r RECEIVE     1
   12291   3 proc/boot/pipe                10r RECEIVE     1
   12291   4 proc/boot/pipe                10r RECEIVE     1
   12291   5 proc/boot/pipe                10r RECEIVE     1
   12292   1 proc/boot/dumper              10r RECEIVE     1
   12292   2 proc/boot/dumper              10r RECEIVE     2
   12293   1 proc/boot/devc-pty            10r RECEIVE     1
   12294   1 proc/boot/random              10r SIGWAITINFO
   12294   2 proc/boot/random              10r NANOSLEEP
   12294   3 proc/boot/random              10r RECEIVE     1
   12294   4 proc/boot/random              10r RECEIVE     2
   12295   1 proc/boot/rpi_mbox            10r RECEIVE     1
   12296   1 proc/boot/devc-serminiuart    10r RECEIVE     1
   12296   2 proc/boot/devc-serminiuart   254i INTR
   12297   1 proc/boot/i2c-bcm2711         10r RECEIVE     1
   12298   1 proc/boot/devb-sdmmc-bcm2711  10r SIGWAITINFO
   12298   2 proc/boot/devb-sdmmc-bcm2711  21r RECEIVE     1
   12298   3 proc/boot/devb-sdmmc-bcm2711  21r RECEIVE     2
   ...
   12298  16 proc/boot/devb-sdmmc-bcm2711  21r RECEIVE     4
   36875   1 proc/boot/pci-server          10r RECEIVE     1
   36875   2 proc/boot/pci-server          10r RECEIVE     1
   36875   3 proc/boot/pci-server          10r RECEIVE     1
   49166   1 proc/boot/rpi_frame_buffer    10r RECEIVE     1
   61455   1 proc/boot/devc-bootcon        10r RECEIVE     1
   61455   2 proc/boot/devc-bootcon        10r CONDVAR     (0x50d562a3cc)
   73744   1 proc/boot/rpi_thermal         10r RECEIVE     1
   86033   1 proc/boot/rpi_gpio            10r RECEIVE     1
   86033   2 proc/boot/rpi_gpio           200r INTR
  118802   1 proc/boot/io-usb-otg          10r SIGWAITINFO
  118802   2 proc/boot/io-usb-otg          10r CONDVAR     (0x5970918880)
  118802   3 proc/boot/io-usb-otg          10r CONDVAR     (0x59709193a0)
  ...
  118802  13 proc/boot/io-usb-otg          10r RECEIVE     2
  139283   1 system/bin/io-pkt-v6-hc       21r SIGWAITINFO
  139283   2 system/bin/io-pkt-v6-hc       21r RECEIVE     1
  139283   3 system/bin/io-pkt-v6-hc       22r RECEIVE     2
  ...
  139283   9 system/bin/io-pkt-v6-hc       21r RECEIVE     7
  163860   1 ystem/bin/wpa_supplicant-2.9  10r SIGWAITINFO
  163861   1 system/bin/dhclient           10r SIGWAITINFO
  192535   1 system/bin/sshd               10r SIGWAITINFO
  196630   1 system/bin/qconn              10r SIGWAITINFO
  196630   2 system/bin/qconn              10r RECEIVE     1
  208908   1 proc/boot/ksh                 10r SIGSUSPEND
  270349   1 proc/boot/pidin               10r REPLY       1
```

Each process has its own process ID, while each thread within a process has its
own thread ID. The output of pidin lists all threads, grouped by process, along
with the path to the executable for that process, the priority and scheduling
policy for the thread, its state and some information relevant to that
state. For example,

```
118802 3 proc/boot/io-usb-otg  10r CONDVAR  (0x59709193a0)
```

shows that thread 3 in process 118802 belongs to a process that runs the
`io-usb-otg` executable (the USB service). Its priority is 10 while the
scheduling policy is round-robin. It is currently blocked waiting on a condition
variable whose virtual address within the process is 0x59709193a0.

It is possible to show other types of information for each process with the
`pidin` command: `pidin fds` shows the file descriptors open for
each process, `pidin arguments` shows the command-line arguments used when
the process was executed, `pidin env` shows the environment variables for
each process and `pidin mem` provides memory information. It is also
possible to restrict the output to just a single process ID (e.g.,
`pidin -p 208908`) or to all processes matching a given name (e.g.,
`pidin -p ksh`).

## Memory {#sec:memory}

RAM, like the CPU, is a shared resource that every process requires, no matter
how simple. As such, all processes compete with each other for use of memory,
and it is up to the system to divide up memory and provide it to processes. In a
QNX system, control of memory is in the hands of the virtual memory manager,
which is a part of the `procnto-smp-instr` process (recall that this is a
special process that bundles the microkernel with a few services).

A common problem with the use of memory is that the system can run out of it,
either due to misbehaving processes, or to poor design that does not account for
the requirements of the system. Such situations naturally lead to two
frequently-asked questions:

1. How much memory does my system use?
2. How much does each process contribute to total system use?

The first question is easy to answer. The second, perhaps surprisingly, is not.

A quick answer to the first question can be obtained with `pidin`:

```
qnxuser@qnxpi:~$ pidin info
CPU:AARCH64 Release:8.0.0  FreeMem:3689MB/4032MB BootTime: ...
Processes: 28, Threads: 105
Processor1: 1091555459 Cortex-A72 1500MHz FPU
Processor2: 1091555459 Cortex-A72 1500MHz FPU
Processor3: 1091555459 Cortex-A72 1500MHz FPU
Processor4: 1091555459 Cortex-A72 1500MHz FPU
```

The first line shows that the system has 4GB of RAM (depending on the model,
yours may have 1GB, 2GB, 4GB or 8GB), with the system managing 4032MB, out
of which 3689MB are still available for use. The QNX memory manager may not have
access to all of RAM on a board if some of it is excluded by various boot-time
services, hypervisors, etc.

A more detailed view is available by looking at the **/proc/vm/stats**
pseudo-file. You will need to access this file as the root user, e.g.:

```
qnxuser@qnxpi:~$ su -c cat /proc/vm/stats
```

The output provides quite a bit of information, some of it only makes sense for
people familiar with the internal workings of the memory manager. However, a few
of the lines are of interest:

* `page_count=0xfc000` shows the number of 4K RAM pages available
   to the memory manager. The total amount of memory in the `pidin info`
   output should match this value.
* `vmem_avail=0xe67f6` is the number of pages that have not been
   reserved by allocations, and are thus available for new allocations. This
   value corresponds to the free memory number provided by `pidin info`.
* `vm_aspace=30` is the number of address spaces in the system,
   which corresponds to the number of active processes.[^3.1]
* `pages_kernel=0x4347` is the number of pages used by the kernel
   for its own purposes. Note that the value includes the page-table pages
   allocated for processes.
* `pages_reserved=0x9012` is the number of pages that the memory
   manager knows about, but cannot allocate to processes as regular memory. Such
   ranges are typically the result of the startup program, which runs before the
   kernel, reserving memory for special-purpose pools, or as a way to reduce boot
   time (in which case the memory can be added to the system later).

The remaining `pages_*` lines represent allocated pages in different
internal states.

The breakup of physical memory into various areas can be observed with the
following command

```
qnxuser@qnxpi:~$ pidin syspage=asinfo
```

All entries that end with `sysram` are available for the memory manager
to use when servicing normal allocation requests from processes. Entries that
are part of RAM but outside the sysram ranges can be allocated using a special
interface known as *typed memory*.

\medskip

We will now attempt to answer the question of how much does a process contribute
to total memory use in the system. To answer this question, we will use the
**mmap_demo** program that is included with the Raspberry Pi image. Start
this program in the background, so that it remains alive while we examine its
memory usage.

```
qnxuser@qnxpi:~$ mmap_demo &
[1] 1617946
qnxuser@qnxpi:~$ Anonymous private mapping 3de2cf0000-3de2cf9fff
Anonymous shared mapping 3de2cfa000-3de2cfbfff
File-backed private mapping 209b3a6000-209b3a9fff
File-backed shared mapping 209b3aa000-209b3adfff
Virtual address range only mapping 3de2cfc000-3ed6f3bfff
```

This program creates different types of mappings (though it doesn't come close
to exhausting all the different combinations of options that can be passed to
`mmap()`). The first two mappings are anonymous, which means that the
resulting memory is filled with zeros. The next two are file-backed, which means
that the memory reflects the contents of a file (the program uses a temporary
file for the purpose of the demonstration). Finally, the last mapping just
carves a large portion of the process' virtual address space, without backing it
with memory. The output shows the virtual addresses assigned to each of these
mappings. The start address is chosen by the *address space layout
randomization* (ASLR) algorithm, while the end address of each range reflects
the requested size. The output you see will therefore be different than the
example above for the start addresses, but the sizes will be the same.

To get a view of the memory usage by this process we can examine all the
mappings it has. This is done by looking at the **/proc/\<pid\>/pmap** file,
where **\<pid\>** should be replaced by the process ID (1617946 in the
example above):[^3.2]

```
qnxuser@qnxpi:~$ cat /proc/1617946/pmap
vaddr,size,flags,prot,maxprot,dev,ino,offset,rsv,guardsize,refcnt,mapc...
0x0000001985914000,0x0000000000080000,0x00081002,0x03,0x0f,0x00000001,...
0x000000209b3a6000,0x0000000000004000,0x00000002,0x03,0x0f,0x0000040c,...
0x000000209b3aa000,0x0000000000004000,0x00000001,0x03,0x0f,0x0000040c,...
0x0000003de2ced000,0x000000000000d000,0x00080002,0x03,0x0f,0x00000001,...
0x0000003de2cfa000,0x0000000000002000,0x00080001,0x03,0x0f,0x00000001,...
0x0000003de2cfc000,0x00000000f4240000,0x00080002,0x00,0x0f,0x00000001,...
0x0000004843966000,0x0000000000038000,0x00010031,0x05,0x0d,0x00000802,...
0x000000484399e000,0x0000000000002000,0x00010032,0x01,0x0f,0x00000802,...
0x00000048439a0000,0x0000000000001000,0x00010032,0x03,0x0f,0x00000802,...
0x00000048439a1000,0x0000000000001000,0x00080032,0x03,0x0f,0x00000001,...
0x00000048439a2000,0x00000000000a6000,0x00010031,0x05,0x0d,0x00000802,...
0x0000004843a48000,0x0000000000004000,0x00080032,0x01,0x0f,0x00000001,...
0x0000004843a4c000,0x0000000000002000,0x00010032,0x03,0x0f,0x00000802,...
0x0000004843a4e000,0x0000000000006000,0x00080032,0x03,0x0f,0x00000001,...
0x0000004843a54000,0x0000000000013000,0x00010031,0x05,0x0d,0x00000802,...
0x0000004843a67000,0x0000000000001000,0x00080032,0x01,0x0f,0x00000001,...
0x0000004843a68000,0x0000000000001000,0x00010032,0x03,0x0f,0x00000802,...
0x0000005b7124b000,0x0000000000001000,0x00000031,0x05,0x0d,0x0000040b,...
0x0000005b7124c000,0x0000000000001000,0x00000032,0x01,0x0f,0x0000040b,...
0x0000005b7124d000,0x0000000000001000,0x00000032,0x03,0x0f,0x0000040b,...
```

Note that the output was truncated to fit the page. Here is a complete line that
will be examined in detail:

```
0x000000209b3a6000,0x0000000000004000,0x00000002,0x03,0x0f,0x0000040c,
0x000000000000001c,0x0000000000000000,0x0000000000004000,0x00000000,
0x00000004,0x00000002,/data/home/qnxuser/mmap_demo.9ubgxv,{sysram}
```

The first two columns show the virtual address (0x209b3a6000) and size (0x4000,
or 4 4K pages) of the mapping. You should find a matching entry in the output of
**mmap_demo**. The next two columns show the flags and protection bits
passed to the `mmap()` call.\footnote{The protection bit values are
  shifted right by 8 bits, due to internal representation concerns.} This line
corresponds to a private mapping with read and write permissions. The next 4
fields are not important for this discussion.

Next comes the reservation field, which is crucial. This field shows
how many pages were billed to this process for the mapping. In this case the
value is 0x4000, which is the same as the size of the mapping. The reason is
that this is a private mapping of a shared object, and such a mapping requires
that the system allocate new pages for it with a *copy* of the contents of
the object (in this case the temporary file). By contrast, a read-only shared
mapping of the same file will show a value of 0.

Of the last two fields, the first shows the mapped object. This can be a file, a
shared-memory object, or a single anonymous object that is used to map the
process' stacks and heaps. The second field (new in version 8.0 of the OS) shows
the typed memory object from which memory was taken (recall that "sysram"
refers to generic memory that the memory manager can use to satisfy most
allocations).

Given the information provided here, why is it hard to answer the question about
per-process memory consumption? The complexity comes from shared objects. Every
process maps multiple such objects, including its own executable and the C
library. As mentioned above, shared mapping of such objects (that is, mappings
that directly reflect the contents of the object, rather than creating a private
copy) show up in the **pmap** list as though they consume no memory. And
yet the underlying object may[^3.3] require memory allocation that
does affect the amount of memory available in the system. Additionally, there
may be shared memory objects that are not mapped by any process: one process can
create such an object with a call to `shm_open()` and never map it. That
process may even exit, leaving the shared memory object to linger (which is
required for POSIX semantics). A full system analysis of memory consumption thus
requires a careful consideration of all shared objects, along with which
processes, if any, map them.


## Resource Managers {#sec:resmgr}

A resource manager is a process that registers one or more paths in the path
space, and then services requests that open files under these paths. The best
example of a resource manager is a file system, which registers its mount path,
and then handles all requests to open directories and files under that
path. Once a file is opened by a client process, the client can send messages to
the resource manager, which acts as the server. Resource managers typically
handle some standard message types, such as read, write, stat and close, but can
also handle ad-hoc messages that are relevant to the specific service they
provide.

An example of a resource manager on the QNX Raspberry Pi image is the GPIO
server, **rpi_gpio**. You can see it running on the system by using the
**pidin** command, as described above:

```
qnxuser@qnxpi:~$ pidin -p rpi_gpio
     pid tid name                         prio STATE       Blocked
   86033   1 proc/boot/rpi_gpio            10r RECEIVE     1
   86033   2 proc/boot/rpi_gpio           200r INTR
```

The resource managed by this process is the 40-pin GPIO header on the Raspberry
Pi (See [GPIO Header](gpio.md#chapter:gpio_header)} for more information).
It does so by memory-mapping the hardware registers that control the header, and
then handling requests by various client programs to control GPIOs, e.g., to
turn an output GPIO pin on or off. The advantage of this design is that it
allows only one process to have access to the hardware registers, preventing
GPIO users from interfering with each other, either by accident or maliciously.

The **rpi_gpio** resource manager registers the path **/dev/gpio**
(though it can be configured to register a different path, if desired, via a
command-line argument). Under this path it creates a file for each GPIO pin,
with a name matching that of the GPIO number, as well as a single **msg**
file.

```
qnxuser@qnxpi:~$ ls /dev/gpio
0   12  16  2   23  27  30  34  38  41  45  49  52  8
1   13  17  20  24  28  31  35  39  42  46  5   53  9
10  14  18  21  25  29  32  36  4   43  47  50  6   msg
11  15  19  22  26  3   33  37  40  44  48  51  7
```

The numbered files can be read and written, which means that the resource
manager handles messages of type `_IO_READ` and `_IO_WRITE` on
these files. Consequently, we can use shell utilities such as **echo** and
**cat** on these files. To see this at work, follow the instructions for
building a simple LED circuit, as described in
[Basic Output (LED)](python.md#sec:basic_output).
Do not proceed to write Python code at this point. Once the circuit is built,
log in to the Raspberry Pi and run the following shell commands:

```
qnxuser@qnxpi:~$ echo out > /dev/gpio/16
qnxuser@qnxpi:~$ echo on > /dev/gpio/16
qnxuser@qnxpi:~$ echo off > /dev/gpio/16
```

The first command configures GPIO 16 as an output. The second turns the GPIO on
(sets it to ``HIGH") and the third turns the GPIO off (sets it to "LOW").

Let us consider how these shell commands end up changing the state of the
GPIO pin.

1. The command `echo on > /dev/gpio/16` causes the shell to open the
   file **/dev/gpio/16**, which establishes a connection (via a file
   descriptor) to the **rpi_gpio** resource manager.
2. The shell executes the **echo** command, with its standard output
   replaced by the connection to the resource manager.
3. **echo** invokes the `write()` function
   on its standard output with a buffer containing the string "on". The C
   library implements that call by sending
   a `_IO_WRITE` message to the resource manager.
4. The resource manager handles the `_IO_WRITE` message sent by
   **echo** as the client. It knows (from the original request to open
   the file **16** under its path) that the message pertains to GPIO 16, and
   it knows from the buffer passed in the message payload that the requested
   command is "on". It then proceeds to change the state of the GPIO pin by
   writing to the GPIO header's registers.

The **msg** file can be used for sending ad-hoc messages, which provide
better control of GPIOs without the need for the resource manager to parse
text. Such messages are easier to handle and are less error prone. The structure
of each message is defined in a header file that client programs can
incorporate into their code. We will see in
[How Does It Work?](python.md#sec:python_qnx) how a Python library uses such
messages to implement a GPIO interface.

For information on how to write resource managers, see the QNX documentation.


[^3.1]: The word *active* here is not a redundancy, as a process in the middle of
     its creation, or after termination but before being claimed by its parent
     (i.e., a zombie) does not have an address space.
[^3.2]: Confusingly, every process also has a **/proc/\<pid\>/mappings** file,
     which provides the state of every page assigned to this process (both from
     the virtual and physical point of view). This file can be huge and is
     rarely of interest.
[^3.3]: Only *may*, because some objects do not reduce the amount of allocatable
     memory, such as files in the IFS, typed-memory pools or memory-mapped
     devices.
