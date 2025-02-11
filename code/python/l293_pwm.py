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
