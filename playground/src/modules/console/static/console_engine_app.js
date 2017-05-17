"use strict";
exports.__esModule = true;
///<reference path="../../lib/static/ace/ace.d.ts"/>
var engine_rpc_1 = require("../../lib/static/playground/engine_rpc");
var ConsoleEngineApp = (function () {
    function ConsoleEngineApp() {
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
        this.rpc = new engine_rpc_1.EngineRPC();
        this.sub = new engine_rpc_1.EngineSubscriber();
        this.sub.subscribe(function (msg) {
            var log_msg = msg;
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
    ConsoleEngineApp.prototype.send_cmd = function () {
        var _this = this;
        var script = this.editor.getValue();
        this.rpc.call("lua_system.execute", {
            script: script
        }, function (msg) {
            _this.editor2.setValue(JSON.stringify(msg, null, 2));
        });
    };
    ConsoleEngineApp.prototype.connect_to_cetech = function () {
        this.rpc.close();
        this.sub.close();
        var cetech_url = document.getElementById("cetech_url");
        this.rpc.connect(cetech_url.value);
        this.sub.connect("ws://localhost:4445");
    };
    ;
    return ConsoleEngineApp;
}());
exports.ConsoleEngineApp = ConsoleEngineApp;
//# sourceMappingURL=console_engine_app.js.map