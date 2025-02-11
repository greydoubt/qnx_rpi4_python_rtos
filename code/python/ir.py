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
