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
