System.register(["modules/lib/static/playground/playground_rpc"], function(exports_1, context_1) {
    "use strict";
    var __moduleName = context_1 && context_1.id;
    var playground_rpc_1;
    var ConsoleApp;
    return {
        setters:[
            function (playground_rpc_1_1) {
                playground_rpc_1 = playground_rpc_1_1;
            }],
        execute: function() {
            ConsoleApp = (function () {
                function ConsoleApp() {
                    var _this = this;
                    this.level_to_class = {
                        info: "info",
                        warning: "warning",
                        error: "danger",
                        debug: "success"
                    };
                    this.level_to_icon = {
                        info: "glyphicon-info-sign",
                        warning: "glyphicon-warning-sign",
                        error: "glyphicon-remove-sign",
                        debug: "glyphicon-ok-sign"
                    };
                    this.rpc = new playground_rpc_1.PlaygroundRPC();
                    this.sub = new playground_rpc_1.PlaygroundSubscriber();
                    this.sub.subcribeService("engine_service", function (msg) {
                        if (msg.msg_type != 'log') {
                            return;
                        }
                        var log_msg = msg.msg.log;
                        var table = document.getElementById("log_table");
                        var row = table.insertRow(1);
                        row.className += _this.level_to_class[log_msg['level']];
                        var level = row.insertCell(0);
                        var where = row.insertCell(1);
                        var worker_id = row.insertCell(2);
                        var message = row.insertCell(3);
                        level.innerHTML = "<span aria-hidden=\"true\" class=\"glyphicon  " + _this.level_to_icon[log_msg['level']] + "\"></span>";
                        where.innerHTML = log_msg['where'];
                        worker_id.innerHTML = log_msg['worker_id'];
                        message.innerHTML = log_msg['msg'];
                    });
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
                    this.editor2.$blockScrolling = Infinity;
                    this.editor2.setTheme("ace/theme/solarized_dark");
                    this.editor2.getSession().setMode("ace/mode/json");
                    this.editor2.setFontSize("15px");
                    var btn = document.getElementById("btn_connect");
                    btn.onclick = function () {
                        _this.connect_to_cetech();
                    };
                    btn = document.getElementById("btn_send");
                    btn.onclick = function () {
                        _this.send_cmd();
                    };
                }
                ConsoleApp.prototype.send_cmd = function () {
                    var _this = this;
                    var script = this.editor.getValue();
                    this.rpc.callService("engine_service", "call", {
                        fce_name: "lua_execute",
                        instance_name: "level_view",
                        script: script
                    }, function (msg) {
                        _this.editor2.setValue(JSON.stringify(msg, null, 2));
                    });
                };
                ConsoleApp.prototype.connect_to_cetech = function () {
                    this.rpc.close();
                    this.sub.close();
                    var cetech_url = document.getElementById("cetech_url");
                    this.rpc.connect(cetech_url.value);
                    this.sub.connect("ws://localhost:8889");
                };
                ;
                return ConsoleApp;
            }());
            exports_1("ConsoleApp", ConsoleApp);
        }
    }
});
//# sourceMappingURL=console_app.js.map