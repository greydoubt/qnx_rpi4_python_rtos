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

GPIO.add_event_detect(20, GPIO.BOTH, callback=buttonPressed)

while True:
    time.sleep(1)

