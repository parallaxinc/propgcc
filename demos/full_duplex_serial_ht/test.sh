#!/bin/bash

PORT="/dev/ttyUSB0"

# Display what is recieved from port aand...
cat < $PORT &

# The test message.
MSG="All work and no play makes Jack a dull bot."

count=0

while [ true ]
do
    # Send message to port
    echo "$COUNT $MSG" > $PORT

    COUNT=$(($COUNT + 1))

    # Wait a bit as the other end can't quite keep up
    sleep 0.004
done;
