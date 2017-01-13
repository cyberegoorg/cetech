System.register(["./lib/playground/playground_rpc", "vis"], function(exports_1, context_1) {
    "use strict";
    var __moduleName = context_1 && context_1.id;
    var playground_rpc_1, vis;
    var ProfilerApp;
    return {
        setters:[
            function (playground_rpc_1_1) {
                playground_rpc_1 = playground_rpc_1_1;
            },
            function (vis_1) {
                vis = vis_1;
            }],
        execute: function() {
            ProfilerApp = (function () {
                function ProfilerApp() {
                    var _this = this;
                    this.sub = new playground_rpc_1.PlaygroundSubscriber();
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
                    var options = {
                        timeAxis: { scale: 'millisecond', step: 0.01 },
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
                    var data_window = [];
                    this.last_now = performance.now();
                    this.time_acum = 0.0;
                    var container = document.getElementById('vizu');
                    this.items = new vis.DataSet();
                    this.timeline = new vis.Timeline(container, this.items, groups, options);
                    this.Record = false;
                    this.sub.subcribe_service("engine_service", function (msg) {
                        if (!_this.Record) {
                            return;
                        }
                        if (msg.msg_type != 'pub') {
                            return;
                        }
                        if (msg.msg.instance_name != 'level_view') {
                            return;
                        }
                        var events = msg.msg.pub;
                        for (var i = 0; i < events.length; ++i) {
                            var event = events[i];
                            if (event.etype == "EVENT_SCOPE") {
                                var label = event.name + ": " + (event.duration) + "ms,\n depth: " + event.depth;
                                var item = {
                                    content: label,
                                    title: label,
                                    start: event.start,
                                    end: event.start + (event.duration),
                                    group: event.worker_id,
                                    depth: event.depth
                                };
                                _this.items.add(item);
                            }
                        }
                        if (_this.Record) {
                            if (_this.items.length >= 100) {
                                _this.Record = false;
                                _this.timeline.fit();
                            }
                        }
                    });
                    var btn = document.getElementById("btn_connect");
                    btn.onclick = function () {
                        _this.connect_to_cetech();
                    };
                    var btn = document.getElementById("btn_send");
                    btn.onclick = function () {
                        if (!_this.Record) {
                            _this.items.clear();
                        }
                        else {
                            _this.timeline.fit();
                        }
                        _this.Record = !_this.Record;
                    };
                }
                ProfilerApp.prototype.connect_to_cetech = function () {
                    this.sub.close();
                    var cetech_url = document.getElementById("cetech_url");
                    this.sub.connect("ws://localhost:8889");
                };
                ;
                return ProfilerApp;
            }());
            exports_1("ProfilerApp", ProfilerApp);
        }
    }
});
//# sourceMappingURL=profiler_app.js.map