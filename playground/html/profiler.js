var container = document.getElementById('vizu');
var items = new vis.DataSet();

var chart = new SmoothieChart(),
    canvas = document.getElementById('smoothie-chart'),
    series = new TimeSeries();

var options = {
    timeAxis: {scale: 'millisecond', step: 0.01},
    showMajorLabels: false,

    order: function (a, b) {
        if (a.depth > b.depth)
            return -1;
        if (a.depth < b.depth)
            return 1;

        if (a.start < b.start)
            return -1;
        if (a.start > b.start)
            return 1;

        return 0;
    }
};

// TODO: dynamic or static? (N=64)
var groups = [
    {
        id: 0,
        content: 'Worker 0'
    },
    {
        id: 1,
        content: 'Worker 1'
    },
    {
        id: 2,
        content: 'Worker 2'
    },
    {
        id: 3,
        content: 'Worker 3'
    }
];

var timeline = new vis.Timeline(container, items, groups, options);

var Record = false;

ws = new WebSocket("ws://localhost:4447", "pub.sp.nanomsg.org");
ws.binaryType = "arraybuffer";
ws.onopen = function () {
    console.log("opend");
};
ws.onclosed = function () {
    console.log("closed");
};
ws.onmessage = function (evt) {
    var events = msgpack.decode(new Uint8Array(evt.data));
    var data_window = [];

    for (var i = 0; i < events.length; ++i) {
        var event = events[i];

        if (event.etype == "EVENT_RECORD_FLOAT") {
            if (event.name == "engine.delta_time") {
                series.append(new Date().getTime(), events[i].value);
            }
        } else if (event.etype == "EVENT_SCOPE") {
            if (!Record) {
                continue;
            }

            var label = event.name + ": " + (event.duration) + "ms,\n depth: " + event.depth;

            var item = {
                content: label,
                title: label,
                start: event.start,
                end: event.start + (event.duration),
                group: event.worker_id,
                depth: event.depth
            };

            data_window.push(item);
        }
    }

    if(Record) {
        console.log(data_window.length);

        items.clear();
        items.add(data_window);
        timeline.fit();

        Record = false;
    }
};

document.getElementById('start').onclick = function () {
    Record = true;
};

chart.addTimeSeries(series, {lineWidth: 2, strokeStyle: '#00ff00'});
chart.streamTo(canvas, 500);
