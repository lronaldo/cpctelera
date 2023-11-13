if [ -f v09.hex ]; then
    cd ..
fi

if [ ! -f sm6809 ]; then
    echo >&2 "No simulator"
    exit 1
fi

./sm6809 -e uart0_base=0xe000,uart1_base=0xe008,pia0_base=0xe010,pia1_base=0xe020 -S uart=0,port=5555 test/v09.hex "$@"
