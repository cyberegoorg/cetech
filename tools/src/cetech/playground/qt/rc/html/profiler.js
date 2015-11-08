var container = document.getElementById('vizu');
var frame_id = document.getElementById('frame_id');
// Create a DataSet (allows two way data-binding)

var items = new vis.DataSet();
items.clear()

// Configuration for the Timeline
var options = {
 // timeAxis: {scale: 'minute', step: 1}
    //showMajorLabels: false,

    timeAxis: {scale: 'millisecond', step: 1},

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
}

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

function update() {
    var val = JSON.parse(RecordEventWidget.get_data(parseInt(frame_id.value)))

    items.clear()
    items.add(val)
    timeline.fit()

    //console.log(items)
}

document.getElementById('draw').onclick = update

//setInterval(update, 1000)
