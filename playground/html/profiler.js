var container = document.getElementById('vizu');
var items = new vis.DataSet();

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

function ab2str(buf) {
    return String.fromCharCode.apply(null, new Uint8Array(buf));
}

function diff_time(start, end) {
    var temp = [0, 0];
    if( (end[1] - start[1]) < 0) {
        temp[0] = end[0] - start[0] - 1;
        temp[1] = 1000000000 + end[1] - start[1];
    } else {
        temp[0] = end[0] - start[0];
        temp[1] = end[1] - start[1];
    }
    return temp;
}
function parse_data(data) {
    jsyaml.safeLoadAll(data, function (doc) {
        doc.events.forEach(function (event) {
            if (event.etype != 'EVENT_SCOPE') {
                return;
            }

            if (sample_count < 100) {
                sample_count += 1;
                return;
            }
            sample_count = 0;


            var t_s = [event.start, event.start_ns];
            var t_e = [event.end, event.end_ns];
            var delta_ms = diff_time(t_s, t_e)[1] / 1000000;
            var label = event.name + ": " + delta_ms + "ms, depth: " + event.depth;

            var item = {
                content: label,
                title: label,
                start: (t_s[0] * 1000) + (t_s[1] / 1000000),
                end: (t_e[0] * 1000) + (t_e[1] / 1000000),
                group: event.worker_id,
                depth: event.depth
            };

            data_window.push(item);
        });
    });
}

ws = new WebSocket("ws://localhost:5556", "pub.sp.nanomsg.org");
ws.binaryType = "arraybuffer";
ws.onopen = function () {
    console.log("opend");
};
ws.onclosed = function () {
    console.log("closed");
};
ws.onmessage = function (evt) {
    if (!Record) {
        return;
    }

    var data = ab2str(evt.data);

    if (data.indexOf("#develop_manager") != 0) {
        return;
    }

    parse_data(data);
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
