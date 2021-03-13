read V
while [ -n "$V" ]
do
    echo port1_pins.7=$V
    sleep 0.01
    read V
done
