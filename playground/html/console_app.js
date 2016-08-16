///<reference path="ace.d.ts"/>
"use strict";
require("ace");
var ConsoleApp = (function () {
    function ConsoleApp() {
        var _this = this;
        this.editor = ace.edit("editor");
        this.editor.$blockScrolling = Infinity;
        this.editor.setTheme("ace/theme/solarized_dark");
        this.editor.getSession().setMode("ace/mode/lua");
        // editor.getSession().on("change", function(e){
        // });
        this.editor.setOptions({
            enableBasicAutocompletion: true,
            enableSnippets: true
        });
        this.editor.setFontSize("15px");
        this.editor.commands.addCommand({
            name: "send",
            exec: function () { _this.send_cmd(); },
            bindKey: { win: "alt-enter" }
        });
        this.editor2 = ace.edit("editor2");
        this.editor.$blockScrolling = Infinity;
        this.editor2.setTheme("ace/theme/solarized_dark");
        this.editor2.getSession().setMode("ace/mode/json");
        this.editor2.setFontSize("15px");
    }
    ConsoleApp.prototype.send_cmd = function () {
        var script = this.editor.getValue();
        var msg = msgpack.encode({
            name: "luasystem.execute",
            args: {
                script: script
            }
        });
        var header = new Uint8Array([255, 255, 255, 255]);
        var tmp = new Uint8Array(header.byteLength + msg.byteLength);
        tmp.set(header, 0);
        tmp.set(msg, header.byteLength);
        this.ws.send(tmp.buffer);
    };
    ConsoleApp.prototype.parse_data = function (events) {
        console.log(events);
        this.editor2.setValue(JSON.stringify(events, null, 2));
    };
    ConsoleApp.prototype.connect_to_cetech = function () {
        var _this = this;
        var cetech_url = document.getElementById("cetech_url");
        this.ws = new WebSocket(cetech_url.value, "rep.sp.nanomsg.org");
        this.ws.binaryType = "arraybuffer";
        this.ws.onopen = function () {
            console.log("opend");
        };
        this.ws.onclose = function () {
            console.log("closed");
        };
        this.ws.onmessage = function (evt) {
            var data = new Uint8Array(evt.data);
            var msg_data = data.subarray(4);
            var events = msgpack.decode(msg_data);
            _this.parse_data(events);
        };
        this.ws2 = new WebSocket("ws://localhost:4445", "pub.sp.nanomsg.org");
        this.ws2.binaryType = "arraybuffer";
        this.ws2.onopen = function () {
            console.log("opend");
        };
        this.ws2.onclose = function () {
            console.log("closed");
        };
        this.ws2.onmessage = function (evt) {
            var data = new Uint8Array(evt.data);
            var msg_data = data; //.subarray(4);
            var events = msgpack.decode(msg_data);
            var table = document.getElementById("log_table");
            var row = table.insertRow(1);
            var level_to_class = {
                info: "info",
                warning: "warning",
                error: "danger",
                debug: "success"
            };
            var level_to_icon = {
                info: "glyphicon-info-sign",
                warning: "glyphicon-warning-sign",
                error: "glyphicon-remove-sign",
                debug: "glyphicon-ok-sign"
            };
            row.className += level_to_class[events['level']];
            var level = row.insertCell(0);
            var where = row.insertCell(1);
            var worker_id = row.insertCell(2);
            var message = row.insertCell(3);
            level.innerHTML = "<span aria-hidden=\"true\" class=\"glyphicon  " + level_to_icon[events['level']] + "\"></span>";
            where.innerHTML = events['where'];
            worker_id.innerHTML = events['worker_id'];
            message.innerHTML = events['msg'];
            console.log(events);
        };
    };
    ;
    return ConsoleApp;
}());
//# sourceMappingURL=console_app.js.map