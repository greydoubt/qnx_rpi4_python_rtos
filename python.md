---
title: Controlling I/O with Python
author: Elad Lahav
chapter: 4
---

# Controlling I/O with Python {#chapter:python}

In this chapter we will get the Raspberry Pi to do some useful (and fun!) work,
by controlling various external devices. Such control will be achieved by
writing code in the Python programming language and running it on the Raspberry
Pi. To complete the exercises you will need a few components, including:

* breadboard
* jumper wires
* various resistors
* LEDs
* breadboard push buttons
* DC motor
* two or more servos
* infrared LED and a photodiode
* PCF8591 Analog-to-digital converter
* 10K$\Omega$ potentiometer
* PCA9685-based 16-channel PWM board
* L293 or L298 H-bridge
* external DC power supply (a battery pack will do)

You may already have some or all of these components. If not, there are many
startup kits available for the Raspberry Pi that include all of these. Such kits
often come with their own Python tutorials, and these tutorials can be much more
comprehensive than what is available in this book. There are also books
dedicated to maker projects and robotics with the Raspberry Pi, and many of
those also use Python. Feel free to use any of these resources on top of, or in
lieu, of the information in this chapter. The Python libraries for GPIO and I2C
included with the QNX image for Raspberry Pi have been designed to be as
compatible as possible with those available for other operating systems. You can
then come back to [How Does It Work?](#sec:python_qnx) to learn how the Python
code you write allows the Raspberry Pi to control external devices.

:::: Warning :::::
Never power a high-current device (such as a motor or a servo) directly from the
Raspberry Pi's GPIO header. Such devices should get their power from an external
source that matches their voltage and current requirements.
::::::::::::::::::

## Running Python Programs {#sec:run-python}

[Writing Code](#sec:write-code) describes the various methods in which you can
write code for programs to run on the image. Since Python is an interpreted
language it needs a special program, the Python interpreter, which runs the code
directly from the source file (or files). There is no need first to compile the
code into an executable. As with writing code, there are a few options for
running the resulting program on the Raspberry Pi. For the following examples we
will use a trivial Python program to print "Hello World!":

~~~ {#hello.py .py .numberLines}
print("Hello World!")
~~~

### Using the command line

Before we can run the program we need to save the code in a file, e.g.,
**hello.py**. Henceforth we will assume that the files for all of the
Python examples in this book are stored in a folder called **python** under
the home directory of the **qnxuser** user. The full path for a file called
**hello.py** is thus **/data/home/qnxuser/python/hello.py** or, using
an abbreviated notation,
**\textasciitilde qnxuser/python/hello.py**. If the directory does not yet
exist, log into the Raspberry Pi and create it with the `mkdir` command:

```
qnxuser@qnxpi:~$ mkdir ~/python
```

If you write the code directly on the Raspberry Pi, or if you use SSHFS to mount
a directory from the Raspberry Pi into your image, then the file is now stored
in the target directory. On the other hand, if you saved it on your local
computer then it needs to be copied to the Raspberry Pi:

```
$ scp hello.py qnxuser@qnxpi:python/
```

We can now invoke the python interpreter to run the program. If you haven't done
so already, log into the Raspberry Pi with SSH and type the following commands:

```
qnxuser@qnxpi:~$ cd ~/python
qnxuser@qnxpi:~/python$ python hello.py
Hello World!
```

### Remote Execution

While SSH can be used to start a shell on the Raspberry Pi, it can also be used
to execute a command remotely. The following commands first copy the file from
the computer to the Raspberry Pi, and then run the program:

```
$ scp hello.py qnxuser@qnxpi:python/
$ ssh -t qnxuser@qnxpi python python/hello.py
```

:::::: Note ::::::
Make sure to specify `-t` as part of the SSH command. Without it, the
program may continue running on the Raspberry Pi even after the SSH command
exits.
::::::::::::::::::

## Basic Output (LED) {#sec:basic_output}

Our first project will blink a light-emitting diode (LED). You will need a
breadboard, an LED (any colour will do), a small resistor and two jumper
wires. The value of the resistor depends on the LED but for the purpose of this
simple experiment any value in the range of a 30$\Omega$ to 330$\Omega$ will do. The
purpose of the resistor is to limit the current and protect the LED from burning
out.

Build the circuit by following these steps. See
[GPIO Header](gpio.md#chapter:gpio_header) for pin number
assignment. Note that GPIO numbers and pin numbers are not the same.

1. Connect the long leg of the LED (the positive leg, or *anode*) to any
   hole in the breadboard.
2. Connect the short leg of the LED (the negative leg, or *cathode*) to a
   different hole. Make sure that different hole is also in a different row, as
   all holes within the same row are connected.
3. Connect one leg of the resistor to a hole in the same row as the LED long
   leg.
4. Connect the other leg of the resistor to a hole in a row different from
   either the long or short leg of the LED.
5. Connect a jumper wire such that one end plugs into a hole in the same row
   as the resistor leg in step 4, and the other end is connected to pin 36 in the
   Raspberry Pi GPIO header. This is GPIO 16.
6. Connect another jumper wire such that one end plugs into a hole in the
   same row as the short leg of the LED and the other end is connected to pin 14
   in the Raspberry Pi GPIO header. This is a ground pin.

:::::: Note ::::::
The jumper wires can be connected to any other pin in the GPIO header, as long
as the short leg of the LED is connected to a ground pin and the resistor is
connected to a GPIO pin. If you decide to use a different GPIO pin make sure to
adjust the code such that it uses the correct number.
::::::::::::::::::

The following diagram illustrates the circuit:

![A basic LED circuit](images/led_circuit.png){#fig:led_circuit}

Next, we will write the code for making the LED blink:

~~~ {#led.py .py .numberLines}
import rpi_gpio as GPIO
import time

GPIO.setup(16, GPIO.OUT)
GPIO.output(16, GPIO.LOW)

while True:
    GPIO.output(16, GPIO.HIGH)
    time.sleep(.5)
    GPIO.output(16, GPIO.LOW)
    time.sleep(.5)
~~~

Save this file as **led.py**, and run the program as described in the
previous section. For example, if using the command line while connected via
SSH, run the command:

```
qnxuser@qnxpi:~/python$ python led.py
```

If all goes well the LED should turn on and off at half second
intervals. You can press Ctrl-C to stop it.

If the program doesn't work, try the following steps to determine what has gone
wrong:

1. Double-check all connections.
2. Ensure the jumper wires are connected to the correct GPIO header pins.
3. Connect the positive jumper wire to a 3v pin instead of a GPIO pin (e.g.,
   pin number 1). The LED should turn on. If it doesn't, try a different LED (in
   case this one is burnt out).
4. Check your code to ensure that it is exactly the same as in the
   listing. If you connected the positive wire to a different GPIO pin, make sure
   that all references to 16 have been replace by the right number.

We will now take a closer look at the code. The first two lines make the
necessary libraries available to the program. The `rpi_gpio` library
provides the functionality for working with the GPIO pins. It
is made visible using an alias `GPIO` so that the code matches examples
found on the Internet for similar libraries written for other operating systems,
such as the Raspbian. The time library is used to add the necessary delays via
the `time.sleep` call.

Next, the program prepares the GPIO to be used as an output. Every GPIO can
function as either an output or an input. Many GPIO pins also have other
functions, some of which will be explored in the next sections. The program then
enters an infinite loop in which the LED's state is toggled between high (on)
and low (off), using a delay of 500 milliseconds between each state change.

:::::: Note ::::::
The code in this program was stripped to the bare minimum, eschewing
functionality that may be considered as best practice. For example, it does
not ensure that the GPIO state is set back as low when the program is
terminated.
::::::::::::::::::

## Basic Input (Push Button) {#sec:basic_input}

The next exercise adds a push button to the LED circuit. A push button usually
has 4 legs, such that each pair protruding from opposite
sides is permanently connected. When the button is pressed all four legs become
connected, allowing current to flow. We will use the Raspberry Pi to detect when
the button is pressed and turn on the LED.

Modify the circuit from the previous exercise to match the following diagram:

![A circuit with a push button and an LED](images/button_circuit.png){#fig:button_circuit}

Note that the Raspberry Pi ground pin is now connected to the breadboard's
ground bus. This allows us to have a common ground for all components, without
using more ground pins from the Raspberry Pi. The LED's anode is connected to
GPIO 16 as before. The push button is connected to ground on one leg and to GPIO
20 on another. The permanently connected legs straddle the trough in the middle
of the breadboard.

Save the following program as **button.py**:

~~~{#button.py .py .numberLines}
import rpi_gpio as GPIO
import time

GPIO.setup(16, GPIO.OUT)
GPIO.output(16, GPIO.LOW)

GPIO.setup(20, GPIO.IN, GPIO.PUD_UP)

while True:
    if GPIO.input(20) == GPIO.LOW:
        GPIO.output(16, GPIO.HIGH)
    else:
        GPIO.output(16, GPIO.LOW)

    time.sleep(.01)
~~~

The first lines are identical to those in the previous exercise. Line 7 defines
GPIO 20 as input. The `GPIO.PUD_UP` argument tells the Raspberry Pi to
use its internal pull-up resistor on this pin. Without a pull-up resistor the
state of the pin is undetermined (or "floating"), which means that the
connection to ground established by pushing the button may not be
detected. Pulling up means that the pin detects a high state by default, and
then a low state (from the connection to ground) when the button is pressed. We
could have replaced the connection of the button to ground with a
connection to a 3V pin and used `GPIO.PUD_DOWN`, in which case the
default state of GPIO 20 would have been low, and would change to high when the
button is pressed. The internal pull resistors in the Raspberry Pi avoid the
need for using discrete resistors in the circuit to achieve the same effect.

The loop on lines 9-15 checks the state of GPIO 20 every 10 milliseconds. If the
state is high (the default, due to the pull-up resistor) then the LED is turned
off. If the state is low that means that the button is pressed and the LED is
turned on.

Having a check run every 10 milliseconds can waste processor cycles, while at
the same time miss some events (especially if the button is replaced by some
other detector for events that happen at a higher frequency). Instead of polling
for button state changes, we can ask the system to detect such changes and
notify our program when they happen:

~~~{#button_event.py .py .numberLines}
import rpi_gpio as GPIO
import time

def buttonPressed(pin):
    if GPIO.input(20) == GPIO.LOW:
        GPIO.output(16, GPIO.HIGH)
    else:
        GPIO.output(16, GPIO.LOW)

GPIO.setup(16, GPIO.OUT)
GPIO.output(16, GPIO.LOW)

GPIO.setup(20, GPIO.IN, GPIO.PUD_UP)

GPIO.add_event_detect(20, GPIO.BOTH, callback = buttonPressed)

while True:
    time.sleep(1)
~~~

The call to `GPIO.add_event_detect()` installs a callback function
which is executed whenever GPIO 20 detects a change from high to low ("falling
edge") or from low to high ("rising edge"). The callback can be restricted to
just one of these changes, by changing `GPIO.BOTH` to
`GPIO.FALLING` or `GPIO.RISING`, respectively. The loop on lines
17-18 just sleeps, but can be replaced with some other code to perform tasks
while still servicing button presses.

One problem that may arise when dealing with inputs is that the value can change
rapidly, or "bounce", before it stabilizes. This is not an issue in this
exercise, as the state of the LED follows that of the button. If, however, you
change the program such that the state of the LED changes with every button
press (i.e., the LED is turned on when the button is pressed, and
then turned off when the button is pressed a second time), you may
find that on some presses the state of the LED is not the desired one. Fixing
such problems is referred to as "debouncing", which can be done either
electronically (e.g., by adding a capacitor), or in code (e.g., by requiring the
same value to be read some number of times consecutively before deciding that
the value has changed).

\bigskip

It may seem redundant, and somewhat excessive, to use a 4-core, multi-gigabyte
computer to achieve the same effect as connecting the button directly to the
LED. In a real project the button can be used to initiate a much more
interesting activity. Also, the input may not be a push button at all. Other
common components that provide discrete input (i.e., input that is either "on"
or "off") include magnetic reed switches, photo-resistors detecting infrared
light and various sensors.

## PWM {#sec:pwm}

### Background

The GPIO pins on the Raspberry Pi are only capable of discrete output - each
pin, when functioning as an output, can be either in a high state or a low
state. Sometimes, however, it is necessary to have intermediate values, e.g., to
vary the brightness of an LED or to generate sound at different pitch
levels. One option for accomplishing such tasks is to use a digital to analog
converter, which is a separate device that can be connected to the Raspberry Pi
over a bus such as I2C or SPI (see below). Alternatively, we can use Pulse Width
Modulation (PWM) to approximate an intermediate state.

PWM works by changing the state of a GPIO output repeatedly at
regular intervals. Within a period of time, the state is set to high for a
certain fraction of that time, and to low for the remainder. If the period is
long enough the result is a noticeable pulse. With a short period the effect is
perceived as an intermediate value. What constitutes "short" and "long"
depends on the device and human perception.

PWM is specified using two values: the period, which is the repeating interval,
and the duty cycle, which is the fraction of the period in which the output is
high. The period can be specified in units of time (typically milliseconds or
microseconds), or in terms of frequency (specified in Hz). Thus a period of 20ms
is equivalent to a frequency of 50Hz. The duty cycle can be specified as a
percentage of the period, or in time units. A duty cycle of 1ms for a period of
10ms is thus 10\%. Period and duty cycle are demonstrated in Figure @fig:pwm.

![Period and duty cycle in PWM](images/pwm.png){#fig:pwm}

It is possible to implement PWM in software, by configuring any GPIO pin as an
output and using a repeating timer to toggle its state. However, the Raspberry
Pi provides a hardware implementation, albeit on just four GPIO pins: 12, 13, 18
and 19. Note, however, that only two PWM channels are available: channel 1 for
GPIOs 12 and 18, and channel 2 for GPIOs 13 and 19. This means that if both
GPIO 13 and 19 are configured for hardware PWM then they will have the same
period and duty cycle. Moreover, both channels share the same clock, further
limiting their independence. Expansion boards are available that allow for more
PWM channels with finer control, and these are recommended for applications that
require multiple PWM sources, such as robotic arms.

In addition to the period and duty cycle, each PWM pin can be configured to work
in one of two modes of operation. The first mode, referred to in the manual as
M/S mode, works exactly as depicted in Figure @fig:pwm, i.e., the GPIO
is kept high for the duration of the duty cycle and then low for the
remainder. The second mode spreads the duty cycle more or less evenly across the
period, with several transitions from low to high and back. This mode more
closely emulates an analog output and is preferred when such output is
desired. On the other hand, M/S mode is used with devices that expect a signal
with exact characteristics as a mode of communication. This is the case for
servo motors, which will be described below.

### Fading LED

For our first experiment with PWM, build the following circuit. This circuit is
similar to the simple LED circuit from the first exercise, only this time we use
GPIO 12, which is one of the pins that supports hardware PWM.

![LED circuit connected to a PWM-enabled GPIO pin](images/pwm_led_circuit.png){#fig:pwm_led_circuit}

Now save this program as **pwm_led.py** and run it. You should see the LED
fade in and out.

~~~{#pwm_led.py .py .numberLines}
import rpi_gpio as GPIO
import time

pwm = GPIO.PWM(12, 1000, mode = GPIO.PWM.MODE_PWM)

while True:
    for dc in range(0, 100, 5):
        pwm.ChangeDutyCycle(dc)
        time.sleep(.1)

    for dc in range(100, 0, -5):
        pwm.ChangeDutyCycle(dc)
        time.sleep(.1)
~~~

Line 4 creates a PWM object using GPIO 12 with a frequency of 1KHz and the
standard PWM mode (not M/S). The loop on lines 7-9 changes the duty cycles from
0\% to 100\% in increments of 5\% every 100 milliseconds, while the loop on
lines 11-13 does the opposite.

### Servo

Next, we will use PWM to control a servo motor. A servo is an electric motor
that has a control mechanism that allows for exact positioning of the shaft. The
angle of the shaft is communicated to the motor via PWM, where the period is
fixed and the duty cycle is used to determine the angle.

There are many different types of servo motors, but one of the most common for
beginners is the SG90, which is marketed under different brand names. This is an
inexpensive, albeit weak, servo, but will do for the purpose of this
exercise. The shaft can only be rotated 180 degrees between a minimum position
and a maximum position.

The servo has three wires: brown (ground), red (power) and orange
(control). Connect the servo as in the diagram:

![Servo circuit](images/pwm_servo_circuit.png){#fig:pwm_servo_circuit}

Note that we have added an external power source (the diagram shows 1.5V
batteries connected in series, but you can substitute a different DC source).
The required voltage for the SG90 servo is between 4.8V and 6V. As mentioned
before, do not use the Raspberry Pi 5V pin to power the servo, as it may exceed
the current limit for this pin and damage the Raspberry Pi. It is important,
however, to make sure that the ground terminal of the external power source is
connected to a ground pin on the Raspberry Pi (in this case both are connected
to the ground bus on the breadboard).

The orange wire is connected to GPIO 12 for control via PWM. To control the
servo we have to use a period of 20 milliseconds (or a frequency of 50Hz). The
servo's motor is in its minimum position (0°) when the duty cycle is 0.5
milliseconds (or 2.5%), and at its maximum position (180°) when the duty
cycle is 2.5 milliseconds (or 12.5%). Intermediate duty cycle values position
the motor in between these two angles.

:::::: Note ::::::
Some datasheets claim that the minimum duty cycle is 1ms and the maximum is
2ms. It is not clear whether that is a mistake, or that different vendors
manufacture SG90 servos with different control parameters. You can experiment
with various values to see that you get a full 180° motion. Just be
careful not to exceed the minimum or maximum values for too long, to avoid
strain on the gears.
::::::::::::::::::

Save this program as **pwm_servo.py** and run it. It should cause the
servo's arm to rotate to one side and then the other, going through several
angle changes.

~~~{#pwm_servo.py .py .numberLines}
import rpi_gpio as GPIO
import time

pwm = GPIO.PWM(12, 50, mode = GPIO.PWM.MODE_MS)

MIN = 2.5
MAX = 12.5
dc = MIN
while dc <= MAX:
    pwm.ChangeDutyCycle(dc)
    dc += 0.5
    time.sleep(.5)

while dc >= MIN:
    dc -= 0.5
    pwm.ChangeDutyCycle(dc)
    time.sleep(.5)
~~~

Line 4 creates a PWM object using GPIO 12, with a frequency of 50Hz. Note that
the mode must be set to M/S, as the control mechanism expects a continuous high
level for the duty cycle in each period. Lines 8-12 move the shaft from 0°
to 180° in regular increments every 500 milliseconds, and lines 14-17 do
the opposite.

## I2C {#sec:i2c}

### Background

All of the examples so far have only used a few GPIO pins. Once you start
building less trivial systems you may find that a 40-pin header is simply not
enough. Consider an LED matrix with dozens of diodes, a robotic arm that
requires multiple servos (remember that the Raspberry Pi has only two PWM
channels) or a sensor that provides a wide range of values. Instead of using a
GPIO pin for each bit of data, the Raspberry Pi can connect to external devices
using a communication bus. The simplest of these is the Inter-Integrated
Circuit bus, commonly referred to as I2C.

The I2C bus requires just two pins - a serial data pin (SDA) and a serial clock
pin (SCL). The data pin is used to communicate data to the device, by switching
between low and high states, while the clock pin synchronizes these changes such
that the target device can interpret them as a stream of bits. The device can
respond with its own stream of bits, for bi-directional communication. Pin 3 on
the Raspberry Pi is the SDA pin, while pin 5 is the SCL pin.

Each I2C device has its own interpretation of this bit stream, of which the
controlling device needs to be aware. This establishes a device-specific
protocol on top of the I2C transport layer.

How does I2C solve the problem of replacing multiple GPIO pins? For one, the bit
stream can be used to convey much more information between the Raspberry Pi and
a device than a simple on/off state, or even PWM. For example, an LED matrix can
be told to change the state of an LED at a given row and column, by interpreting
the bit stream as command packets. An analog-to-digital converter replies to
requests from the Raspberry Pi with bits that are interpreted as a value in a
certain range.

Moreover, multiple devices can be connected to each other, forming a chain. Each
device has an address (typically 7 bits, for a maximum of 128 addresses in a
chain), and will respond only to communication that is preceded by its address
(which is part of the bit stream sent by the Raspberry Pi).

The following sections show two examples of I2C devices and how to communicate
with them. You may not have these devices (though both are cheap and readily
available), but the principles should be the same for all I2C devices.

### PCF8591 Digital/Analog Converter

PCF8591 is a simple 8-bit analog-to-digital and digital-to-analog converter. The
device comes in the form of a 16-pin integrated circuit chip, though there are
different packages (some with an extra carrier board). The device address is 72
(or 48 in hex), plus the value obtained from the chip's three address bit
pins. Each of these address pins can be connected to ground for 0 or to the
source voltage for 1, giving a total of 8 possible addresses between 72 and 79
(inclusive).

The device provides four analog inputs, which can be read by sending the byte 64
(or 40 in hex), plus the number of the input, between 0 and 3. Thus sending the
byte 64 on address 72 reads the first analog input, byte 65 on address 72 the
second input, etc. The value returned is between 0 and 255.

In this experiment we will use a 10K$\Omega$ potentiometer to change the value
of input 0, read the result and print it to the console. The build is more
complicated than in previous exercises, so make sure to check all
connections. The PCF8591 chip should be inserted into the breadboard such that
it straddles the middle trough, and with the semi-circular notch oriented as in
the diagram:

![Analog-to-digital converter with a potentiometer](images/pcf8591_circuit.png){#fig:pcf8591_circuit}

Pin 1 from the Raspberry Pi, which provides a 3.3V source, is connected to one
of the positive buses in the breadboard. That bus is then connected to the
second positive bus on the other side of the breadboard. Similarly, pin 14
(ground) is connected to the first negative bus, which is then connected to the
second negative bus. This configuration makes it easy to connect multiple pins
from the PCF8591 chip to the source voltage and to ground.

The yellow and green wires in the diagram are connected to the SDA and SCL pins,
respectively, on both the Raspberry Pi and the PCF8591 chip. These pins on the
chip are also connected via 10K$\Omega$ pull-up resistors to the source
voltage (or else they will not be able to detect changes on these pins). All of
the address pins are connected to ground, which means that the device address
is 72 (48 hex).

Finally, a 10K$\Omega$ potentiometer is connected to the source voltage, analog
input 0 on the PCF8591, and ground.

Run the following program, and then turn the potentiometer to see the output.

~~~ {#adc.py .py .numberLines}
import smbus
import time

adc = smbus.SMBus(1)

prev_value = 0
while True:
    value = adc.read_byte_data(0x48, 0x40)
    if prev_value != value:
        print('New value: {}'.format(value))
        prev_value = value

    time.sleep(0.1)
~~~

Line 4 establishes a connection to the I2C resource manager. The number 1 in the
argument to `smbus.SMBus()` matches the I2C bus number, as assigned by the
resource manager (not to be confused with the address assigned to the device,
the ADC converter in this case, attached to that bus). If you get an error on
this line check which device was registered by the resource manager:

```
qnxuser@qnxpi:~$ ls /dev/i2c*
/dev/i2c1
```

If the result is different than `/dev/i2c1` change the code to match the
number in the device name.

Line 8 sends the command 0x40 on address 0x48 in order to request the value for
analog input 0 from the device. It then prints the value if it has changed since
the last time it was read. Turning the potentiometer all the way to one end
should result in the value 0, the other end in the value 255, and intermediate
positions in values between these two.

### PCA9685 16 Channel PWM Controller

PCA9685 is a 16-channel, I2C-controlled integrated circuit. Each channel can be
controlled individually by specifying two 12-bit values (i.e., values between 0
and 4095). The first value determines the time within the PWM period in which
the channel turns on and the second value determines the time in which it is
turned off. The period is determined by a common frequency value, which is
derived from the internal oscillator running at 25MHz and a programmable
pre-scale value. For example, for a frequency of 1KHz (i.e., a period of 1ms),
if the first value is 300, and the second value is 1600, then the
channel will turn on 73 microseconds after the beginning of the period and turn
off 390 microseconds after the beginning of the period, resulting in a duty
cycle of 31.7\%.

Several vendors provide a board that contains a PCA9685 chip and 16 3-pin
headers for connecting servos. Such a board is an easy and cheap way to control
multiple servos. As mentioned before the Raspberry Pi only provides two PWM
channels, which prevents the use of more than two independently-controlled
servos at the same time.

The board connects to the Raspberry Pi using 4 wires: 3.3V, ground, SDA and
SCL. An independent power source is connected to the two power terminals, which
is used to power the servos (recall that you cannot use the 5V output from the
Raspberry Pi as it cannot sustain the current requirements of the
servos). Finally, up to 16 servos can be connected to the 3-pin headers.
Figure @fig:pca9685_circuit  illustrates a circuit with two servo motors.

![PCA9685 circuit with two servos](images/pca9685_circuit.png){#fig:pca9685_circuit}

The chip is controlled by writing to I2C address 0x40 by default. It is
possible to change this address by soldering one or more of the pads in the
corner of the board, allowing for multiple devices to be connected on the same
I2C bus. The two channel values are programmed with two bytes each, the first for
the lower 8 bits, the second for the upper 2 bits. This means that each channel
requires 4 bytes. These start at command number 6 for channel 0, and increment
by four for each channel up to 15. Commands 0 and 1 are used to configure the
chip, while command 254 configures the pre-scalar value to determine the
frequency.

The following program controls the two servos, which are connected to channels 0
and 8, respectively, as in the diagram:

~~~ {#pca9685.py .py .numberLines}
import smbus
import time

smbus = smbus.SMBus(1)

# Configure PWM for 50Hz
smbus.write_byte_data(0x40, 0x0, 0x10)
smbus.write_byte_data(0x40, 0xfe, 0x7f)
smbus.write_byte_data(0x40, 0x0, 0xa0)
time.sleep(0.001)

# Servo 1 at 0 degrees
smbus.write_block_data(0x40, 0x6, [0x0, 0x0, 0x66, 0x0])

# Servo 2 at 180 degrees
smbus.write_block_data(0x40, 0x26, [0x0, 0x0, 0x0, 0x2])

time.sleep(1)

# Servo 1 at 180 degrees
smbus.write_block_data(0x40, 0x6, [0x0, 0x0, 0x0, 0x2])

# Servo 2 at 0 degrees
smbus.write_block_data(0x40, 0x26, [0x0, 0x0, 0x66, 0x0])
~~~

Lines 7-10 set the pre-scalar for a frequency of 50Hz.[^4.1] This requires
putting the chip to sleep first, and then restarting it, followed by a short
delay. When restarting, the chip is also configured for command auto-increment,
which means that we can write the per-channel 4-byte values in one I2C
transaction. Line 13 writes the four byte values for channel 0, starting at
command 6. These values say that the channel should turn on immediately at the
beginning of a period, and turn off after 102 (0x66) steps out of 4096, giving a
duty cycle of 2.5\%. Line 16 configures channel 8, starting at command 38
(0x26), to turn on immediately at the beginning of the period and turn off after
512 steps out of 4096, giving a duty cycle of 12.5\%. After one second each
servo is turned the opposite way.

## Towards Robotics: Motor Control {#sec:motor}

### DC Motor with an H-Bridge

There are different kinds of motors used in robotics. We have already
encountered servo motors, which provide precise control of the shaft
angle. Servos, however, provide a limited motion range, and are not suitable for
robot locomotion.[^4.2] The simplest way to drive a robot is with a DC motor. The
motor itself has no control mechanism: once connected to a voltage source and
ground it moves at a constant speed in one direction. For a robot we would like
to have control over the direction and the speed of the motor.

:::::: Note ::::::
The typical DC motor is too fast and too weak to move a robot. It is therefore
coupled with a gear box the slows the motor down while increasing its torque.
::::::::::::::::::

Controlling the direction can be done with a device called an
*H-Bridge*, a fairly simple circuit that allows for the polarity of the
connection to the motor to be switched. Thus, instead of one terminal being
permanently connected to a voltage source and the other to ground, the two can
change roles, reversing the direction of the motor.

While you can build an H-bridge yourself with a few transistors, it is easier to
use an integrated circuit, such as L293D. This integrated circuit provides two
H-bridges, which means it can control two motors at a time. Another popular
choice is L298, which can work with higher voltage and is more suitable for
bigger robots.

Figure @fig:l293d_circuit shows a circuit that combines a L293D chip with a
5V DC motor. The chip has two voltage source connections: one for the chip's
logic, connected to a 5V pin on the Raspberry Pi, and the other for the motor,
connected to an external power source. Inputs 1 and 2 of the L293D chip are
connected to GPIOs 20 and 16, respectively, while outputs 1 and 2 are connected
to the DC motor. GPIO 19 is connected to the pin that enables outputs 1 and
2. Note that the two ground buses are connected (by the left-most black wire) to
ensure a common ground between the Raspberry Pi and the external power
source. The four ground pins of the L293D chip are connected to the ground
buses.

![L293D circuit with a DC motor](images/l293d_circuit.png){#fig:l293d_circuit}

The motor is at rest when both inputs are the same (either high or low), rotates
one way when input 1 is high and input 2 is low, and the other way when input 1
is low and input 2 is high. The following program demonstrates this.

~~~{#l293.py .py .numberLines}
import rpi_gpio as GPIO
import time

GPIO.setup(20, GPIO.OUT)
GPIO.setup(16, GPIO.OUT)
GPIO.setup(19, GPIO.OUT)

GPIO.output(19, GPIO.HIGH)

GPIO.output(20, GPIO.HIGH)
GPIO.output(16, GPIO.LOW)
time.sleep(1)

GPIO.output(20, GPIO.LOW)
GPIO.output(16, GPIO.HIGH)
time.sleep(1)

GPIO.output(20, GPIO.LOW)
GPIO.output(16, GPIO.LOW)
GPIO.output(19, GPIO.LOW)
~~~

Line 8 enables the outputs. Lines 10-12 cause the motor to rotate one way for 1
second. Lines 13-16 reverse the motor's direction for one second. Then both
outputs, as well as the enable pin, are turned off.

So far we have only used the L293D chip to control the direction of the
motor. We can also control the speed by using PWM on the enable pin, which in
our circuit is connected to GPIO 19. A duty cycle of 100\% works as though the
pin is always enabled, which means that the motor rotates at full speed. Lower
values of the duty cycle slow the motor down. The next program starts the motor
at full speed one way, slows it down to a halt, switches direction and then
speeds up to full speed.

~~~{#l293_pwm.py .py .numberLines}
import rpi_gpio as GPIO
import time

GPIO.setup(20, GPIO.OUT)
GPIO.setup(16, GPIO.OUT)
pwm = GPIO.PWM(19, 20, mode = GPIO.PWM.MODE_MS)

GPIO.output(20, GPIO.HIGH)
GPIO.output(16, GPIO.LOW)

for dc in range (100, 0, -1):
    pwm.ChangeDutyCycle(dc)
    time.sleep(0.1)

GPIO.output(20, GPIO.LOW)
GPIO.output(16, GPIO.HIGH)

for dc in range (0, 100):
    pwm.ChangeDutyCycle(dc)
    time.sleep(0.1)

pwm.ChangeDutyCycle(0)
GPIO.output(20, GPIO.LOW)
GPIO.output(16, GPIO.LOW)
~~~

### Encoders

The level of control provided above is not sufficient for accurate positioning
of a robot. Even if we set the speed of each wheel to a desired value and
measure for a specific amount of time, there is no guarantee that the robot will
move exactly as desired. The motors may take some time to reach the desired
speed, and friction may affect movement. To solve this problem, we can use
encoders, which tell us exactly how much each wheel has moved.

It is possible to purchase DC motors with built-in encoders. However, in the spirit
of this book, we will create one from scratch using a pair diodes: an infrared
LED and an infrared photodiode. First, construct a basic circuit to see these
diodes at work, as depicted in Figure @fig:ir_circuit.

![Infrared LED and photodiode circuit](images/ir_circuit.png){#fig:ir_circuit}

The clear diode is the infrared (henceforth IR) LED, while the black one is the
photodiode. The IR LED is connected to 3.3V via a small resistor (27$\Omega$ in
the example), which makes it permanently on. (An alternative is to connect the
LED to a GPIO output and turn it on as needed). The photodiode's cathode (short
leg) is directly connected to 3.3V (unlike an LED), while the anode (long leg) is
connected via a large resistor (10K$\Omega$ in the example) to ground. A
connection to GPIO 4 is made between the anode and the resistor.

If the photodiode does not detect IR light (which it should not in the configuration
depicted in the diagram) then it does not pass any current. In this case the
voltage level read by GPIO 4 is 0, as it is connected to ground via the
resistor. However, when the photodiode detects IR light, which can be done by
placing a white piece of paper over the heads to the two diodes, it passes
current, which creates a voltage difference across the resistor. GPIO 4 detects
this difference as a high input.

The next program reports how many times the photodiode detected IR light in the
span of 5 seconds. Once you run it, move a white piece of paper back and forth
at a small distance over the diodes, such that it alternates between covering
and not covering these. The program should report how many times it transitioned
between the non-covered and the covered states.

~~~ {#ir.py .py .numberLines}
import rpi_gpio as GPIO
import time

count = 0

def ir_detect(pin):
    global count
    count += 1

GPIO.setup(4, GPIO.IN, GPIO.PUD_DOWN)
GPIO.add_event_detect(4, GPIO.RISING, callback = ir_detect)

time.sleep(5)

print('Photodiode detected {} signals'.format(count))
~~~

To control a motor with IR light we add an *encoder wheel* to the motor. The
encoder wheel is a disc with slots that can be detected by the photodiode as
the motor rotates. Counting the slots allows the motor to be stopped by a
program once the desired angle has been reached (which can be more than
360°). Using the angle and the circumference of the robot wheel attached
to the motor (not the encoder disc) we can determine the distance that wheel
has traversed. Figure @fig:encoder_wheel shows an encoder wheel with 9 slots.

![An encoder wheel](images/encoder_wheel.png){#fig:encoder_wheel}

The slots can be holes in the disc, in which case the IR diodes are arranged
as in Figure @fig:encoder_wheel. Alternatively, the disc can have black
stripes on a white background (or vice versa), in which case the diodes should
be placed on the same side and use the reflection of the IR light for detection.
For the purpose of this exercise you can use a 3D printer for creating an
encoder wheel, or just make one out of cardboard. If opting for a slotted wheel,
make sure that whatever material you use blocks IR light (printed plastic may
need to be painted).

We will now combine the last two circuits for a full motor control device. The
circuit, depicted in Figure @fig:encoder_circuit is somewhat complicated by
the need to supply three different voltage sources: 5V from the Raspberry Pi to
power the L293D chip, 3.3V from the Raspberry Pi for the IR LED and photodiode,
and external power for the motor.[^4.3]

![Motor and encoder circuit](images/encoder_circuit.png){#fig:encoder_circuit}

The program for this circuit starts the motor at a 40\% PWM duty-cycle (you can
change this value depending on the rotation speed of your motor), and then stops
it once 9 transitions from low to high have been detected. Assuming the encoder
has 9 slots, as in the example, this code should stop the motor after one full
rotation of the wheel.

~~~ {#encoder.py .py .numberLines}
import rpi_gpio as GPIO
import time

encoder_detect = 0

def stop_motor(pin):
    global encoder_detect
    encoder_detect += 1
    if encoder_detect == 9:
        GPIO.output(20, GPIO.LOW)
        GPIO.output(16, GPIO.LOW)
        print('Stopping motor')

GPIO.setup(4, GPIO.IN, GPIO.PUD_DOWN)
GPIO.add_event_detect(4, GPIO.RISING, callback = stop_motor)

GPIO.setup(20, GPIO.OUT)
GPIO.setup(16, GPIO.OUT)
pwm = GPIO.PWM(19, 20, mode = GPIO.PWM.MODE_MS)

GPIO.output(20, GPIO.HIGH)
GPIO.output(16, GPIO.LOW)

pwm.ChangeDutyCycle(40)

while encoder_detect < 9:
    time.sleep(0.1)

pwm.ChangeDutyCycle(0)
GPIO.output(20, GPIO.LOW)
GPIO.output(16, GPIO.LOW)
~~~

When running this program you may find that the motor is stopped well after the
time it is supposed to. There are a couple of reasons for that:

1. Python is not very efficient. It is an interpreted language, with poor
   support for concurrency. By the time the thread that runs the callback
   function handles the events sent by the GPIO resource manager they may have
   been queued for quite some time.
2. Python's threads execute at the system's normal priority, which means that
   their scheduling is affected by that of other threads in the system (as well
   as each other).

We can alleviate the problem by using the `match` argument to
`GPIO.add_event_detect()`, which allows the GPIO resource manager's
event thread to keep track of changes, and notify the Python program only once
the requested number of changes have occurred. This does not solve the problem
fully, as the callback that stops the motor still runs at normal system
priority. A better solution will be provided by running the same program using a
high-priority thread for motor control, a subject that will be revisited in
[Threads and Priorities](realtime.md#sec:threads).

~~~ {#encoder_match.py .py .numberLines}
import rpi_gpio as GPIO
import time

encoder_detect = False

def stop_motor(pin):
    global encoder_detect
    GPIO.output(20, GPIO.LOW)
    GPIO.output(16, GPIO.LOW)
    print('Stopping motor')
    encoder_detect = True

GPIO.setup(4, GPIO.IN, GPIO.PUD_DOWN)
GPIO.add_event_detect(4, GPIO.RISING, match = 9, callback = stop_motor)

GPIO.setup(20, GPIO.OUT)
GPIO.setup(16, GPIO.OUT)
pwm = GPIO.PWM(19, 20, mode = GPIO.PWM.MODE_MS)

GPIO.output(20, GPIO.HIGH)
GPIO.output(16, GPIO.LOW)

pwm.ChangeDutyCycle(40)

while encoder_detect == False:
    time.sleep(0.1)

pwm.ChangeDutyCycle(0)
GPIO.output(20, GPIO.LOW)
GPIO.output(16, GPIO.LOW)
~~~

## How Does It Work? {#sec:python_qnx}

At this point you may be curious to know how does a Python program control the
GPIO pins on the Raspberry Pi. (Of course, you may not be curious about it at
all, in which case you can skip this section).

GPIO control in the Raspberry Pi QNX image is implemented by a resource manager
(see [Resource Managers](system.md#sec:resmgr)) called **rpi_gpio**. The
resource manager exposes a few files under the path **/dev/gpio**. We have
already encountered this resource manager when we used the **echo** command to
write to a file that controls a single GPIO pin from the shell. One of the files
provided by the resource manager is **/dev/gpio/msg**, which can be opened by a
client program. That program can then use QNX native message passing functions
to request the resource manager to perform certain actions on its behalf.

The Python interpreter becomes such a client program when you import the
`rpi_gpio` library (both the resource manager and the library have the
same name, but are not to be confused). This library is implemented using
Python's C extensions.[^4.4] You can find the library file under
**/system/lib/python3.11/lib-dynload/rpi_gpio.so**. When loaded, the library
opens **/dev/gpio/msg**, keeping a file descriptor to the resource manager. It
then uses `MsgSend()` to send messages to the resource manager and wait for it
to reply. For example, the following code implements the library's `output()`
method, which was used in the LED example to turn the LED on and off:

~~~ {.c .numberLines}
static PyObject *
rpi_gpio_output(PyObject *self, PyObject *args)
{
    unsigned    gpio;
    unsigned    value;

    if (!PyArg_ParseTuple(args, "II", &gpio, &value)) {
        return NULL;
    }

    gpio = get_gpio(gpio);
    if (gpio == 0) {
        set_error("Invalid GPIO number");
        return NULL;
    }

    if ((value != 0) && (value != 1)) {
        set_error("Invalid GPIO output value");
        return NULL;
    }

    rpi_gpio_msg_t  msg = {
        .hdr.type = _IO_MSG,
        .hdr.subtype = RPI_GPIO_WRITE,
        .hdr.mgrid = RPI_GPIO_IOMGR,
        .gpio = gpio,
        .value = value
    };

    if (MsgSend(gpio_fd, &msg, sizeof(msg), NULL, 0) == -1) {
        set_error("RPI_GPIO_WRITE: %s", strerror(errno));
        return NULL;
    }

    Py_RETURN_NONE;
}
~~~

Lines 4-20 get the arguments passed to the function (the GPIO pin number and
value, which can be either 1 for "HIGH" or 0 for "LOW"), and validate
those. Lines 22-28 define a message, composed of a header and a payload. The
header tells the resource manager that this message wants to write to a GPIO,
while the payload tells it which GPIO pin to update and to what value. Finally,
lines 30-33 send the message, wait for the reply and check for errors.

The resource manager runs a loop around the `MsgReceive()` call, which
waits for client messages. When it gets such a message it uses the header to
determine what type of message it is and then handles it appropriately. In the
case of a `RPI_GPIO_WRITE` message the resource manager writes to the
GPIO hardware registers a value that matches the GPIO pin specified in the
payload of the message. If that value is 1 then it sets a bit in one of the
GPSET registers, while for a value of 0 it sets a bit in one of the GPCLR
registers. These registers are associated with physical addresses that the
resource manager maps into its address space when it starts. More information
about mapping hardware registers is provided in
[Controlling Hardware](realtime.md#sec:control-hardware).


[^4.1]: According to the datasheet the pre-scalar value for 50Hz is calculated as
     121, or 0x79. Nevertheless, an inspection with an oscilloscope revealed that
     a value of 0x7f is closer to the required frequency.
[^4.2]: Some servos have their internal gearbox modified to allow for continuous
     motion and then used for driving wheels. However, at that point they are
     really just standard motors.
[^4.3]: I was able to power the L293D from the 3.3V output of the Raspberry Pi,
     which simplifies the circuit. Nevertheless, the datasheet for the L293D
     specifies a minimum of 4.5V.
[^4.4]: <https://docs.python.org/3/extending/extending.html>
