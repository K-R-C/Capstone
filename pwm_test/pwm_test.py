#! /usr/bin/python

with open('/sys/class/pwm/pwmchip0/pwm0/duty_cycle', 'w') as f:
    while True:
        for i in [1, 999]:
            print(f'{i}', end='', file=f)
