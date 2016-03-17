var container = document.getElementById('vizu');
var items = new vis.DataSet();

var chart = new SmoothieChart(),
    canvas = document.getElementById('smoothie-chart'),
    series = new TimeSeries();

var options = {
    timeAxis: {scale: 'millisecond', step: 1},
    showMajorLabels: false,

    order: function (a, b) {
        if(a.depth > b.depth)
            return -1;
        if(a.depth < b.depth)
            return 1;

        if(a.start < b.start)
            return -1;
        if(a.start > b.start)
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
  }
];

var timeline = new vis.Timeline(container, items, groups, options);

var data_window = [];
var Record = false;
var sample_count = 0;

function parse_data(data) {
    var events = msgpack.decode(data);

    events.EVENT_SCOPE.forEach(function (event) {
        if (sample_count < 10) {
            sample_count += 1;
            return;
        }
        sample_count = 0;

        var label = event.name + ": " + (event.end - event.start) + "ms, depth: " + event.depth;

        //console.log(event);

        var item = {
            content: label,
            title: label,
            start: event.start,
            end:  event.end,
            group: event.workerid,
            depth: event.depth
        };

        data_window.push(item);
    });
}

ws = new WebSocket("ws://localhost:5558", "pub.sp.nanomsg.org");
ws.binaryType = "arraybuffer";
ws.onopen = function () {
    console.log("opend");
};
ws.onclosed = function () {
    console.log("closed");
};
ws.onmessage = function (evt) {
    var events = msgpack.decode(new Uint8Array(evt.data));

    events.EVENT_RECORD_INT.forEach(function (event) {
        if(event.name != 'renderer.frame') {
            return
        }

        series.append(new Date().getTime(), event.value);
    });

    if (!Record) {
        return;
    }

    parse_data(events);
};

document.getElementById('start').onclick = function () {
    Record = true;
};

document.getElementById('stop').onclick = function () {
    Record = false;

    console.log(data_window.length);

    items.clear();
    items.add(data_window);
    timeline.fit();
};

chart.addTimeSeries(series, {lineWidth:2,strokeStyle:'#00ff00'});
chart.streamTo(canvas, 500);