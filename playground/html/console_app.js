System.register([], function(exports_1, context_1) {
    "use strict";
    var __moduleName = context_1 && context_1.id;
    var msgpack, ConsoleApp;
    return {
        setters:[],
        execute: function() {
            ConsoleApp = (function () {
                function ConsoleApp() {
                    var _this = this;
                    this.editor = ace.edit("editor");
                    this.editor.$blockScrolling = Infinity;
                    this.editor.setTheme("ace/theme/solarized_dark");
                    this.editor.getSession().setMode("ace/mode/lua");
                    this.editor.setOptions({
                        enableBasicAutocompletion: true,
                        enableSnippets: true
                    });
                    this.editor.setFontSize("15px");
                    this.editor.commands.addCommand({
                        name: "send",
                        exec: function () {
                            _this.send_cmd();
                        },
                        bindKey: { win: "alt-enter" }
                    });
                    this.editor2 = ace.edit("editor2");
                    this.editor.$blockScrolling = Infinity;
                    this.editor2.setTheme("ace/theme/solarized_dark");
                    this.editor2.getSession().setMode("ace/mode/json");
                    this.editor2.setFontSize("15px");
                    var btn = document.getElementById("btn_connect");
                    btn.onclick = function () {
                        _this.connect_to_cetech();
                    };
                    var btn = document.getElementById("btn_send");
                    btn.onclick = function () {
                        _this.send_cmd();
                    };
                }
                ConsoleApp.prototype.send_cmd = function () {
                    var script = this.editor.getValue();
                    var msg = msgpack.encode({
                        name: "lua_system.execute",
                        args: {
                            script: script
                        }
                    });
                    var header = new Uint8Array([255, 255, 255, 255]);
                    var tmp = new Uint8Array(header.byteLength + msg.length);
                    tmp.set(header, 0);
                    tmp.set(msg, header.byteLength);
                    this.rpc_ws.send(tmp.buffer);
                };
                ConsoleApp.prototype.parse_data = function (events) {
                    this.editor2.setValue(JSON.stringify(events, null, 2));
                };
                ConsoleApp.prototype.connect_to_cetech = function () {
                    var _this = this;
                    if (this.rpc_ws != null) {
                        this.rpc_ws.close();
                    }
                    if (this.log_ws != null) {
                        this.log_ws.close();
                    }
                    var cetech_url = document.getElementById("cetech_url");
                    this.rpc_ws = new WebSocket(cetech_url.value, "rep.sp.nanomsg.org");
                    this.rpc_ws.binaryType = "arraybuffer";
                    this.rpc_ws.onopen = function () {
                        console.log("opend");
                    };
                    this.rpc_ws.onclose = function () {
                        console.log("closed");
                    };
                    this.rpc_ws.onmessage = function (evt) {
                        var data = new Uint8Array(evt.data);
                        var msg_data = data.subarray(4);
                        var events = msgpack.decode(msg_data);
                        _this.parse_data(events);
                    };
                    this.log_ws = new WebSocket("ws://localhost:4445", "pub.sp.nanomsg.org");
                    this.log_ws.binaryType = "arraybuffer";
                    this.log_ws.onopen = function () {
                        console.log("opend");
                    };
                    this.log_ws.onclose = function () {
                        console.log("closed");
                    };
                    this.log_ws.onmessage = function (evt) {
                        var msg_data = new Uint8Array(evt.data);
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
                    };
                };
                ;
                return ConsoleApp;
            }());
            exports_1("ConsoleApp", ConsoleApp);
        }
    }
});
//# sourceMappingURL=console_app.js.map