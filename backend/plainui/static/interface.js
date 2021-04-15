
const VEHICLE_ID = 'r01';

var serverConnection = null;
var canvas = null;
var ctx = null;

var mouse = null;

function Point(x, y) {
  this.x = x;
  this.y = y;
}

$(document).ready(function() {
    var url = 'ws://' + window.location.host + '/api/v1/ws/vehicle/' + VEHICLE_ID + '/subscribe?interval=150ms';
    serverConnection = new WebSocket(url);
    serverConnection.onmessage = handleMessage;
    controlVehicle();

    $("#btn-accelerate").click(function() {
        sendJoystick(0.15, 0.0);
    });

    $("#btn-decelerate").click(function() {
        sendJoystick(-0.15, 0.0);
    });

    $("#btn-turn-left").click(function() {
        sendJoystick(0.0, 0.08);
    });

    $("#btn-turn-right").click(function() {
        sendJoystick(0.0, -0.08);
    });

    // make the canvas internal coords match the element's size in HTML
    canvas = $('#image-canvas');
    canvas.attr('width', canvas.width());
    canvas.attr('height', canvas.height());
    canvas.mousemove(handleHover);
    canvas.click(handleClick);

    // get the drawing context
    ctx = canvas.get(0).getContext('2d');
});

function handleClick(ev) {
    var mouse = new Point(ev.clientX, ev.clientY);
    var bounds = canvas.get(0).getBoundingClientRect();
    var p = pointIn(bounds, mouse);
    sendPointAndGo(p.x / canvas.get(0).width, p.y / canvas.get(0).height);
}

function handleHover(ev) {
    var bounds = canvas.get(0).getBoundingClientRect();
    var mouse = new Point(ev.clientX, ev.clientY);
    draw(pointIn(bounds, mouse));
}

function pointIn(rect, p) {
    return new Point(p.x - rect.left, p.y - rect.top);
}

function draw(p) {
    ctx.clearRect(0, 0, canvas.width(), canvas.height());

    var w = canvas.width();
    var h = canvas.height();

    ctx.lineWidth = 4;
    ctx.strokeStyle = 'rgba(255, 0, 0, .75)';
    
    ctx.beginPath();
    ctx.moveTo(w/2-50, h);
    ctx.quadraticCurveTo(w/2-50, h-100, p.x-30, p.y);
    ctx.stroke();

    ctx.beginPath();
    ctx.moveTo(w/2+50, h);
    ctx.quadraticCurveTo(w/2+50, h-100, p.x+30, p.y);
    ctx.stroke();
}

var curFrame = null;
var prevFrame = null;

function handleMessage(message) {
    // do not do any realy work here or else the websocket will get backed up
    // (buffering cannot be turned off or observed in Chrome)
    curFrame = message;
}

function updateImage() {
    if (curFrame !== null && curFrame !== prevFrame) {
        prevFrame = curFrame;
        var msg = JSON.parse(curFrame.data);
        if (msg.frame) {
            var src = 'data:image/' + msg.frame.encoding + ';base64,' + msg.frame.content;
            $('#frame').attr('src', src);
        }
    }
}

setInterval(updateImage, 30);

function sendJoystick(linearVelocity, inverseRadius) {
    console.log("sending joystick command");
    serverConnection.send(JSON.stringify({
        "joystick": {
            "linearVelocity": linearVelocity,
            "curvature": inverseRadius,
        }
    }));
}

function sendPointAndGo(x, y) {
    console.log("sending point-and-go command");
    serverConnection.send(JSON.stringify({
        "pointAndGo": {
            "imageX": x,
            "imageY": y,
        }
    }));
}

function controlVehicle() {
    $.ajax({
        url: "/api/v1/vehicle/" + VEHICLE_ID + "/request-control",
        type: "GET",
        error: function(xhr, resp, text) {
            console.log(xhr, resp, text);
        }
    })
}