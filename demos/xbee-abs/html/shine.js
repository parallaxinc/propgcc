
var shineCount;

var shine1;
var shine2;

var shinyTimerVar;
function startShine(led)
{
    shine1 = document.getElementById("shine1");
    shine2 = document.getElementById("shine2");

    shine1.style.top  = (led.offsetTop-4)+"px";
    shine1.style.left = (led.offsetLeft-6)+"px";
    shine2.style.top  = (led.offsetTop-9)+"px";
    shine2.style.left = (led.offsetLeft-12)+"px";

    shineCount = 8;
    if(shinyTimerVar) {
        window.clearInterval(shinyTimerVar);
    }
    shinyTimerVar = window.setInterval(shinyTimer,150);
}

function shinyTimer()
{
    if(shineCount > 0) {
        if(shineCount % 4 == 0) {
            shine1.style.visibility = "visible";
            shine2.style.visibility = "hidden";
        }
        else
        if(shineCount % 4 == 3) {
            shine1.style.visibility = "visible";
            shine2.style.visibility = "visible";
        }
        else
        if(shineCount % 4 == 2) {
            shine1.style.visibility = "hidden";
            shine2.style.visibility = "visible";
        }
        else
        if(shineCount % 4 == 1) {
            shine1.style.visibility = "hidden";
            shine2.style.visibility = "hidden";
        }
    }
    else {
        window.clearInterval(shinyTimerVar);
    }
    shineCount--;
}

function shinning()
{
    return shineCount > 0 ? 1 : 0;
}

var dinled;
var dotled;
var dinCount = 0;
var dotCount = 0;
var dinTimerVar;
var dotTimerVar;

function startDinTimer()
{
    if(dinCount > 0) {
        return;
    }
    dinled= document.getElementById("DIN_LED");
    dinCount = 7;
    dotTimerVar = window.setInterval(dinTimer, 50);
}

function startDotTimer()
{
    if(dotCount > 0) {
        return;
    }
    dotled= document.getElementById("DOT_LED");
    dotCount = 7;
    dotTimerVar = window.setInterval(dotTimer, 50);
}

function dinTimer()
{
    if(dinCount > -1) {
        if(dinCount % 3) {
            dinled.style.backgroundColor = "red";
        }
        else {
            dinled.style.backgroundColor = "gray";
        }
    }
    else {
        window.clearInterval(dinTimerVar);
    }
    dinCount--;
}

function dotTimer()
{
    if(dotCount > -1) {
        if(dotCount % 3) {
            dotled.style.backgroundColor = "lightblue";
        }
        else {
            dotled.style.backgroundColor = "gray";
        }
    }
    else {
        window.clearInterval(dotTimerVar);
    }
    dotCount--;
}


