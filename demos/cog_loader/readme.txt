To run this demo on a board that has LEDs on P0-P6, use the following command:

propeller-load -b<board-type> toggle.elf -r -t

To run this demo on a board with LEDs starting at <n>, use the following command:

propeller-load -b<board-type> -Dbasepin=<n> toggle.elf -r -t

For example, to run this demo on the QuickStart board, use this command:

propeller-load -bquickstart -Dbasepin=16 toggle.elf -r -t
