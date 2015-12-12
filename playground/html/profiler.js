var container = document.getElementById('vizu');
//var frame_id = document.getElementById('frame_id');
// Create a DataSet (allows two way data-binding)

var items = new vis.DataSet();
items.clear();

// Configuration for the Timeline
var options = {
 // timeAxis: {scale: 'minute', step: 1}
    timeAxis: {scale: 'millisecond', step: 1},
    showMajorLabels: false,

//    moment: function(date) {
//        console.log(date)
//        return vis.moment(date).utc();
//    },


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

//function update() {
//    var val = JSON.parse(RecordEventWidget.get_data(parseInt(frame_id.value)));
//
//    items.clear();
//    items.add(val);
//    timeline.fit();
//
//    //console.log(items)
//}

//document.getElementById('draw').onclick = update;

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

ws = new WebSocket("ws://localhost:5556", "pub.sp.nanomsg.org");
ws.binaryType = "arraybuffer";
ws.onopen = function () {
    console.log("opend");
};
ws.onclosed = function () {
    console.log("closed");
};
ws.onmessage = function (evt) {
    //var yaml = require('js-yaml');
    //
    jsyaml.safeLoadAll(ab2str(evt.data), function (doc) {
        doc.events.forEach(function(event) {
            if(event.etype == 'EVENT_SCOPE') {
                var t_s = [event.start, event.start_ns];
                var t_e = [event.end, event.end_ns];
                var delta_ms = diff_time(t_s, t_e)[1] / 1000000;
                var label = event.name + ": " + delta_ms +"ms, depth: "+event.depth;

                var item = {
                    content: label,
                    title: label,
                    start: (t_s[0] * 1000) + (t_s[1] / 1000000),
                    end: (t_e[0] * 1000) + (t_e[1] / 1000000),
                    group: event.worker_id,
                    depth: event.depth
                }

                //items.clear();
                //items.add(item);
                //timeline.fit();

            }

            //console.log(entry);
        });
    });
};

//setInterval(update, 1000)
