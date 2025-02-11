---
title: Getting Started
author: Elad Lahav
chapter: 2
---

# Getting Started {#chapter:getting_started}

## Shopping List {#sec:shopping}

In order to follow the exercises in this book you will need a few items.

1. **Raspberry Pi 4 board** Version 4 of this small-board computer is the only
   one officially supported by QNX at the time the book was written. You can use
   either the 2GB, 4GB or 8GB variants of this board. The 1GB variant is not
   supported. Please purchase the board from an authorized dealer.
2. **Micro SD card** This should be at least 8GB in size. Note that not all SD
   cards work, and you may need to try different ones (see
   [Troubleshooting](#sec:troubleshoot) below). I have not had problems with
   SanDisk cards.
3. **HDMI display, micro-HDMI connector and a USB keyboard** Only required if
   you want to connect to the system with a keyboard and a display.
4. **USB-TTL converter** Only required for troubleshooting the system over a
   serial console.
5. **Breadboard, jumper wires, LEDs, resistors, motor, servos** See
   [Controlling I/O with Python](python.md#chapter:python) for a full list. Note
   that many sellers of Raspberry Pi boards also offer cheap kits with these
   components.

It is possible to interact with the Raspberry Pi using a network connection
only, in which case there is no need to have a display, a keyboard or a USB-TTL
converter. Nevertheless, you may wish to have these available in case the system
fails to boot, or does not connect to the network. See
[Connecting to The System](#sec:connect) below.

## Installing the QNX SDP {#sec:install_sdp}

The QNX Software Development Platform (SDP) provides the necessary files for
building a QNX-based system. These files include pre-built binaries (programs
and libraries), as well the tools needed for writing and building new software
(compilers, linkers, header files, etc.). The SDP is freely available for
non-commercial use.

Obtaining the SDP involves the following steps:

1. Register for a myQNX account.
2. Request a free licence.
3. Download QNX Software Centre.
4. Install the SDP on your computer.

To get started, visit \url{qnx.com/getqnx}. You will be prompted for your myQNX
account credentials. If you do not have an account, create one first. Once you
have logged in, you can request a free QNX SDP licence to be associated with
your account. Follow the steps on the web page for getting the licence,
activating it, and associating it with your account.

The next step is to download the QNX Software Centre (QSC). Pick the version
that matches your host operating system. Windows users can run the installer
directly, while Linux users will have to make the file executable first. Refer
to the installation instructions for further information.

With QSC installed, we can now get the SDP. Run QSC, choose **Add Installation**,
and then select the SDP version to install. As this book was written for QNX
8.0, use this version of the SDP. Follow the prompts to complete the
installation. We will assume that the SDP was installed in a folder called
**qnx800** under your home directory, but you are free to choose any location.

Take a look at the installation directory.

```
$ ls ~/qnx800
host  qnxsdp-env.bat  qnxsdp-env.sh  target
```

The two scripts can be used to set up the environment for development (one for
Windows and one for Linux). The **host** folder is where you will find the tools
necessary for building programs on your computer, while the **target** folder
contains all of the files that can go on your QNX system (though you will only
ever likely need a small subset of these).

The base installation contains only a small portion of the content that is
available as part of the SDP. Additional packages can be installed with QSC,
some of which are free and some require a commercial licence. To develop for a
particular board, you will need to add the relevant *board support package*
(BSP), which contains the source code and binaries for the specific hardware.

The SDP provides everything you need in order to build your own QNX
system. However, to get you up and running quickly with Raspberry Pi, we will
use a pre-defined image that you can just copy to an SD card. Open QNX Software
Centre, and install the "QNX® SDP 8.0 Quick Start image for Raspberry Pi 4"
package. The image file is now located in the **images** folder under SDP
installation path:

```
$ ls ~/qnx/sdp/8.0/images/
qnx_sdp8.0_rpi4_quickstart_20240920.img
```

(The name of the image will likely be different for you, as it includes the
version number and date.)

## Creating The SD Card Image {#sec:sd_card}

The Raspberry Pi boots from a micro SD card, which is inserted to a slot under
the board. While it is possible to use other storage devices once the system is
running, we will use the SD card both for the boot image and for holding the
primary file systems. SD cards are neither fast nor resilient when compared with
some other storage devices, but they will do fine for the activities described
in this book.

### Generate the Image

Note that we cannot just copy the image file to the card. The image file holds
the partition and file system information, along with all of the file data that
the system requires. Copying the image file to the SD card, which is typically
formatted with a FAT file system, will not have the desired effect, as the disk
image is not bootable. Instead, we are going to perform a raw copy of the image
file to the card.

### Copy the Image: Raspberry Pi Imager

The Raspberry Pi Foundation publishes a utility called **Raspberry Pi Imager**,
which provides an easy way to transfer any OS image to a removable medium, such
as an SD card. The utility is available for Windows, Linux and macOS.[^2.3]

Once you have downloaded, installed, and run the Imager, you will be prompted
for the following information:

1. Device: Choose the Raspberry Pi 4.
2. OS: Choose **Use custom**, and then navigate to pick the QNX image.
3. Storage: Choose the SD card to use (assuming it is already connected via a
   card reader).

Click **Next** and wait for the Imager to complete before removing the device.

![Raspberry Pi Imager](images/rpi_imager.png){#fig:rpi_imager}

### Copy the Image: Linux Command Line

The `dd` command can be used to write raw data from one file or device
to another. The source is the image file, while the destination is the device
name for the SD card.

:::: Warning :::::
It is crucial that you determine the correct device name!
Copying the image to the wrong device can cause permanent damage to other
storage devices, including the one used for your computer's file system.
::::::::::::::::::

First, insert the card to a card reader connected to your computer. The
operating system may mount the card automatically, presenting an existing file
system under some path. We are not interested in this file system or
path. Instead, we want to establish the name of the device.

The `lsblk` command can be used to list the available block devices.
Find the device that matches the size of the card you have just inserted. In the
example below, this is **/dev/sdb**. Note that the numbered devices
appearing below it are partitions on the device, but we want the primary device
name in order to write the image to the entire card. If you have doubt, remove
the card, run `lsblk` again to confirm that device is no longer listed
(or is listed with a 0 size), and repeat the exercise after re-inserting the
card. Alternatively, use the `dmesg` command to see which device was
detected when the SD card was inserted.

```
$ lsblk
NAME        MAJ:MIN RM   SIZE RO TYPE MOUNTPOINTS
...
sda           8:0    1     0B  0 disk
sdb           8:16   1  14.5G  0 disk
|-sdb1        8:17   1     1G  0 part /media/elahav/1945-029C
|-sdb2        8:18   1     1G  0 part
|-sdb3        8:19   1  12.5G  0 part
nvme0n1     259:0    0   1.9T  0 disk
|-nvme0n1p1 259:1    0   260M  0 part /boot/efi
...
```

You are now ready to copy the image. Run the following command from the
directory that contains the image file:

```
$ dd if=qnx_rpi_sdcard.img of=<device name> bs=4M status=progress
```

Make sure to replace `<device name>` with the name of the card block
device discovered above (e.g., **/dev/sdb**).

Your system may require root privileges for this command, in which case you will
need to run the command as root. On most modern Linux systems this is done by
prefixing the `sudo` command:

```
$ sudo dd if=qnx_rpi_sdcard.img of=<device name> bs=4M status=progress
```

## Booting The Raspberry Pi {#sec:boot-rpi}

Once the image has been copied to the SD card, and while it is still plugged in
to your computer, you should be able to see the files written to the first
partition (a FAT partition, visible to any operating system). In that partition
there are two files that need to be edited to configure the system. The first is
**qnx_config.txt**, which can be left alone unless you want to change the
default host name of **qnxpi**, or control where the console output goes.

The second file, which you must update, is **wpa_supplicant.conf**. At the
very bottom of this file (after much documentation) is the network settings
configuration structure. Update the `ssid` entry to the name of your WiFi
network, and the `psk` entry to the password. These settings assume a
standard home network WPA authentication scheme. On a different kind of network
you may need to change these settings.\footnote{You may consult documentation
  such as \url{https://wiki.netbsd.org/tutorials/how_to_use_wpa_supplicant/}, or
  your organization's IT professional.}

After updating these files, eject the SD card and insert it into the Raspberry
Pi's slot. Connect the Raspberry Pi to the power supply to boot the system. This
image is not set up to provide a graphical desktop, but it does show information
on the screen. Assuming that all
goes well and the network configuration is correct, the system should be ready
within a few seconds.

## Connecting to The System {#sec:connect}

There are different ways to interact with the system. The most convenient and
powerful is over a secure network connection (SSH). However, it may be required,
at least at first, to have a way to interact with the system in case the network
connection fails (see "Troubleshooting" below). Whenever you need to log in,
use **qnxuser** as the user name and password, or, if root access is
required, **root** as the user name and password.

:::::: Note ::::::
Make sure you update these passwords the first time you log in. Passwords can be
changed using the `passwd` shell command. Note that root access over SSH is
disabled by default.
::::::::::::::::::

### Display and Keyboard

While the system does not support a graphical desktop, it does provide a simple
terminal program. Attach a display to the first micro-HDMI connector (the one
closest to the power connector), and a keyboard and a mouse to the USB ports.
When the system boots you should see a welcome screen. Pressing the button in
the middle starts a terminal instance. You can log in as either **qnxuser** or
**root** and execute shell commands.

### Serial Connection

The most resilient way to connect to the system is with the aid of
a USB-to-TTL converter, which provides a basic serial connection between the
Raspberry Pi and another computer. There are many models of such converters
available for purchase, and many of those have instructions on how to connect to
the Raspberry Pi.

:::::: Note ::::::
In order to get a login prompt on the serial console you must edit the
**qnx_config.txt** file in the boot partition to remove (or comment out)
the line that says `CONSOLE=/dev/con1`. This can be done either from the
Raspberry Pi itself, or when the SD card is plugged into your computer.
::::::::::::::::::

The connection involves three pins on the Raspberry Pi 40-pin GPIO header (see
[GPIO Header](gpio.md#chapter:gpio_header)). Pin 8 is the transmit pin,
pin 10 the receive pin and the third is any of the ground pins (though usually
either 6 or 14 is used due to their proximity to the transmit/receive
pins). Which of the converter connections goes to which pin on the Raspberry Pi
depends on the converter itself. If you need to buy one, make sure it has
instructions for the Raspberry Pi.

::::: Warning ::::
Never connect a USB-TTL converter to any of the power pins on the Raspberry
Pi. Some converters have a fourth connector that should be left
unattached. Such converters tend to be very susceptible to current surges and
once damaged are beyond repair.
::::::::::::::::::

An example connection of a USB-TTL converter to a computer is shown in Figure
@fig:ttl_usb.

![USB-TTL connection example](images/ttl_usb.png){#fig:ttl_usb}

Once the USB-TTL converter is connected to the computer you can use a terminal
program to connect to the Raspberry Pi. Terminal programs include
**minicom**, **GNU Screen** and **c-kermit** for UNIX-like
systems, or **PuTTY** for Windows. There are many tutorials on the Internet
on how to use such a program with the Raspberry Pi and you should follow one of
those. Next, boot the Raspberry Pi and look for output in the terminal.

### Secure Shell

The system is configured to use mDNS to
advertise its IP address on the local network. Assuming your computer is set up
for mDNS you should be able to log in using the host name specified in the
**qnx_config.txt** file. By default that name is **qnxpi**, and the
advertised name is **qnxpi.local**.

```
$ ssh qnxuser@qnxpi.local
```

You should see the system's shell prompt:

```
qnxuser@qnxpi:~$
```

## Writing Code {#sec:write-code}

In [Controlling I/O with Python](python.md#chapter:python) we will be writing
code in Python for controlling various external devices using the Raspberry
Pi. In [Real-Time Programming in C](realtime.md#chapter:realtime) we
will be writing code in the C language for better control and real-time
responsiveness. In both cases you will need to create and edit source files.

There are a few options for writing code for use with the QNX Raspberry Pi
image. The first is to use the **Vim** editor included with the image. For
that you log in via SSH and run the `vim` command. While **Vim** is a
very popular editor, people who first encounter it can be baffled by the
interface, especially by the split between a command mode and an insert (or
edit) mode. There are many guides on the Internet for getting started with
**Vim** and you can follow these. If you have already started **Vim**
and would really like to quit, press the `ESC` key to go back to command
mode, and then `:q` (colon followed by the letter q) and `ENTER` to
quit.

A second option is to write the code on your computer, using your preferred text
editor\footnote{Just remember that Microsoft Word is NOT a text editor!}. In the
case of Python such code can then be run on the Raspberry Pi using the included
Python interpreter (see [Running Python Programs](python.md#sec:run-python)).
Code written in C needs to be compiled first into an executable using the QNX C
compiler. The resulting executable can then be copied over to the Raspberry Pi.

When using an editor on your computer the source files are typically saved on
the computer's own file system. In the case of Python code these files will have
to be copied over to the Raspberry Pi before they can be run with the Python
interpreter. An alternative to copying is to mount a directory from the
Raspberry Pi file system into your computer, using SSHFS, which is available
both for Linux and Windows. The following example mounts the
**/data/home/qnxuser/python** directory on a Linux user's **qnxrpi**
directory (assuming both directories exist):

```
$ mount qnxuser@qnxpi.local:python ./qnxrpi
```

Once this command is executed any file saved on **qnxrpi** from the
computer will be seen under **/data/home/qnxuser/python**, and updates will
be kept in sync. This way you can write Python code on your computer and run it on
the Raspberry Pi without constantly copying the files after every update.

Finally, it is possible to use VSCode[^2.2] with the QNX plugin to edit, build,
deploy and debug code. The plugin also allows you to inspect the system, e.g.,
by listing running processes, analyzing memory usage and collecting trace logs
for system activity. Install the QNX Toolkit extension, which is available on
the VSCode Extension Marketplace.

## Troubleshooting {#sec:troubleshoot}

What if the system doesn't boot, or it does but you cannot connect to it? Before
we start diagnosing any issues with the QNX image for Raspberry Pi, ensure that
the board itself is properly connected and powered.

**Does the red LED on the board turn on?**

: If not, the board is not powered. Check that the board is connected to a proper
power supply. The official power supply for Raspberry Pi is 5.1V and 3A.

**Does the firmware detect the SD card?**

: Connect the Raspberry Pi to a display, using one of the micro-HDMI connectors on
the board. If the SD card is not properly inserted you will see a message on the
display that no system image was found.

**Does the firmware detect the image?**

: A sign of the firmware starting the system image is a multi-coloured rectangle
displayed on the screen.

Any of the above issues are indicative of a problem with the setup of the board
(or, if you are very unlucky, with the board itself), and not with the QNX
image. Assuming the firmware appears to boot fine, we need to examine what is
not working with the QNX image.

**Is there any output?**

: If not, try to use the official Raspbian image instead, enabling its UART output
option. Again, follow tutorials on how to do that, ensuring that you have
connected the USB-TTL converter correctly, that the terminal program is
connected to the right device (e.g., **/dev/ttyUSB0** on Linux) and with
the correct settings: a baud rate of 115200, 8 bits, no parity, 1 stop bit and
no control flow. On Windows, make sure that the driver for the converter is
installed by checking the device manager.

**Do you see errors from the QNX image about a failure to mount file systems?**

: Some SD cards are not recognized by the SD driver. Try a different
card, preferably from a different manufacturer.

**Does the system boot to a command prompt, but there is no network connection?**

: Run `ifconfig` and check that the **bcm0** interface
shows up and is associated with an IP address. If not, ensure that the network
configuration in **wpa_supplicant.conf** are correct.

:::::: Note ::::::
Most logs on a QNX system are accessible by running the `slog2info`
command. This command will dump all logs from all processes. You can refine it
with the `-b <component>` command, where `<component>` should be
replaced by the name of the process. For example, `devb_sdmmc_bcm2711`
is the SD card driver.
::::::::::::::::::


[^2.1]: <https://sourceforge.net/projects/win32diskimager>
[^2.2]: <https://code.visualstudio.com>
[^2.3]: <https://www.raspberrypi.com/software/>
