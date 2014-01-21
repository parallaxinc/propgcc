
This is a work in progress!

The Activity Bot (ABS) is a Parallax Propeller product.

This code represents a client-server application between a PC (client)
and the ABS with a Digi S6B Wifi module (server).

A few things need to be done before this code will work.

Basics:

* This program requires Propeller-GCC
* Ensure FTDI drivers are up-to-date
* This build requires make and msys/mingw on windows.
* Use an up to date browser Chrome, Firefox, Internet Explorer, or Safari

Procedure:

* Install the Digi S6B Wifi module
* Add jumper from DO to P13
* Add jumper from DI to P12
* Connect Activity Bot to USB port
* Build and run the xbee-loader/xbee-config program (make config)
* Setup the SSID and WPA2 password (only WPA2 supported at this time)
* Build and run the xbee-abs/server (make run)
* Finally load the xbee-abs/html/index.html page

When the page starts, the user must set the IP Address in the text box.

After setting IP Address, click the HELLO button. The ABS server console
should show activity and the Hello! message should appear in the text
area at the bottom of the web page.

If the HELLO test works, then you should be able to click the P26 and P27
LEDs on the web page ABS picture and see the ABS LEDs light. Click the
web page ABS leds again to turn them off.


More features will be added to this demo as time goes on.


Client/Server messaging (command/response/effect):

XPING                   /Hello!         /Hello! message printed on text area.

XABS PIN 26 HIGH        /Got request    /LED P26 turns on
XABS PIN 27 HIGH        /Got request    /LED P27 turns on
XABS PIN 26 LOW         /Got request    /LED P26 turns off
XABS PIN 27 LOW         /Got request    /LED P27 turns off

XABS PIN # HIGH         /Got request    /PIN # output set high 
XABS PIN # LOW          /Got request    /PIN # output set low 
XABS PIN # TOGGLE       /Got request    /PIN # output toggles
XABS PIN # INPUT        /Got request    /PIN # set to input

More to come ... (we miss you Johnny Carson).

