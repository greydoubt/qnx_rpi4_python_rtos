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
