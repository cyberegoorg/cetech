var chart = new SmoothieChart(),
    canvas = document.getElementById('smoothie-chart'),
    series = new TimeSeries();

chart.addTimeSeries(series, {lineWidth: 2, strokeStyle: '#00ff00'});
chart.streamTo(canvas, 500);

ws = new WebSocket("ws://localhost:8889", "pub.sp.nanomsg.org");
ws.binaryType = "arraybuffer";
ws.onopen = function () {
    console.log("opend");
};
ws.onclosed = function () {
    console.log("closed");
};

window.performance = window.performance || {};
performance.now = (function () {
    return performance.now ||
        performance.mozNow ||
        performance.msNow ||
        performance.oNow ||
        performance.webkitNow ||
        Date.now;
    /*none found - fallback to browser default */
})();

ws.onmessage = function (evt) {
    var events = msgpack.decode(new Uint8Array(evt.data));

    if (events.msg_type != 'pub') {
        return;
    }

    events = events.msg.pub;

    for (var i = 0; i < events.length; ++i) {
        var event = events[i];

        if (event.etype == "EVENT_RECORD_FLOAT") {
            if (event.name == "engine.delta_time") {
                series.append(new Date().getTime(), events[i].value);
            }
        }
    }
};

