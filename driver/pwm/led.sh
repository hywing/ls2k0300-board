num=10
peroid=100000
max=100000
min=10
flag=0
while true
do
	if [ $flag -eq 0 ]
	then 
		if [ $num -lt $max ]
		then  
			echo 0 > /sys/class/pwm/pwmchip0/pwm0/enable
#			echo $peroid > /sys/class/pwm/pwmchip0/pwm0/period
			echo $num > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
			echo "normal" > /sys/class/pwm/pwmchip0/pwm0/polarity
			echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable
			sleep 0.1
			let num+=10000	
		else
			flag=1
		fi
		
		
	else
		if [ $num -gt $min ]
		then  
			echo 0 > /sys/class/pwm/pwmchip0/pwm0/enable
#			echo $peroid > /sys/class/pwm/pwmchip0/pwm0/period
			echo $num > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
			echo "normal" > /sys/class/pwm/pwmchip0/pwm0/polarity
			echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable
			sleep 0.1
			let num-=10000
		else
			flag=0
		fi	
		
	fi	
done

