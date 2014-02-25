VAR
    long my_pin
    long my_other_pin
    
OBJ
    blinker : "blinker"
    
PUB set_pin(pin)
    my_other_pin := my_pin
    my_pin := pin

PUB set_pins(pin1, pin2)
    my_pin := pin1
    my_other_pin := pin2

PUB swap_pins | tmp
    tmp := my_pin
    my_pin := my_other_pin
    my_other_pin := tmp

PUB show_pins
    outa[my_pin] := 1
    outa[my_other_pin] := 0
    dira[my_pin] := 1
    dira[my_other_pin] := 1
    
PUB blink
    repeat 5
        show_pins
        waitcnt(clkfreq / 2 + CNT)
        swap_pins

PUB start_blinker
    blinker.start