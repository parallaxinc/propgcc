To build:

source setenv.linux
make

To load a cog or lmm binary and start the terminal emulator:

./bin/linux/propeller-elf-load -p <port> -b <board> fibo.elf -r -t

To create a Spin binary for the Propeller Tool:

./bin/linux/propeller-elf-load -b <board> fibo.elf -s

The <port> parmeter will default to whatever the PROPELLER_LOAD_PORT environment variable is set to.

The <board> parameter will default to whatever the PROPELLER_LOAD_BOARD environment variable is set to
If there is no PROPELLER_LOAD_BOARD environment variable <board> will default to "default"

The loader uses a board configuration file called propeller-elf-load.cfg.
It uses the following algorithm for locating the configuration file:

1) look in the directory specified by the -I command line option
2) look in the directory where the elf file resides
3) look in the directory pointed to by the environment variable PROPELLER_LOAD_PATH
4) look in the directory where the loader executable resides if a path was given on the command line

The loader has built in configuration settings for the "default" configuration that match most
common Propeller boards.

The definition of the default configuration is:

[default]
    clkfreq: 80000000
    clkmode: XTAL1+PLL16X
    baudrate: 115200
    rxpin: 31
    txpin: 30
    tvpin: 12
