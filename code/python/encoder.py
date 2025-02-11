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
