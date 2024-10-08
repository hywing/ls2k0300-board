
num=1
while true
do
	echo 0 > /sys/class/pwm/pwmchip1/pwm0/enable
	echo 100000 > /sys/class/pwm/pwmchip1/pwm0/period
	echo $num > /sys/class/pwm/pwmchip1/pwm0/duty_cycle
	echo "inversed"  sys/erclass/pwm/pwmchip1/pwm0/polarity
	echo 1 > /sys/class/pwm/pwmchip1/pwm0/enable
	sleep 0.1
	let num+=50
done
