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
    texta.value = url;
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
    sendHello();

    var d26led= document.getElementById("D26_LED");
    var d27led= document.getElementById("D27_LED");
    d26led.style.backgroundColor = "gray";
    d27led.style.backgroundColor = "gray";

    /* Server now clears P26/27 on hello.
     * This keeps us from needing more than one request.
     * Sending more than one request for hello may not be
     * the best thing to do.
     *
    sendCommand("PIN 26 LOW");
    sendCommand("PIN 27 LOW");
     */
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
                texta.value += "\n"+xhr.responseText;
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
    /*
    while(cmd.indexOf(" ") > -1) {
        cmd = cmd.replace(" ", "/");
    }
    var url = "http://"+ipaddr.value+"/"+cmd;
     */

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
        //xhr.send();
    }
    catch (err) {
        texta.value += "\nXHR Request Exception "+err;
    }
}

function clickBattery()
{
    ipaddr= document.getElementById("ipaddr");
    texta = document.getElementById("response");

    var battery = document.getElementById("battery");
    var batchg  = document.getElementById("batchg");
    var batval  = document.getElementById("batval");
    var srvled  = document.getElementById("SRV_LED");

    var method = "XABS";
    var url = "http://"+ipaddr.value;

    try {
        var xhr = newCORSRequest(method, url);
        if(!xhr) {
            throw new Error('CORS not supported');
        }
        xhr.onreadystatechange=function() {
            var adc;
            startDinTimer();
            if (xhr.readyState==4 && xhr.status==200) {
                texta.value += "\n" + xhr.readyState + " " + xhr.status + " ";
                texta.value += xhr.statusText + " " + xhr.responseText;
                texta.value += " Done";
                var btop = battery.offsetTop;
                var bhgt = battery.offsetHeight;
                adc = parseInt(xhr.responseText);
                batchg.style.height = ((adc*2.0*bhgt*0.90)/4096.0)+"px";
                batchg.style.top = (btop+bhgt*0.95-batchg.offsetHeight)+"px";

                if(adc) {
                    srvled.style.backgroundColor = "lime";
                } else {
                    srvled.style.backgroundColor = "gray";
                }
                batval.textContent = (adc*15.0/4096.0).toPrecision(2)+"V";
            }
        }
        xhr.setRequestHeader("Content-Type", "text/plain");
        startDotTimer();
        xhr.send("ADC 0");
    }
    catch (err) {
        texta.value += "\nXHR Request Exception "+err;
    }
}

function clickSolarCell()
{
    ipaddr= document.getElementById("ipaddr");
    texta = document.getElementById("response");

    var solval  = document.getElementById("solval");

    var method = "XABS";
    var url = "http://"+ipaddr.value;

    try {
        var xhr = newCORSRequest(method, url);
        if(!xhr) {
            throw new Error('CORS not supported');
        }
        xhr.onreadystatechange=function() {
            var adc;
            startDinTimer();
            if (xhr.readyState==4 && xhr.status==200) {
                texta.value += "\n" + xhr.readyState + " " + xhr.status + " ";
                texta.value += xhr.statusText + " " + xhr.responseText;
                texta.value += " Done";
                adc = parseInt(xhr.responseText);
                solval.textContent = (adc*5.1/4096.0).toPrecision(3)+"mV";
            }
        }
        xhr.setRequestHeader("Content-Type", "text/plain");
        startDotTimer();
        xhr.send("ADC 3");
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

function clickStart()
{
    sendCommand("SERVO START");
}
function clickStop()
{
    sendCommand("SERVO STOP");
}

function stopBot()
{
    zeroArrows();
    sendCommand("SERVO RUN 0 0");
}

function zeroArrows()
{
    var fspd = document.getElementById("FWD_SPD");
    fspd.style.height = 0+"px";
    var dspd = document.getElementById("REV_SPD");
    dspd.style.height = 0+"px";
    var lspd = document.getElementById("LFT_SPD");
    lspd.style.width = 0+"px";
    var rspd = document.getElementById("RGT_SPD");
    rspd.style.width = 0+"px";

}

function clickUpArrow()
{
    var arrow = document.getElementById("up");
    var left  = -(arrow.offsetHeight-(event.clientY-arrow.offsetTop));
    var right =  (arrow.offsetHeight-(event.clientY-arrow.offsetTop));

    zeroArrows();
    var fspd = document.getElementById("FWD_SPD");
    fspd.style.height = right+"px";
    fspd.style.top    = (arrow.offsetTop+arrow.offsetHeight-right-1)+"px";

    sendCommand("SERVO RUN "+left+" "+right);
}

function clickDownArrow()
{
    var arrow = document.getElementById("down");
    var left  =  (event.clientY-arrow.offsetTop);
    var right = -(event.clientY-arrow.offsetTop);

    zeroArrows();
    var fspd = document.getElementById("REV_SPD");
    fspd.style.height = left+"px";
    fspd.style.top    = (arrow.offsetTop)+"px";

    sendCommand("SERVO RUN "+left+" "+right);
}

function clickLeftArrow()
{
    var arrow = document.getElementById("left");
    var left  = -(arrow.offsetWidth-(event.clientX-arrow.offsetLeft));
    var right = -(arrow.offsetWidth-(event.clientX-arrow.offsetLeft));

    zeroArrows();
    var lspd = document.getElementById("LFT_SPD");
    lspd.style.width  = -left+"px";
    lspd.style.left   = (arrow.offsetLeft+arrow.offsetWidth+left-1)+"px";

    right = 0;
    sendCommand("SERVO RUN "+left+" "+right);
}

function clickRightArrow()
{
    var arrow = document.getElementById("right");
    var left  =  (event.clientX-arrow.offsetLeft);
    var right =  (event.clientX-arrow.offsetLeft);

    zeroArrows();
    var rspd = document.getElementById("RGT_SPD");
    rspd.style.width  = left+"px";
    rspd.style.left   = (arrow.offsetLeft+1)+"px";

    left = 0;
    sendCommand("SERVO RUN "+left+" "+right);
}

