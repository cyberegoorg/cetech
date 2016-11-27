///<reference path="ace/ace.d.ts"/>
///<reference path="vs.d.ts"/>

import {PlaygroundSubscriber} from "./lib/playground/playground_rpc";
import * as vis from "vis";

declare var msgpack: any;

export class ProfilerApp {
    sub: PlaygroundSubscriber;

    items: vis.DataSet<any>;
    timeline: vis.Timeline;

    Record: boolean;
    last_now: number;
    time_acum: number;

    constructor() {
        this.sub = new PlaygroundSubscriber();
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

        var options = {
            timeAxis: {scale: <vis.TimelineTimeAxisScaleType>'millisecond', step: 0.01},
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


        var container = <HTMLDivElement>document.getElementById('vizu');
        this.items = new vis.DataSet();
        this.timeline = new vis.Timeline(container, this.items, groups, options);

        this.Record = false;

        this.sub.subcribe_service("engine_service", (msg): void => {
            if (!this.Record) {
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

                    this.items.add(item);
                }
            }

            if (this.Record) {
                if (this.items.length >= 100) {
                    this.Record = false;
                    this.timeline.fit();
                }
                //this.items.clear();

                //this.Record = false;
            }
        });

        var btn = <HTMLButtonElement>document.getElementById("btn_connect");
        btn.onclick = () => {
            this.connect_to_cetech()
        };

        var btn = <HTMLButtonElement>document.getElementById("btn_send");
        btn.onclick = () => {
            if (!this.Record) {
                this.items.clear();
            } else {
                this.timeline.fit();
            }

            this.Record = !this.Record;
        };
    }


    connect_to_cetech() {
        this.sub.close();

        var cetech_url = <HTMLInputElement>document.getElementById("cetech_url");

        this.sub.connect("ws://localhost:8889")
    };
}
