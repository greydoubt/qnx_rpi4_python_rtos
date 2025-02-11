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
