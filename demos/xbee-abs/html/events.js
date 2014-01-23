var texta;
var ipaddr;
var rqst;

function clearText()
{
    texta = document.getElementById("response");
    texta.value = "";
}

function newCORSRequest(method, url)
{
    var xhr = new XMLHttpRequest();
    texta = document.getElementById("response");
    texta.value = "";
    //texta.value = "Got new XHR ";

    if ("withCredentials" in xhr)
    {
        // Check if the XMLHttpRequest object has a "withCredentials" property.
        // "withCredentials" only exists on XMLHTTPRequest2 objects.
        //texta.value += "Opened withCredentials";
        xhr.open(method, url, true);
    }
    else if (typeof XDomainRequest != "undefined")
    {
        // Otherwise, check if XDomainRequest.
        // XDomainRequest only exists in IE, and is IE's way of making CORS requests.
        //texta.value += "Opened XDomainRequest";
        xhr = new XDomainRequest();
        xhr.open(method, url);
    }
    else
    {
        // Otherwise, CORS is not supported by the browser.
        xhr = null;
        texta.value += "XHR null";
    }
    return xhr;
}

function hello()
{
    var d26led= document.getElementById("D26_LED");
    var d27led= document.getElementById("D27_LED");

    sendHello();

    sendCommand("PIN 26 LOW");
    d26led.style.backgroundColor = "gray";
    sendCommand("PIN 27 LOW");
    d27led.style.backgroundColor = "gray";
}

function sendHello()
{
    texta = document.getElementById("response");
    ipaddr= document.getElementById("ipaddr");
    //port  = document.getElementById("port");

    var method = "XPING";
    //var url = "http://"+ipaddr.value+":"+port.value;
    var url = "http://"+ipaddr.value;

    try {
        var xhr = newCORSRequest(method, url);
        if(!xhr) {
            throw new Error('CORS not supported');
        }
        xhr.onreadystatechange=function() {
            startDinTimer();
            if (xhr.readyState==4 && xhr.status==200) {
                texta.value += xhr.responseText;
            }
        }
        xhr.setRequestHeader("Content-Type", "text/plain");
        startDotTimer();
        xhr.send();
    }
    catch (err) {
        texta.value += "\nXHR Request Exception "+err;
    }
}

function sendCommand(cmd)
{
    texta = document.getElementById("response");
    ipaddr= document.getElementById("ipaddr");
    //port  = document.getElementById("port");

    var method = "XABS";
    //var url = "http://"+ipaddr.value+":"+port.value;
    var url = "http://"+ipaddr.value;

    try {
        var xhr = newCORSRequest(method, url);
        if(!xhr) {
            throw new Error('CORS not supported');
        }
        xhr.onreadystatechange=function() {
            startDinTimer();
            if (xhr.readyState==4 && xhr.status==200) {
                texta.value += "\n" + xhr.readyState + " " + xhr.status + " ";
                texta.value += xhr.statusText + " " + xhr.responseText;
                texta.value += " Done";
            }
        }
        xhr.setRequestHeader("Content-Type", "text/plain");
        startDotTimer();
        xhr.send(cmd);
    }
    catch (err) {
        texta.value += "\nXHR Request Exception "+err;
    }
}

function clickLED(name)
{
    var led = document.getElementById(name);
    if(!led) {
        return;
    }
    if(led.style.backgroundColor != "yellow") {
        if(led.id == "D26_LED") {
            sendCommand("PIN 26 HIGH");
            led.style.backgroundColor = "yellow";
        }
        else
        if(led.id == "D27_LED") {
            sendCommand("PIN 27 HIGH");
            led.style.backgroundColor = "yellow";
        }
    }
    else {
        if(led.id == "D26_LED") {
            sendCommand("PIN 26 LOW");
        }
        else
        if(led.id == "D27_LED") {
            sendCommand("PIN 27 LOW");
        }
        led.style.backgroundColor = "gray";
    }
    startShine(led);
}

function shineLED(name)
{
    var led = document.getElementById(name);
    if(!led) {
        return;
    }
    startShine(led);
}

var ledsOn = false;

/* toggleLeds, but don't send packets
 */
function toggleLeds()
{
    var leds = {};
    var canvas = document.getElementById("canvas");
    if(ledsOn) {
        ledsOn = false;
    } else {
        ledsOn = true;
    }
    for (var n in canvas.children) {
        var led  = canvas.children[n];
        var name = led.id;
        if(name && name.indexOf("_LED") > 0) {
            if(ledsOn) {
                if(name.indexOf("PWR") < 0) {
                    led.style.backgroundColor = "yellow";
                } else {
                    led.style.backgroundColor = "lime";
                }
            } else {
                led.style.backgroundColor = "gray";
            }
        }
    }
}

function clickUpArrow()
{
}

function clickDownArrow()
{
}

function clickLeftArrow()
{
}

function clickRightArrow()
{
}


