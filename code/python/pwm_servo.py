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
