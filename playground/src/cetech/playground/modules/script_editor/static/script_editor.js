System.register(["../../lib/static/playground/playground_rpc"], function(exports_1, context_1) {
    "use strict";
    var __moduleName = context_1 && context_1.id;
    var playground_rpc_1;
    var ScriptEditor;
    return {
        setters:[
            function (playground_rpc_1_1) {
                playground_rpc_1 = playground_rpc_1_1;
            }],
        execute: function() {
            ScriptEditor = (function () {
                function ScriptEditor() {
                    var _this = this;
                    this.SUFIX_2_MODE = {
                        'lua': 'ace/mode/lua',
                        'sc': 'ace/mode/c_cpp',
                        'sh': 'ace/mode/c_cpp',
                        '': 'ace/mode/yaml'
                    };
                    this.path = window.location.search.substr(1);
                    this.rpc = new playground_rpc_1.PlaygroundRPC();
                    this.sub = new playground_rpc_1.PlaygroundSubscriber();
                    this.saving_in_progres = false;
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
                            _this.save();
                        },
                        bindKey: { win: "ctrl-s" }
                    });
                    this.connect_to_cetech();
                    this.sub.subcribeService("filesystem_service", function (msg) {
                        if (msg.msg_type == 'event') {
                            msg = msg.msg;
                            if (msg.msg_type == 'file_modified') {
                                if (msg.path == _this.path) {
                                    if (_this.saving_in_progres) {
                                        _this.saving_in_progres = false;
                                    }
                                    else {
                                        _this.open(_this.path);
                                    }
                                }
                            }
                        }
                    });
                }
                ScriptEditor.prototype.save = function () {
                    this.saving_in_progres = true;
                    this.rpc.callService("filesystem_service", "write", {
                        path: this.path,
                        content: this.editor.getValue()
                    }, function (msg) {
                    });
                };
                ScriptEditor.prototype.open = function (path) {
                    var _this = this;
                    this.rpc.callService("filesystem_service", "read", {
                        path: this.path
                    }, function (msg) {
                        var curent_pos = _this.editor.getCursorPosition();
                        _this.editor.setValue(msg.response, -1);
                        var type = path.split(".").pop();
                        if (_this.SUFIX_2_MODE.hasOwnProperty(type)) {
                            _this.editor.getSession().setMode(_this.SUFIX_2_MODE[type]);
                        }
                        else {
                            _this.editor.getSession().setMode(_this.SUFIX_2_MODE['']);
                        }
                        _this.editor.moveCursorToPosition(curent_pos);
                    });
                };
                ScriptEditor.prototype.connect_to_cetech = function () {
                    var _this = this;
                    this.rpc.close();
                    this.sub.close();
                    this.rpc.connect("ws://localhost:8888", function () {
                        _this.open(_this.path);
                    });
                    this.sub.connect("ws://localhost:8889");
                };
                ;
                return ScriptEditor;
            }());
            exports_1("ScriptEditor", ScriptEditor);
        }
    }
});
//# sourceMappingURL=script_editor.js.map